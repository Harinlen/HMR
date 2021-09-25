#ifndef HP_FILE_MAP_H
#define HP_FILE_MAP_H

#include <cstddef>

#ifdef __unix__
#define HP_FASTA_MMAP
#endif

#ifdef HP_FASTA_MMAP
int map_file(const char *file_path, char **data, size_t *fasta_size);
void unmap_file(int fd, void *fp, size_t fp_size);
#endif


#endif // HP_FILE_MAP_H
