#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "file_reader.h"
#include "bit_inp_out.h"
#include "dcmpressor.h"
#include "huffman_types.h"
#include "huffman.h"

void bit_writer_init(BitWriter_t *obj, FILE *file) {
    obj->file = file;
    obj->buffer_pos = 0;
    obj->current_byte = 0; // текущий байт для записи
    obj->bit_pos = 7;
}

void bit_writer_write(BitWriter_t *obj, int bit) {
    if (bit) { // если биь 1, то записываем в буфер его
        obj->current_byte |= (1 << obj->bit_pos);
    }
    obj->bit_pos--; // next 
    // буфер заполнен, значит сбрасываем его, но сначала записываем в файл
    if (obj->bit_pos < 0) {
        obj->write_buffer[obj->buffer_pos++] = obj->current_byte; // записываем текущий байт в буфер
        obj->current_byte = 0;
        obj->bit_pos = 7;

        if (obj->buffer_pos == 4096) { // если буфер заполнен
            fwrite(obj->write_buffer, 1, 4096, obj->file);
            obj->buffer_pos = 0;
        }
    }
}

void bit_writer_flush(BitWriter_t *obj) {
    if (obj->bit_pos != 7) { // если в буфере остались не записанные биты
        obj->write_buffer[obj->buffer_pos++] = obj->current_byte;
    }
    if (obj->buffer_pos > 0) {
        fwrite(obj->write_buffer, 1, obj->buffer_pos, obj->file);
        obj->buffer_pos = 0;
    }
}

void bitreader_init(BitReader_t *br, FILE *file) {
    br->file = file;
    br->buffer = 0;
    br->bits_left = 0;
}

int bitreader_read_bit(BitReader_t *br) {
    if (br->bits_left == 0) { // читаем новый байт если буфер все
        if (feof(br->file)) return -1;
        if (fread(&br->buffer, 1, 1, br->file) != 1){
            return -1;
        }
        br->bits_left = 8;
    }
    // двигаем буфер
    int bit = (br->buffer >> 7) & 1;
    br->buffer <<= 1;
    br->bits_left--;
    return bit;
}




void compress_file_n_write(FILE *archive, const char *input, ArchiveEntry_t *entry) {
    printf("Compressing %s ---> archive file\n", input);

    FileReader_t *reader = file_reader_init(input);
    if (!reader) {
        printf("Error: Problem with open input file %s\n", input);
        return;
    }
    
    FILE *input_fp = fopen(input, "rb");
    if (!input_fp) {
        printf("Error: Cannot open file %s to get size\n", input);
        file_reader_free(reader);
        return;
    }
    
    long file_size = get_file_size_from_fp(input_fp);
    fclose(input_fp);
    
    if (file_size <= 0) {
        printf("Error: Cannot get file size for %s\n", input);
        file_reader_free(reader);
        return;
    }

    size_t process = 0;
    printf("Building frequency table...\n");

    // подсчет частот
    size_t bytes_read;
    while ((bytes_read = read_next_block(reader)) > 0) {
        process += bytes_read;
        if (file_size > 0) {
            print_progress_bar(process, file_size);
        }
    }

    printf("Building Huffman tree...\n");
    HuffmanNode_t *tree_root = generate_huffman_tree(reader->freq);
    if (!tree_root) {
        printf("Error: Cannot build Huffman tree\n");
        file_reader_free(reader);
        return;
    }

    HuffmanCode_t table[256] = {0};
    generate_codes(tree_root, table, 0, 0);

    // сохраняем метаданные
    entry->offset = ftell(archive); // запоминаем позицию в архиве
    strncpy(entry->filename, input, 255); // копируем имя
    entry->filename[255] = '\0';
    entry->original_size = file_size; // сохраняем оригинальный размер


    fwrite(&entry->original_size, sizeof(size_t), 1, archive); // записываем оригинальный размер
    // записываем частоты в архив
    fwrite(reader->freq, sizeof(unsigned int), 256, archive);

    printf("Compressing data...\n");
    rewind(reader->file); // прыгаем в начало файла
    process = 0;
    
    BitWriter_t obj_bw;
    bit_writer_init(&obj_bw, archive);

    // сжимаем данные
    while ((bytes_read = read_next_block(reader)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char symbol = reader->buffer[i];
            HuffmanCode_t code = table[symbol]; // берем код для символа
            // запись происходит побитого со старшего бита
            for (int j = code.length - 1; j >= 0; j--) {
                int bit = (code.bit_code >> j) & 1;
                bit_writer_write(&obj_bw, bit);
            }
        }
        process += bytes_read;
        if (file_size > 0) {
            print_progress_bar(process, file_size);
        }
    }

    bit_writer_flush(&obj_bw); // вдруг остался неполный байт
    entry->size = ftell(archive) - entry->offset; // сохранение рахмера сжатых данных
    
    file_reader_free(reader);
    free_huffman_tree(tree_root);
    
    printf("Compression completed successfully!\n");
}

void compress_files(char **input_files, int file_count, const char *archive_name) {
    printf("Creating archive: %s\n", archive_name);
    FILE *archive = fopen(archive_name, "wb");
    if (!archive) {
        printf("Problem with oppenning archive");
        return;
    }

    // записываем заголовок архива
    fwrite(&file_count, sizeof(int), 1, archive); // кол-во файлов
    long header_pos = ftell(archive);
    fseek(archive, sizeof(ArchiveEntry_t) * file_count, SEEK_CUR);
    
    ArchiveEntry_t *entries = calloc(file_count, sizeof(ArchiveEntry_t));

    // сжимаем каждый файл
    for (int i = 0; i < file_count; i++) {
        printf("[%d/%d] %s\n", i + 1, file_count, input_files[i]);
        compress_file_n_write(archive, input_files[i], &entries[i]);
    }

    // Записываем таблицу файлов
    fseek(archive, header_pos, SEEK_SET);
    fwrite(entries, sizeof(ArchiveEntry_t), file_count, archive);
    
    fclose(archive);
    free(entries);
    printf("Archive successfully created.\n");
}



