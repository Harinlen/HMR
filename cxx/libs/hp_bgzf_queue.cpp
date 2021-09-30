#include "hp_bgzf_queue.h"

void bgzf_push_queue(BGZF_QUEUE *q, const BGZF_DATA_SLICE &buffer)
{
    //Lock the queue first.
    std::unique_lock<std::mutex> queue_lock(q->mutex);
    q->push_cv.wait(queue_lock, [q]{return q->queue.size() < 16;});
    //Push the buffer to the end of the queue.
    q->queue.push_back(buffer);
    //Notify the queue.
    q->pop_cv.notify_one();
}

BGZF_DATA_SLICE bgzf_pop_queue(BGZF_QUEUE *q)
{
    //Try to lock the queue.
    std::unique_lock<std::mutex> queue_lock(q->mutex);
    q->pop_cv.wait(queue_lock, [q]{return (!q->queue.empty()) || (q->finish);});
    //Extract the buffer from the queue.
    BGZF_DATA_SLICE data_slice {NULL, 0};
    if(!q->queue.empty())
    {
        data_slice = q->queue.front();
        q->queue.pop_front();
    }
    //Unlock the queue.
    queue_lock.unlock();
    q->push_cv.notify_one();
    return data_slice;
}

void bgzf_queue_complete(BGZF_QUEUE *q)
{
    //Lock the queue first.
    std::unique_lock<std::mutex> queue_lock(q->mutex);
    //Mark the queue is finished.
    q->finish = true;
    //Notify the queue.
    q->pop_cv.notify_one();
}
