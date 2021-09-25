#include <algorithm>
#include <assert.h>
#include <cstdio>
#include <thread>

#include "hp_bgzf_parser.h"
#include "hp_bgzf_queue.h"
#include "hp_bam_parser.h"
#include "bam_correction.h"
#include "hp_fasta_parser.h"
#include "ui_utils.h"

#include "arguments.h"

extern Argument opts;

static BGZF_QUEUE bgzf_slice_queue;
static BAM_CORRECT correct;
static FASTA_MAP reference_map;

int main(int argc, char *argv[])
{
    //Parse the arguments.
    parse_arguments(argc, argv);
    //Initialize the correction map.
    bam_initial_correct(&correct, opts.mapq);
    //Start up the pipeline for FASTA read and parsing.
    std::thread thread_fasta_load(fasta_load_file, opts.reference, &reference_map);
    //Start up the BAM extracting pipeline.
    std::thread thread_bam_correction([&]
    {
        pipeline_bam_parsing(&bgzf_slice_queue,
                             NULL,
                             bam_correct_n_ref,
                             bam_correct_ref_info,
                             bam_correct_align,
                             &correct);
    });
    std::thread thread_bgzf_extract(pipeline_bgzf_parser, opts.mapping, opts.threads, &bgzf_slice_queue);
    //Wait the FASTA reading complete.
    thread_fasta_load.join();
    //Wait the pipeline complete.
    thread_bam_correction.join();
    thread_bgzf_extract.join();
    //Now we get all the information, start to build the narrow mismatches.
    time_print("Calculating the narrow mismatches.");
    NARROW_MAP narrow_mismatches;
    bam_correct_narrow_map(&correct, &narrow_mismatches);
    time_print("Narrow mismatches complete.");
    //Writing the data to output file.
    FILE *out_f = fopen(opts.output, "w");
    //Sort all the values of the map.
    std::sort(reference_map.names.begin(), reference_map.names.end());
    time_print_file("Writing results to %s", opts.output);
    //Loop for each name in the map.
    for(auto &seq_name: reference_map.names)
    {
        //Extract the sequence.
        auto seq_ref = reference_map.seqs[seq_name.idx];
        //Check whether the name appears in the mismatch.
        auto narrow_seq_iter = narrow_mismatches.find(seq_name.name);
        if(narrow_seq_iter == narrow_mismatches.end())
        {
            //Failed to find the name, write the entire string.
            fprintf(out_f, ">%s\n", seq_name.name.c_str());
            fwrite(seq_ref.seq, 1, seq_ref.seq_len, out_f);
            fprintf(out_f, "\n");
        }
        else
        {
            //Find the name, write the slice.
            int base = 0;
            const char *block_name = seq_name.name.c_str(), *fasta_block = seq_ref.seq;
            for(auto mismatch_iter: narrow_seq_iter->second)
            {
                auto s = mismatch_iter[0] - 1, e = mismatch_iter[1] - 1;
                //Write the header.
                fprintf(out_f, ">%s_%d_%d\n", block_name, base+1, s);
                fwrite(fasta_block + base, 1, s - base, out_f);
                fprintf(out_f, "\n");
                fprintf(out_f, ">%s_%d_%d\n", block_name, s+1, e);
                fwrite(fasta_block + s, 1, e - s, out_f);
                fprintf(out_f, "\n");
                //Update base
                base = e;
            }
            if(base < static_cast<int>(seq_ref.seq_len))
            {
                fprintf(out_f, ">%s_%d_%lu\n", block_name, base, seq_ref.seq_len);
                fwrite(fasta_block + base, 1, seq_ref.seq_len - base, out_f);
                fprintf(out_f, "\n");
            }
        }
    }
    fclose(out_f);
    time_print_file("Output complete.", opts.output);
    return 0;
}
