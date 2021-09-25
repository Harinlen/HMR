
#include "hp_zip_crc32.h"

static volatile bool crc_table_empty = true;
static uint32_t crc_table[256];

void make_crc_table()
{
    uint32_t c, poly; /* polynomial exclusive-or pattern */
    int n, k;
    /* terms of polynomial defining this crc (except x^32): */
    static const unsigned char p[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};

    // Make exclusive-or pattern from polynomial (0xedb88320UL)
    poly = 0;
    for (n = 0; n < (int)(sizeof(p)/sizeof(unsigned char)); n++)
        poly |= (uint32_t)1 << (31 - p[n]);

    /* generate a crc for every 8-bit value */
    for (n = 0; n < 256; n++) {
        c = static_cast<uint32_t>(n);
        for (k = 0; k < 8; k++)
            c = c & 1 ? poly ^ (c >> 1) : c >> 1;
        crc_table[n] = c;
    }

    crc_table_empty = false;
}

#define DO1 crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

uint32_t crc32(uint32_t crc, const uint8_t *buf, size_t len)
{
    if (buf == NULL) return 0UL;
    crc = crc ^ 0xffffffff;
    while (len >= 8)
    {
        DO8;
        len -= 8;
    }
    if(len)
    {
        do
        {
            DO1;
        }
        while (--len);
    }
    return crc ^ 0xffffffff;
}

