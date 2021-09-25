#ifndef HP_BAM_TYPES_H
#define HP_BAM_TYPES_H

#include <cstdint>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

typedef struct BAM_HEADER
{
    char magic[4];
    uint32_t l_text;
    char text[];
} BAM_HEADER;

typedef struct BAM_REF_NAME
{
    uint32_t l_name;
    char name[];
} BAM_REF_NAME;

typedef struct BAM_ALIGN
{
    uint32_t block_size;
    int32_t refID;
    int32_t pos;
    uint8_t l_read_name;
    uint8_t mapq;
    uint16_t bin;
    uint16_t n_cigar_op;
    uint16_t flag;
    uint32_t l_seq;
    int32_t next_refID;
    int32_t next_pos;
    int32_t tlen;
    char read_name[];
} BAM_ALIGN;

typedef enum BAM_STATES
{
    BAM_FSM_FIND_HEADER,
    BAM_FSM_FIND_N_REF,
    BAM_FSM_PARSE_REF,
    BAM_FSM_PARSE_ALIGN
} BAM_STATES;

typedef struct BAM_PARSE_FSM
{
    BAM_STATES state;
    uint32_t n_ref;
    uint32_t ref_idx;
    char *r;
    size_t r_size;
    size_t r_reserve;
    size_t offset;
} BAM_PARSE_FSM;

typedef void (*BAM_HEADER_CALLBACK)(const char *, uint32_t, void *);
typedef void (*BAM_N_REF_CALLBACK)(uint32_t, void *);
typedef void (*BAM_REF_INFO_CALLBACK)(uint32_t, BAM_REF_NAME *, uint32_t, void *);
typedef void (*BAM_ALIGN_CALLBACK)(size_t, BAM_ALIGN *, void *);

#endif // HP_BAM_TYPES_H
