.PHONY: MUSE

server: MUSE.c ./segmentpag.c ../libmuse/utils.c ../libmuse/network.c
	gcc MUSE.c ./segmentpag.c ../libmuse/utils.c ../libmuse/network.c -o MUSE -lm -lcommons -lpthread