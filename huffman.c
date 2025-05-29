#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

int cmp(const void *a, const void *b) {
    return (*(HuffmanNode_t**)a)->freq - (*(HuffmanNode_t**)b)->freq;
}

HuffmanNode_t* generate_huffman_tree(unsigned int freq[256]) {
    HuffmanNode_t *leaf_nodes[256]; // массив для листьев
    int leaf_counter = 0; 

    // если частота больше 1, создаем узел
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffmanNode_t* obj = malloc(sizeof(HuffmanNode_t));
            obj->symbol = (unsigned char)i; // добавляем символ используя его код
            obj->freq = freq[i]; // количество
            obj->left = NULL;
            obj->right = NULL;
            leaf_nodes[leaf_counter++] = obj; // добавялем в массив листьев
        }
    }
    
    if (leaf_counter == 0) {
        return NULL;
    }
    else if (leaf_counter == 1) {
        // добавляем фейковый лист и связываем сразу
        HuffmanNode_t* fake = malloc(sizeof(HuffmanNode_t));
        fake->symbol = 0;
        fake->freq = 0;
        fake->left = NULL;
        fake->right = NULL;

        HuffmanNode_t* parent = malloc(sizeof(HuffmanNode_t));
        parent->symbol = 0;
        parent->freq = leaf_nodes[0]->freq;
        parent->left = leaf_nodes[0];
        parent->right = fake;

        return parent;
    }
    
    // сортируем листья по частоте
    qsort(leaf_nodes, leaf_counter, sizeof(HuffmanNode_t*), cmp);

    HuffmanNode_t* inner_nodes[256]; // массив внтуренних узлов
    int inner_counter = 0;
    int in = 0, out = 0; // указатели для 2х массивов, т.е проходим по 2 разным и берем только наименьшее

    // строим дерево Хаффмана
    while ((inner_counter - in) + (leaf_counter - out) > 1) {
        HuffmanNode_t *temp_A;
        if (inner_counter > in && (leaf_counter <= out || inner_nodes[in]->freq <= leaf_nodes[out]->freq)) {
            temp_A = inner_nodes[in++];
        } else {
            temp_A = leaf_nodes[out++];
        }

        HuffmanNode_t *temp_B;
        if (inner_counter > in && (leaf_counter <= out || inner_nodes[in]->freq <= leaf_nodes[out]->freq)) {
            temp_B = inner_nodes[in++];
        } else {
            temp_B = leaf_nodes[out++];
        }

        HuffmanNode_t *parent = malloc(sizeof(HuffmanNode_t));
        parent->symbol = 0;
        parent->freq = temp_A->freq + temp_B->freq;
        parent->left = temp_A;
        parent->right = temp_B;
        inner_nodes[inner_counter++] = parent;
    }

    if (out < leaf_counter) {
        return leaf_nodes[out]; // последний узел является корнем
    } else {
        return inner_nodes[in];
    }
}

void generate_codes(HuffmanNode_t* obj, HuffmanCode_t code_table[256], unsigned int curr_bits, int depth_recursion) {
    if (!obj) {
        return;
    }

    // Листовой узел - сохраняем код
    //     0
    //   /   \     //
    //  b     s
    if (!obj->left && !obj->right) {
        code_table[obj->symbol].bit_code = curr_bits;
        code_table[obj->symbol].length = depth_recursion;
        return;
    }

    // Рекурсивно идём влево (0)
    generate_codes(obj->left, code_table, curr_bits << 1, depth_recursion + 1);

    // Рекурсивно идём вправо (1)
    generate_codes(obj->right, code_table, (curr_bits << 1) | 1, depth_recursion + 1);
}

void free_huffman_tree(HuffmanNode_t* node) {
    if (!node) return;
    free_huffman_tree(node->left);
    free_huffman_tree(node->right);
    free(node);
}