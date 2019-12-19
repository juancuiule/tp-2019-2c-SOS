#include <stdio.h>
#include <stdlib.h>
#include <libmuse/libmuse.h>
#include <unistd.h>

void recursiva(int num)
{
	if(num == 0)
		return;

	uint32_t ptr = muse_alloc(4);
	muse_cpy(ptr, &num, 4);
	printf("%d\n", num);

	recursiva(num - 1);
	num = 0; // Se pisa para probar que muse_get cargue el valor adecuado
	muse_get(&num, ptr, 4);
	printf("%d\n", num);
	muse_free(ptr);
}

int main(void)
{
	muse_init(getpid(), "127.0.0.1", 9000);
	recursiva(10);
	muse_close();
}
