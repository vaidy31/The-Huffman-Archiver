#ifndef BIT_IO_H
#define BIT_IO_H
// структуры и функц. для записи и чтения данных
#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *file;              
    unsigned char buffer;    // буфер для накопления битов
    int bit_pos;            // позиция бита в буфере
} BitWriter_t;


typedef struct {
    FILE *file;
    uint8_t buffer;         // буфер для чтения битов (255)
    int bits_left;          
} BitReader_t;


void bit_writer_init(BitWriter_t *obj, FILE *file); // init для побитоовой записи
void bit_writer_write(BitWriter_t *obj, int bit);// запись одного бита
void bit_writer_flush(BitWriter_t *obj); // отстаочные биты
void bitreader_init(BitReader_t *br, FILE *file); //init для побитоового чтения
int bitreader_read_bit(BitReader_t *br); //чтение 1 бита

#endif