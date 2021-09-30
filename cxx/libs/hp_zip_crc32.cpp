#include <libdeflate.h>

#include "hp_zip_crc32.h"

uint32_t crc32(uint32_t crc, const uint8_t *buf, size_t len)
{
    return libdeflate_crc32(crc, buf, len);
}

