#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <cstddef>

/*!
 * \brief Print the text with the current time at the beginning.
 * \param str The text to be printed.
 */
void time_print(const char *str);

/*!
 * \brief Exit the program with an error code, and print an error info.
 * \param exitCode The exit code to be set.
 * \param str The text to be printed.
 */
void time_error(int exitCode, const char *str);


void time_error_file(const char *err_msg, const char *file_path);

void time_print_file(const char *log_msg, const char *file_path);

void time_print_size(const char *log_msg, size_t size_data);

void time_print_counter(const char *log_msg, size_t counter, size_t total);

#endif // UI_UTILS_H
