#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

uint64_t get_current_time();
size_t get_filesize(char *filename);
void reverse_string_vector(char **strings);
int get_size_filenames(char **names);
char* get_last_filename_from_path(char *path);
char* get_lastfile_previous_path(char *path);
int roundup(double number);
#endif /* UTILS_H_ */
