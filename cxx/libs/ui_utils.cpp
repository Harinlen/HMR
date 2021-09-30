#include <ctime>
#include <cstdlib>
#include <cstdio>

#include "ui_utils.h"

void time_print(const char *str)
{
    time_t now = time(0);
    struct tm *tstruct = localtime(&now);
    printf("\033[32m[%02d:%02d:%02d]\033[0m %s\n", tstruct->tm_hour, tstruct->tm_min, tstruct->tm_sec, str);
}

void time_error(int exitCode, const char *str)
{
    time_t now = time(0);
    struct tm *tstruct = localtime(&now);
    printf("\033[32m[%02d:%02d:%02d]\033[0m %s\n", tstruct->tm_hour, tstruct->tm_min, tstruct->tm_sec, str);
    exit(exitCode);
}

void time_error_file(const char *err_msg, const char *file_path)
{
    char buf[1024];
    sprintf(buf, err_msg, file_path);
    time_error(-1, buf);
}

void time_print_file(const char *log_msg, const char *file_path)
{
    char buf[1024];
    sprintf(buf, log_msg, file_path);
    time_print(buf);
}

void time_print_size(const char *log_msg, size_t size_data)
{
    char buf[1024];
    sprintf(buf, log_msg, size_data);
    time_print(buf);
}

void time_print_counter(const char *log_msg, size_t counter, size_t total)
{
    char buf[1024];
    sprintf(buf, log_msg, counter, total);
    time_print(buf);
}
