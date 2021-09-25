#ifndef HP_BAM_PARSER_H
#define HP_BAM_PARSER_H

#include <cstddef>
#include <vector>

#include "hp_bam_types.h"

void bam_initial_fsm(BAM_PARSE_FSM *fsm);

typedef struct BGZF_QUEUE BGZF_QUEUE;
void pipeline_bam_parsing(BGZF_QUEUE *bgzf_queue,
                          BAM_HEADER_CALLBACK header_callback,
                          BAM_N_REF_CALLBACK n_ref_callback,
                          BAM_REF_INFO_CALLBACK ref_info_callback,
                          BAM_ALIGN_CALLBACK align_callback, void *user);

#endif // HP_BAM_PARSER_H
