#ifndef BAM_FILTER_H
#define BAM_FILTER_H

#include <unordered_map>

#include "hp_bam_types.h"

typedef std::unordered_map<uint64_t, uint64_t> BAM_EDGES;

typedef struct BAM_ALIGN_INFO
{
    size_t offset;
    int32_t refID;
    int32_t pos;
    int32_t next_refID;
    int32_t next_pos;
} BAM_ALIGN_INFO;

typedef struct FASTA_ENZYME FASTA_ENZYME;
typedef struct BAM_FILTER_USER
{
    int mapq;
    size_t total;
    int32_t *enzyme_idx;
    FASTA_ENZYME *enzyme_info;
    std::unordered_map<std::string, BAM_ALIGN_INFO> pending_map;
    std::list<size_t> writing_queue;
    BAM_EDGES edges;
} BAM_FILTER_USER;

void bam_edge_dump(const char *file_name, BAM_FILTER_USER *user);

void bam_filter_n_ref(uint32_t n_ref, void *user);
void bam_filter_ref_info(uint32_t ref_idx, BAM_REF_NAME *ref_name, uint32_t l_ref, void *user);
void bam_filter_align(size_t offset, BAM_ALIGN *align, void *user);

#endif // BAM_FILTER_H
