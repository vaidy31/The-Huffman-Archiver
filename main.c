#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcmpressor.h"

void print_help(const char *program_name) {
    printf("Huffman Archiver - архиватор на алгоритме Хаффмана\n\n");
    printf("Использование:\n");
    printf("  %s --compress <архив.huf> <файлы...>\n", program_name);
    printf("  %s --decompress <архив.huf>\n", program_name);
    printf("  %s --help\n\n", program_name);
    
    printf("Примеры:\n");
    printf("  %s --compress data.huf file1.txt file2.jpg\n", program_name);
    printf("  %s --decompress data.huf\n", program_name);
}

void print_usage_error(const char *program_name) {
    fprintf(stderr, "Ошибка: неверные аргументы командной строки.\n\n");
    fprintf(stderr, "Использование:\n");
    fprintf(stderr, "  %s --compress <архив.huf> <файл1> [файл2] ...\n", program_name);
    fprintf(stderr, "  %s --decompress <архив.huf>\n", program_name);
    fprintf(stderr, "  %s --help\n\n", program_name);
    fprintf(stderr, "Для получения подробной справки используйте: %s --help\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage_error(argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        return 0;
    }
    if (strcmp(argv[1], "--compress") == 0 || (strcmp(argv[1], "--c") == 0)) {
        if (argc < 4) {
            fprintf(stderr, "Ошибка: для сжатия необходимо указать имя архива и хотя бы один файл.\n");
            print_usage_error(argv[0]);
            return 1;
        }
        
        printf("=HUFFMAN ARCHIVER - РЕЖИМ СЖАТИЯ=\n");
        compress_files(&argv[3], argc - 3, argv[2]);
        return 0;
    }
    
    if (strcmp(argv[1], "--decompress") == 0 || (strcmp(argv[1], "--d") == 0)) {
        if (argc != 3) {
            fprintf(stderr, "Ошибка: для распаковки необходимо указать только имя архива.\n");
            print_usage_error(argv[0]);
            return 1;
        }
        
        printf("=HUFFMAN ARCHIVER - РЕЖИМ РАСПАКОВКИ=\n");
        decompress_archive(argv[2]);
        return 0;
    }
    fprintf(stderr, "Ошибка: неизвестная команда '%s'.\n", argv[1]);
    print_usage_error(argv[0]);
    return 1;
}