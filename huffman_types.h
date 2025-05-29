#ifndef HUFFMAN_TYPES_H
#define HUFFMAN_TYPES_H
// базовые структуры
#include <stdio.h>
#include <stdint.h>

typedef struct HuffmanNode {
    unsigned char symbol;     // может не содержать символ (внтуренние вершины)
    unsigned int freq;         
    struct HuffmanNode *left, *right;  // левые и правые листья
} HuffmanNode_t;

// код Хаффмана для символа
typedef struct {
    unsigned int bit_code;     // битовый код
    int length;               // длина кода
} HuffmanCode_t;

// Запись файла в архиве
typedef struct {
    char filename[256];       // имя файла
    size_t offset;           // смещение в архиве 
    size_t size;             // размер сжатых данных
    size_t original_size;    // размер оригинального файла
} ArchiveEntry_t;

// Заголовок архива
typedef struct {
    int file_count;          // количество файлов в архиве
} ArchiveHeader_t;

#endif