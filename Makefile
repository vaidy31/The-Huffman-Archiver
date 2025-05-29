CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

huffman_archiver: main.c dcmpress_funcs.c file_reader.c huffman.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f huffman_archiver