void decompress_file(BitReader_t *br, HuffmanNode_t *root, FILE *out, size_t original_size) {
    printf("Decompressing file...\n");
    
    if (!root) {
        printf("Error: Huffman tree is empty\n");
        return;
    }

    HuffmanNode_t *current_node = root;
    size_t decompressed_bytes = 0; // счетчик
   
    int last_percent = -1;

    while (decompressed_bytes < original_size) {
        int bit = bitreader_read_bit(br);
        if (bit < 0) break;

        current_node = bit ? current_node->right : current_node->left;

        if (!current_node) {
            printf("Error: Invalid Huffman tree path\n");
            break;
        }

        // достигли листового узла (есть символ)
        if (!current_node->left && !current_node->right) {
            fputc(current_node->symbol, out);
            current_node = root;
            decompressed_bytes++;
            
            // обновляем прогресс-бар
            int percent = (int)((double)decompressed_bytes / original_size * 100);
            if (percent != last_percent) {
                printf("\rProgress: %d%% (%zu/%zu bytes)", percent, decompressed_bytes, original_size);
                fflush(stdout);
                last_percent = percent;
            }
        }
    }

    printf("\nDecompression completed successfully! Total: %zu bytes\n", decompressed_bytes);
}


void decompress_archive(const char *archive_name) {
    FILE *in = fopen(archive_name, "rb");
    if (!in) {
        perror("fopen archive");
        return;
    }

    int file_count;
    if (fread(&file_count, sizeof(int), 1, in) != 1) {
        printf("Error: Cannot read file count from archive\n");
        fclose(in);
        return;
    }
    
    printf("Archive contains %d file(s)\n", file_count);
    
    if (file_count <= 0) {
        printf("Error: Invalid file count: %d\n", file_count);
        fclose(in);
        return;
    }
    
    // чтение мета-данных
    ArchiveEntry_t *entries = calloc(file_count, sizeof(ArchiveEntry_t));
    if (!entries) {
        printf("Error: Cannot allocate memory for entries\n");
        fclose(in);
        return;
    }
    
    if (fread(entries, sizeof(ArchiveEntry_t), file_count, in) != (size_t)file_count) {
        printf("Error: Cannot read archive entries\n");
        free(entries);
        fclose(in);
        return;
    }
    printf("Entries read successfully. Starting extraction...\n");
    for (int i = 0; i < file_count; ++i) {
        ArchiveEntry_t *entry = &entries[i];
        printf("[%d/%d] Extracting: %s (%zu bytes)\n", i + 1, file_count, entry->filename, entry->original_size);
        
        fseek(in, entries[i].offset, SEEK_SET);

        size_t original_size;
        if (fread(&original_size, sizeof(size_t), 1, in) != 1) {
            printf("Error: Cannot read original size for %s\n", entry->filename);
            continue;
        }
        
        //чтение таблицы частот
        uint32_t freq[256] = {0};
        if (fread(freq, sizeof(uint32_t), 256, in) != 256) {
            printf("Error: Cannot read frequency table for %s\n", entry->filename);
            continue;
        }
        
        // восстанавливаем дерево Хаффмана
        HuffmanNode_t *tree = generate_huffman_tree(freq);
        if (!tree) {
            printf("Error: Cannot build Huffman tree for %s\n", entry->filename);
            continue;
        }

        printf("Attempting to create file: '%s'\n", entry->filename);
        FILE *out = fopen(entry->filename, "wb");
        if (!out) {
            printf("Error: Cannot create output file '%s'\n", entry->filename);
            perror("fopen output");
            free_huffman_tree(tree);
            continue;
        }

        
        BitReader_t br;
        bitreader_init(&br, in);
        decompress_file(&br, tree, out, original_size);

        fclose(out);
        free_huffman_tree(tree);
    }

    free(entries);
    fclose(in);
    printf("Archive extraction completed.\n");
}

void print_progress_bar(size_t curr, size_t total) {
    int last_percent = -1; // чтобы избежать обновление лишних
    int percent = (int)((double)curr / total * 100); // пересчет на проценты
    
    if (percent != last_percent) {
        printf("\r%d%% [", percent); // \r сброс каретки для перезаписи прогресса (=)
        for (int i = 0; i < percent / 2; i++) {
            putchar('=');
        }
        for (int i = percent / 2; i < 50; i++) { // оставшиееся место
            putchar(' ');
        }
        printf("] %zu/%zu bytes total", curr, total);
        fflush(stdout);
        last_percent = percent;
    }
    
    if (curr == total) {
        printf("\n");
    }
}

void print_stats(const char* filename, size_t original, size_t compressed) {
    double ratio = (double)compressed / original * 100;
    printf("File: %s\n", filename);
    printf("  Original size: %zu bytes\n", original);
    printf("  Compressed size: %zu bytes\n", compressed);
    printf("  Compression ratio: %.2f%%\n", ratio);
    printf("  Space saved: %.2f%%\n", 100.0 - ratio);
}