#ifndef COMPRESSOR_H
#define COMPRESSOR_H
// для компресси и декомпресси. вспомогательные функции для прогресс бара
#include "huffman_types.h"
#include "bit_inp_out.h"

void decompress_file(BitReader_t *br, HuffmanNode_t *root, FILE *out, size_t compressed_size);
void decompress_archive(const char *archive_name);
int compress_file_n_write(FILE *archive, const char *input, ArchiveEntry_t *entry);
void compress_files(char **input_files, int file_count, const char *archive_name);
void print_progress_bar(size_t curr, size_t total);
void print_stats(const char* filename, size_t original, size_t compressed);

#endif