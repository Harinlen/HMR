#include <libdeflate.h>

#include "hp_zip.h"

size_t deflate_cdata(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len)
{
    struct libdeflate_compressor *z = libdeflate_alloc_compressor(9);
    size_t data_size = libdeflate_deflate_compress(z, pSrc_buf, src_buf_len, pOut_buf, out_buf_len);
    libdeflate_free_compressor(z);
    return data_size;
}
