#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <libdeflate.h>

#include "hp_unzip.h"

int inflate_cdata(char *dest, uint32_t *destLen,
                  const char *source, const uint16_t sourceLen)
{
    struct libdeflate_decompressor *z = libdeflate_alloc_decompressor();

    size_t actual_out;
    libdeflate_deflate_decompress(z,
                                  source, sourceLen,
                                  dest, static_cast<size_t>(*destLen),
                                  &actual_out);
    *destLen = actual_out;
    libdeflate_free_decompressor(z);
    return 0;
}
