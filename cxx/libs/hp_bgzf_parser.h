#ifndef HP_BGZF_PARSER_H
#define HP_BGZF_PARSER_H

#include <cstdint>

typedef struct BGZF_QUEUE BGZF_QUEUE;
void pipeline_bgzf_parser(const char *bam_path, const int threads, BGZF_QUEUE *queue);

#endif // HP_BGZF_PARSER_H
