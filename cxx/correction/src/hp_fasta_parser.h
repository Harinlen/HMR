#ifndef HP_FASTA_PARSER_H
#define HP_FASTA_PARSER_H

#include <unordered_map>

#include "hp_fasta_types.h"

void fasta_load_file(const char *file_path, FASTA_MAP *map);

#endif // HP_FASTA_PARSER_H
