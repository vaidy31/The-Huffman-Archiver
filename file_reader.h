#ifndef FILE_READER_H
#define FILE_READER_H
// функции и структуры для чтения файла
#include "huffman_types.h"

// чтение и запись частот для каждого символа
typedef struct {
    FILE *file;             
    unsigned char *buffer;   // буфер 4кб (выделяется в функции)
    unsigned int freq[256];  // таблица частот символов
} FileReader_t;


FileReader_t *file_reader_init(const char *file_name);
void file_reader_free(FileReader_t *obj);
int read_next_block(FileReader_t *obj);
long get_file_size_from_fp(FILE *fp);

#endif