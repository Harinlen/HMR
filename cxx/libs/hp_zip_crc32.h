#ifndef HP_ZIP_CRC32_H
#define HP_ZIP_CRC32_H

#include <cstdint>
#include <cstddef>

uint32_t crc32(uint32_t crc, const uint8_t *buf, size_t len);

#endif // HP_ZIP_CRC32_H
