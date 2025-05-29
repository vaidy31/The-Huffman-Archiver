#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_reader.h"

FileReader_t *file_reader_init(const char *file_name) {
    FileReader_t *obj = malloc(sizeof(FileReader_t));
    if (!obj) {
        return NULL;
    }
    
    obj->file = fopen(file_name, "rb"); // открываем в битах
    if (!obj->file) {
        free(obj);
        return NULL;
    }
    
    obj->buffer = malloc(sizeof(unsigned char) * 4096); // выделяем в буфер 4кб
    if (!obj->buffer) {
        fclose(obj->file);
        free(obj);
        return NULL;
    }

    //init
    memset(obj->freq, 0, sizeof(obj->freq)); // зануляем для того чтобы делать freq[символ]++

    return obj;
}

void file_reader_free(FileReader_t *obj) {
    if (obj) {
        if (obj->buffer) {
            free(obj->buffer);
        }
        if (obj->file) {
            fclose(obj->file);
        }
        free(obj);
    }
}

int read_next_block(FileReader_t *obj) {
    size_t bytes_read = fread(obj->buffer, 1, 4096, obj->file); // чтение по 4кб

    if (bytes_read == 0) {
        return 0;
    }
    
    for (size_t i = 0; i < bytes_read; i++) {
        int byte = obj->buffer[i];
        obj->freq[byte]++; // обновление таблицы частот
    }

    return (int)bytes_read; // для проверок
}

long get_file_size_from_fp(FILE *fp) {
    long current = ftell(fp); //текущаб=я позиуи==ция в файле

    fseek(fp, 0, SEEK_END); // в конец
    long size = ftell(fp); // размер
    fseek(fp, current, SEEK_SET); // в начало
    return size;
}