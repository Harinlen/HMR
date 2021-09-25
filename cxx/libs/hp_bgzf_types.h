#ifndef HP_BGZF_TYPES_H
#define HP_BGZF_TYPES_H

#include <cstddef>
#include <mutex>
#include <condition_variable>

typedef struct BAM_DATA_SLICE
{
    char *data;
    size_t size;
    bool is_used;
    std::mutex mutex;
    std::condition_variable cv;
} BAM_DATA_SLICE;

#endif // HP_BGZF_TYPES_H
