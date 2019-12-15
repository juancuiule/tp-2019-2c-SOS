gcc -fPIC -c  libmuse.c
gcc -shared libmuse.o -o libmuse.so
rm libmuse.o
