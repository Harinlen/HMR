#ifndef HP_ZIP_H
#define HP_ZIP_H

#include <cstddef>

size_t deflate_cdata(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len);

#endif // HP_ZIP_H
