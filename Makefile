all:
	gcc test.c defragmenter.c -lm -o defrag -std=c99 -O0 -w
