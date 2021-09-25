#ifdef __unix__
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include "ui_utils.h"

#include "hp_file_map.h"

#ifdef __unix__
int map_file(const char *file_path, char **data, size_t *data_size)
{
    int fd;
    struct stat fd_stat;
    //Open the file by LINUX system.
    if((fd = open(file_path, O_RDONLY)) < 0)
    {
        time_error_file("Failed to open FASTA %s", file_path);
    }
    if(fstat(fd, &fd_stat) < 0)
    {
        time_error_file("Failed to get FASTA file information %s", file_path);
    }
    (*data) = static_cast<char *>(mmap(NULL, fd_stat.st_size, PROT_READ, MAP_SHARED, fd, 0));
    if((*data) == MAP_FAILED)
    {
        time_error_file("Failed to MMAP FASTA file %s", file_path);
    }
    //Save the FASTA size.
    (*data_size) = fd_stat.st_size;
    return fd;
}

void unmap_file(int fd, void *fp, size_t fp_size)
{
    munmap(fp, fp_size);
    close(fd);
}
#endif
