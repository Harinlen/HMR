#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "hp_zip.h"
#include "hp_zip_crc32.h"
#include "hp_bam_types.h"

#include "ui_utils.h"

#include "compose_reduced.h"

#define MAX_BUFFER_SIZE     (65280)
static uint8_t g_magic[18] = {
    0x1F,       //ID1
    0x8B,       //ID2
    0x08,       //CM
    0x04,       //FLG
    0, 0, 0, 0, //MTIME
    0x00,       //XFL
    0xFF,       //OS
    0x06, 0x00, //XLEN
    0x42,       //SI1
    0x43,       //SI2
    0x02,       //SLEN
    0x00,
    0x00,       //BSIZE
    0x00
};
static const size_t BSIZE_POS = 16;
static char magic[5] = "BAM\1";
static uint8_t eof_marker[28] = {
    0x1F, 0x8B, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x06, 0x00, 0x42, 0x43, 0x02, 0x00, 0x1B, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void cflush(BGZF_COMPOSER *composer)
{
    //Check the data size.
    if(composer->data_size == 0)
        return;
    //Compress the data first.
    size_t bsize = MAX_BUFFER_SIZE << 1;
    uint32_t crc = crc32(0, reinterpret_cast<uint8_t *>(composer->data), composer->data_size);
    bsize = deflate_cdata(composer->compress, bsize, composer->data, composer->data_size);
    //Replace the last g_magic data with bsize.
    uint16_t written_bsize = bsize + 6 + 19;
    memcpy(g_magic + BSIZE_POS, static_cast<void *>(&written_bsize), sizeof(uint16_t));
    fwrite(g_magic, 1, 18, composer->fp);
    //Write the compressed data.
    fwrite(composer->compress, 1, bsize, composer->fp);
    fwrite(&crc, 1, sizeof(uint32_t), composer->fp);
    fwrite(&composer->data_size, 1, sizeof(uint32_t), composer->fp);
    //Reset the data size.
    composer->data_size = 0;
}

void cwrite(const char *data, size_t data_size, BGZF_COMPOSER *composer)
{
    if(composer->data_size + data_size < MAX_BUFFER_SIZE)
    {
        //Just copy all the data to buffer.
        memcpy(composer->data + composer->data_size, data, data_size);
        //Increase the data size.
        composer->data_size += data_size;
        return;
    }
    //Keep writing until the buffer can hold.
    size_t size_can_written = 0;
    while(composer->data_size + data_size >= MAX_BUFFER_SIZE)
    {
        //Fill the size up to a buffer size.
        size_can_written = MAX_BUFFER_SIZE - composer->data_size;
        memcpy(composer->data + composer->data_size, data, size_can_written);
        composer->data_size = MAX_BUFFER_SIZE;
        //Compress the data.
        cflush(composer);
        //Move the data pointer.
        data += size_can_written;
        data_size -= size_can_written;
        //Clear the data size.
        composer->data_size = 0;
    }
    //Save the remain bytes.
    memcpy(composer->data, data, data_size);
    composer->data_size = data_size;
}

void compose_init(BGZF_COMPOSER *composer, const char *file_path)
{
    //Create the buffer for the composer.
    composer->data = static_cast<char *>(malloc(MAX_BUFFER_SIZE));
    composer->data_size = 0;
    //Compose the gzip buffer.
    composer->compress = static_cast<char *>(malloc(MAX_BUFFER_SIZE << 1));
    //Write the BAM header.
    composer->fp = fopen(file_path, "wb");
    //Write the magic char.
    cwrite(magic, 4, composer);
}

void compose_reduced_header(const char *text, uint32_t l_text, void *user)
{
    BGZF_COMPOSER *composer = static_cast<BGZF_COMPOSER *>(user);
    //Write the header size.
    cwrite(reinterpret_cast<char *>(&l_text), sizeof(uint32_t), composer);
    cwrite(text, l_text, composer);
}

void compose_n_ref(uint32_t n_ref, void *user)
{
    BGZF_COMPOSER *composer = static_cast<BGZF_COMPOSER *>(user);
    //Write the header size.
    cwrite(reinterpret_cast<char *>(&n_ref), sizeof(uint32_t), composer);
    composer->n_ref = n_ref;
}

void compose_ref_name(uint32_t ref_idx, BAM_REF_NAME *name, uint32_t l_ref, void *user)
{
    BGZF_COMPOSER *composer = static_cast<BGZF_COMPOSER *>(user);
    //Write the info of the name.
    cwrite(reinterpret_cast<char *>(&(name->l_name)), sizeof(uint32_t), composer);
    cwrite(name->name, name->l_name, composer);
    cwrite(reinterpret_cast<char *>(&l_ref), sizeof(uint32_t), composer);
    if(ref_idx == composer->n_ref - 1) { cflush(composer); }
}

void compose_close(BGZF_COMPOSER *composer)
{
    //Flush the composer first.
    cflush(composer);
    //Write the EOF marker.
    fwrite(eof_marker, 1, 28, composer->fp);
    //Close the file.
    fflush(composer->fp);
    fclose(composer->fp);
    //Free the memory.
    free(composer->data);
    free(composer->compress);
}

void compose_align_info(size_t offset, BAM_ALIGN *align, void *user)
{
    BGZF_COMPOSER *composer = static_cast<BGZF_COMPOSER *>(user);
    //Check whether the offset appears in the vector or not.
    if(!std::binary_search(composer->align_offsets->begin(), composer->align_offsets->end(), offset))
        return;
    //Write the content to composer.
    cwrite(reinterpret_cast<char *>(align), align->block_size + sizeof(uint32_t), composer);
}
