#include "utils.h"

size_t get_filesize(char *filename)
{
	FILE* fd = fopen(filename, "r");

	fseek(fd, 0L, SEEK_END);
	size_t size = ftell(fd);

	fclose(fd);
	return size;
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


