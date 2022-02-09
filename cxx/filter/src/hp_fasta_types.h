#ifndef HP_FASTA_TYPES_H
#define HP_FASTA_TYPES_H

#include <string>
#include <vector>

typedef std::pair<int32_t, int32_t> ENZYME_RANGE;
typedef std::vector<ENZYME_RANGE> ENZYME_RANGES;

typedef struct FASTA_ENZYME
{
    size_t n_ref;
    char **ref_name;
    std::vector<size_t> ref_length;
    std::vector<ENZYME_RANGES> ranges;
} FASTA_ENZYME;

#endif // HP_FASTA_TYPES_H
