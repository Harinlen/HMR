#include <list>
#include <condition_variable>

#include "hp_file_map.h"
#include "hp_thread_pool.h"
#include "hp_bgzf_queue.h"
#include "hp_unzip.h"
#include "ui_utils.h"

#include "hp_bgzf_parser.h"

typedef struct BGZF_BLOCK
{
    const char *data;
    uint16_t size;
    char *extract_pos;
    uint32_t extract_size;
} BGZF_BLOCK;

void decompress_field(const BGZF_BLOCK &block)
{
    uint32_t block_data_size = block.extract_size;
    inflate_cdata(block.extract_pos, &block_data_size, block.data, block.size);
}

typedef thread_pool<void (const BGZF_BLOCK &), BGZF_BLOCK> WORK_THREADS;

typedef struct GZIP_HEADER
{
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    uint8_t FLG;
    uint32_t MTIME;
    uint8_t XFL;
    uint8_t OS;
    uint16_t XLEN;
} GZIP_HEADER;

typedef struct GZIP_EXTRA_FIELD_HEADER
{
    uint8_t SI1;
    uint8_t SI2;
    uint16_t SLEN;
} GZIP_EXTRA_FIELD_HEADER;

void pipeline_bgzf_parser(const char *bam_path, const int threads, BGZF_QUEUE *queue)
{
    //Read the BAM file.
    time_print_file("Start extracting BGZF file %s", bam_path);
#ifdef HP_FASTA_MMAP
    char *bam_data;
    size_t bam_data_size;
    int fd = map_file(bam_path, &bam_data, &bam_data_size);
#endif
    //Loop and yield the position.
    std::list<BGZF_BLOCK> works;
    char *bam_pos = bam_data;
    const char *bam_end = bam_data + bam_data_size;
    size_t extract_threshold = static_cast<size_t>(threads) * 128, part_size = 0;
    WORK_THREADS decompress_pool(decompress_field, threads);
    while(bam_pos < bam_end)
    {
        GZIP_HEADER *gzip_header = reinterpret_cast<GZIP_HEADER *>(bam_pos);
        bam_pos += sizeof(GZIP_HEADER);
        uint16_t xlen = gzip_header->XLEN, BSIZE = 0;
        while(xlen > 0)
        {
            GZIP_EXTRA_FIELD_HEADER *extra_field_header = reinterpret_cast<GZIP_EXTRA_FIELD_HEADER *>(bam_pos);
            //Skip the position.
            //Check the header.
            if(extra_field_header->SI1 == 66 && extra_field_header->SI2 == 67 && // Fixed 66, 67
                    extra_field_header->SLEN == 2)
            {
                const uint16_t *BSIZE_PTR = reinterpret_cast<const uint16_t *>(bam_pos + sizeof(GZIP_EXTRA_FIELD_HEADER));
                BSIZE = static_cast<int>(*BSIZE_PTR);
            }
            bam_pos += extra_field_header->SLEN + sizeof(GZIP_EXTRA_FIELD_HEADER);
            xlen -= extra_field_header->SLEN + sizeof(GZIP_EXTRA_FIELD_HEADER);
        }
        //Record the block data.
        const char *cdata = bam_pos;
        uint16_t cdata_size = BSIZE - gzip_header->XLEN - 19;
        bam_pos += cdata_size + 4;
        uint32_t ISIZE = *reinterpret_cast<const uint32_t *>(bam_pos);
        works.push_back(BGZF_BLOCK{cdata, cdata_size, NULL, ISIZE});
        part_size += ISIZE;
        if(works.size() == extract_threshold)
        {
            //Decompress the file.
            char *gzip_cache = static_cast<char *>(malloc(part_size)),
                    *iter_cache = gzip_cache;
            for(auto iter: works)
            {
                //Deploy the task.
                iter.extract_pos = iter_cache;
                decompress_pool.push_task(iter);
                //Update the position.
                iter_cache += iter.extract_size;
            }
            //Wait for all the task complete.
            decompress_pool.wait_for_tasks();
            //Push the task to the queue.
            bgzf_push_queue(queue, BGZF_DATA_SLICE{gzip_cache, part_size});
            //Reset the data.
            part_size = 0;
            works = std::list<BGZF_BLOCK>();
        }
        //Move the bam pointer.
        bam_pos += 4;
    }
    //End the file.
    bgzf_queue_complete(queue);
#ifdef HP_FASTA_MMAP
    unmap_file(fd, bam_data, bam_data_size);
#endif
    time_print("BGZF file extracting complete.");
}
