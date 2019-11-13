#include "utils.h"

uint64_t get_current_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

size_t get_filesize(char *filename)
{
	FILE* fd = fopen(filename, "r");

	fseek(fd, 0L, SEEK_END);
	size_t size = ftell(fd);

	fclose(fd);
	return size;
}

int get_size_filenames(char **names)
{
	int name_length = -1;
	while(names[++name_length] != NULL){/*do nothing*/};
	return name_length;
}

//desc: invierto el orden del vector de strings. de ["home", "lala.txt", NULL] a ["lala.txt", "home", NULL]
void reverse_string_vector(char ** strings)
{
	int size = sizeof(strings);
	int j= size - 2;
	char *temp;

	for(int i = 0; i < j ; i++){
		temp = strings[i];
		strings[i] = strings[j];
		strings[j] = temp;
		j--;
	}
}

//desc: obtengo el nombre del ultimo file del path
char* get_last_filename_from_path(char *path)
{
	char **path_names = string_split(path, "/");
	int path_elems = get_size_filenames(path_names);

	return path_names[path_elems-1];
}

//desc: obtengo de /home/casa/lala.txt a /home/casa
char* get_lastfile_previous_path(char *path)
{
	char **pathnames = string_split(path, "/");
	int path_elems = get_size_filenames(pathnames);
	int tam_last_filename = strlen( pathnames[path_elems - 1] );

	return string_substring(path, 0, strlen(path) - tam_last_filename);

}
