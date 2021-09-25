#ifndef HP_FASTA_TYPES_H
#define HP_FASTA_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

typedef struct SEQ_DATA
{
    char *seq;
    size_t seq_len;
} SEQ_DATA;

typedef struct SEQ_SLICE
{
    char *seq;
    size_t seq_len;
    size_t offset;
} SEQ_SLICE;

typedef struct SEQ_NAME
{
    std::string name;
    size_t idx;
    bool operator < (const struct SEQ_NAME &other) const
    {
        return name < other.name;
    }

} SEQ_NAME;

typedef struct FASTA_MAP
{
    std::vector<SEQ_NAME> names;
    std::vector<SEQ_DATA> seqs;
} FASTA_MAP;

#endif // HP_FASTA_TYPES_H
