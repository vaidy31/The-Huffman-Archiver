#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H
// функции для постройки дерева кодов Хаффмана
#include "huffman_types.h"


HuffmanNode_t* generate_huffman_tree(unsigned int freq[256]);
void generate_codes(HuffmanNode_t* root, HuffmanCode_t code_table[256], unsigned int curr_bits, int depth_recursion);
void free_huffman_tree(HuffmanNode_t* node);
int cmp(const void *a, const void *b);

#endif