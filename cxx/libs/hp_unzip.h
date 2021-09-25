#ifndef HP_UNZIP_H
#define HP_UNZIP_H

#include <cstdint>

int inflate_cdata(char *dest, uint32_t *destLen,
                  const char *source, const uint16_t sourceLen);

#endif // HP_UNZIP_H
