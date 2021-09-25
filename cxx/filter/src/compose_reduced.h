#ifndef COMPOSE_REDUCED_H
#define COMPOSE_REDUCED_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

typedef struct BGZF_COMPOSER
{
    char *data;
    size_t data_size;
    char *compress;
    uint32_t n_ref;
    std::vector<size_t> *align_offsets;
    FILE *fp;
} BGZF_COMPOSER;

typedef struct BAM_REF_NAME BAM_REF_NAME;
typedef struct BAM_ALIGN BAM_ALIGN;

void cflush(BGZF_COMPOSER *composer);

void compose_init(BGZF_COMPOSER *composer, const char *file_path);
void compose_close(BGZF_COMPOSER *composer);

void compose_reduced_header(const char *text, uint32_t l_text, void *user);
void compose_n_ref(uint32_t n_ref, void *user);
void compose_ref_name(uint32_t ref_idx, BAM_REF_NAME *name, uint32_t l_ref, void *user);
void compose_align_info(size_t offset, BAM_ALIGN *align, void *user);


#endif // COMPOSE_REDUCED_H
