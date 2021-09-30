#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <thread>

#include "arguments.h"
#include "enzyme.h"
#include "ui_utils.h"
#include "hp_fasta_search.h"
#include "hp_fasta_types.h"
#include "hp_bgzf_parser.h"
#include "hp_bgzf_queue.h"
#include "hp_bam_parser.h"

#include "bam_filter.h"
#include "compose_reduced.h"

extern Argument opts;
static BGZF_QUEUE bgzf_slice_queue;
static BAM_FILTER_USER bam_filter;
static BGZF_COMPOSER reduced_composer;

int main(int argc, char *argv[])
{
    //Parse the arguments.
    parse_arguments(argc, argv);
    //Read the FASTA file and find the enzyme.
    const char *nuc_seq = NULL;
    size_t nuc_seq_size = 0;
    check_enzyme(opts.enzyme, &nuc_seq, &nuc_seq_size);
    //Start to search enzyme position in FASTA seq.
    FASTA_ENZYME enzyme_pos;
    enzyme_pos.ref_name = NULL;
    time_print_file("Building enzyme index in %s", opts.reference);
    fasta_search_enzyme(opts.reference, &enzyme_pos);
    time_print_size("Enzyme index built, total sequenece: %d", enzyme_pos.n_ref);
    bam_filter.enzyme_info = &enzyme_pos;
    //Now start parsing the BAM file.
    time_print_file("Filtering mapping file %s", opts.mapping);
    {
        std::thread thread_bam_filter([&]
        {
            pipeline_bam_parsing(&bgzf_slice_queue,
                                 NULL,
                                 bam_filter_n_ref,
                                 bam_filter_ref_info,
                                 bam_filter_align,
                                 &bam_filter);
        });
        std::thread thread_bgzf_parsing(pipeline_bgzf_parser, opts.mapping, opts.threads, &bgzf_slice_queue);
        thread_bam_filter.join();
        thread_bgzf_parsing.join();
    }
    time_print("Mapping file filter completed.");
    time_print_counter("Filtered %lu/%lu from the BAM file.", bam_filter.writing_queue.size(), bam_filter.total);
    time_print("Sorting the Align information...");
    size_t writing_idx = 0, writing_counts = bam_filter.writing_queue.size();
    size_t *writing_vector = static_cast<size_t *>(malloc(writing_counts * sizeof(size_t)));
    for(auto i=bam_filter.writing_queue.begin(); i!=bam_filter.writing_queue.end(); ++i, ++writing_idx)
    {
        writing_vector[writing_idx] = *i;
    }
    bam_filter.writing_queue = std::list<size_t>();
    std::sort(writing_vector, writing_vector + writing_counts);
    //Initial the reduced BAM composer.
    compose_init(&reduced_composer, opts.output);
    reduced_composer.align_offsets = writing_vector;
    reduced_composer.align_offsets_counts = writing_counts;
    reduced_composer.align_offsets_idx = 0;
    time_print_file("Composing the reduced bam file to %s", opts.output);
    {
        bgzf_slice_queue.finish = false;
        std::thread thread_bam_filter([&]
        {
            pipeline_bam_parsing(&bgzf_slice_queue,
                                 compose_reduced_header,
                                 compose_n_ref,
                                 compose_ref_name,
                                 compose_align_info,
                                 &reduced_composer);
        });
        std::thread thread_bgzf_parsing(pipeline_bgzf_parser, opts.mapping, opts.threads, &bgzf_slice_queue);
        thread_bam_filter.join();
        thread_bgzf_parsing.join();
    }
    //Flush the composer.
    compose_close(&reduced_composer);
    time_print("Finished");
    return 0;
}
