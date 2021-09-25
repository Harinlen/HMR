#ifndef BAM_CORRECTION_H
#define BAM_CORRECTION_H

#include <cstdint>
#include <cstddef>

#include "hp_bam_types.h"

typedef struct BAM_CORRECT_ALIGN
{
    int32_t pos;
    int32_t next_pos;
} BAM_CORRECT_ALIGN;

typedef struct BAM_CORRECT
{
    uint8_t mapq;
    char **ref_names;
    uint32_t ref_name_size;
    std::vector<std::list<BAM_CORRECT_ALIGN>> ref_aligns;
} BAM_CORRECT;

typedef std::vector<std::vector<int> > MISMATCH;
typedef std::unordered_map<std::string, MISMATCH> NARROW_MAP;

void bam_initial_correct(BAM_CORRECT *correct, uint8_t mapq);

void bam_correct_n_ref(uint32_t n_ref, void *user);
void bam_correct_ref_info(uint32_t ref_idx, BAM_REF_NAME *ref_name, uint32_t, void *user);
void bam_correct_align(size_t offset, BAM_ALIGN *align, void *user);

void bam_correct_narrow_map(BAM_CORRECT *correct, NARROW_MAP *narrow_mismatches);

#endif // BAM_CORRECTION_H
