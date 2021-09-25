#ifndef HP_STR_UTILS_H
#define HP_STR_UTILS_H

#include <cstddef>

inline bool is_space(char c)
{
    return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' ||  c == ' ';
}

inline void trimmed_right(char *buf, size_t size)
{
    while(size > 0 && is_space(buf[size-1]))
        buf[--size] = '\0';
}

#endif // HP_STR_UTILS_H
