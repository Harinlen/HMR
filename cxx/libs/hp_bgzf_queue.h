#ifndef HP_BGZF_QUEUE_H
#define HP_BGZF_QUEUE_H

#include <deque>
#include <condition_variable>
#include <mutex>

typedef struct BGZF_DATA_SLICE
{
    char *data;
    size_t size;
} BGZF_DATA_SLICE;

typedef struct BGZF_QUEUE
{
    bool finish;
    std::deque<BGZF_DATA_SLICE> queue;
    std::mutex mutex;
    std::condition_variable pop_cv, push_cv;
} BGZF_QUEUE;

void bgzf_push_queue(BGZF_QUEUE *q, const BGZF_DATA_SLICE &buffer);
void bgzf_queue_complete(BGZF_QUEUE *q);
BGZF_DATA_SLICE bgzf_pop_queue(BGZF_QUEUE *q);

#endif // HP_BGZF_QUEUE_H
