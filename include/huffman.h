#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "status.h"

#define SYMBOL_COUNT 256
#define MAX_CODE_LENGTH 256


/**
 * Node of the Huffman tree.
 * A node without children is a leaf and stores a symbol.
 */
typedef struct Node
{
    uint64_t frequency; ///< total frequency of all symbols in this subtree
    uint8_t symbol;     ///< byte value, used only in leaves
    struct Node* left;  ///< child for bit 0
    struct Node* right; ///< child for bit 1
} Node;

/**
 * Huffman code of one symbol.
 */
typedef struct
{
    uint8_t bits[MAX_CODE_LENGTH]; ///< code bits, each element is 0 or 1
    size_t length;                 ///< number of bits, 0 if the symbol is not used
} Code;

/**
 * Counts how many times each byte occurs in a file.
 * Reads the file from the current position to the end.
 * @param input       File opened for reading in binary mode.
 * @param frequencies [out] Array of SYMBOL_COUNT counters.
 * @param total_size  [out] Number of bytes read.
 * @return HUF_OK on success, HUF_ERR_READ if reading failed.
 */
HufStatus huffman_count_frequencies(FILE* input, uint64_t* frequencies, uint64_t* total_size);

/**
 * Builds a Huffman tree from symbol frequencies.
 * The same frequencies always give the same tree.
 * @param frequencies Array of SYMBOL_COUNT counters.
 * @param out_root    [out] Root of the tree (must be freed with huffman_free_tree), NULL if all counters are 0.
 * @return HUF_OK on success, HUF_ERR_NO_MEMORY if allocation failed.
 */
HufStatus huffman_build_tree(const uint64_t* frequencies, Node** out_root);

/**
 * Frees a Huffman tree.
 * @param root Root of the tree, may be NULL.
 */
void huffman_free_tree(Node* root);

/**
 * Fills the code table from a Huffman tree.
 * If the tree has only one symbol, it gets the code "0".
 * @param root  Root of the tree, may be NULL.
 * @param codes [out] Array of SYMBOL_COUNT codes.
 */
void huffman_build_codes(const Node* root, Code* codes);

#endif // HUFFMAN_H
