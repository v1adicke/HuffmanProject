#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "status.h"

#include "huffman.h"


HufStatus huffman_count_frequencies(FILE* input, uint64_t* frequencies, uint64_t* total_size)
{
    uint8_t buffer[READ_BUFFER_SIZE];
    uint64_t total = 0;
    size_t read_count = 0;

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        frequencies[i] = 0;
    }

    do
    {
        read_count = fread(buffer, 1, READ_BUFFER_SIZE, input);

        for (size_t i = 0; i < read_count; i++)
        {
            frequencies[buffer[i]]++;
        }

        total += read_count;
    }
    while (read_count == READ_BUFFER_SIZE);

    if (ferror(input))
    {
        return HUF_ERR_READ;
    }

    *total_size = total;
    return HUF_OK;
}

/**
 * Allocates a new tree node.
 * @param frequency Frequency of the node.
 * @param symbol    Symbol of the node, used only for leaves.
 * @param left      Left child or NULL.
 * @param right     Right child or NULL.
 * @return Pointer to the new node, NULL if allocation failed.
 */
static Node* create_node(uint64_t frequency, uint8_t symbol, Node* left, Node* right)
{
    Node* node = malloc(sizeof(Node));

    if (node == NULL)
    {
        return NULL;
    }

    node->frequency = frequency;
    node->symbol = symbol;
    node->left = left;
    node->right = right;
    return node;
}

/**
 * Checks whether a node is a leaf.
 * @param node Tree node.
 * @return true if the node has no children.
 */
static bool is_leaf(const Node* node)
{
    return node->left == NULL && node->right == NULL;
}

/**
 * Removes the node with the smallest frequency from the array.
 * If several nodes have the same frequency, the first one is taken.
 * @param nodes Array of nodes.
 * @param count [in,out] Number of nodes in the array, must be greater than 0.
 * @return The removed node.
 */
static Node* take_min(Node** nodes, size_t* count)
{
    size_t min_index = 0;

    for (size_t i = 1; i < *count; i++)
    {
        if (nodes[i]->frequency < nodes[min_index]->frequency)
        {
            min_index = i;
        }
    }

    Node* min_node = nodes[min_index];
    nodes[min_index] = nodes[*count - 1];
    *count -= 1;
    return min_node;
}

HufStatus huffman_build_tree(const uint64_t* frequencies, Node** out_root)
{
    Node* nodes[SYMBOL_COUNT];
    size_t count = 0;

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        if (frequencies[i] > 0)
        {
            nodes[count] = create_node(frequencies[i], (uint8_t) i, NULL, NULL);

            if (nodes[count] == NULL)
            {
                for (size_t j = 0; j < count; j++)
                {
                    huffman_free_tree(nodes[j]);
                }

                return HUF_ERR_NO_MEMORY;
            }

            count++;
        }
    }

    while (count > 1)
    {
        Node* first = take_min(nodes, &count);
        Node* second = take_min(nodes, &count);
        Node* parent = create_node(first->frequency + second->frequency, 0, first, second);

        if (parent == NULL)
        {
            huffman_free_tree(first);
            huffman_free_tree(second);

            for (size_t j = 0; j < count; j++)
            {
                huffman_free_tree(nodes[j]);
            }

            return HUF_ERR_NO_MEMORY;
        }

        nodes[count] = parent;
        count++;
    }

    if (count == 0)
    {
        *out_root = NULL;
    }
    else
    {
        *out_root = nodes[0];
    }

    return HUF_OK;
}

void huffman_free_tree(Node* root)
{
    if (root == NULL)
    {
        return;
    }

    huffman_free_tree(root->left);
    huffman_free_tree(root->right);
    free(root);
}

/**
 * Walks the tree and writes the path to every leaf into the code table.
 * @param node  Current node.
 * @param path  Bits of the path from the root to the current node.
 * @param depth Length of the path.
 * @param codes [out] Array of SYMBOL_COUNT codes.
 */
static void fill_codes(const Node* node, uint8_t* path, size_t depth, Code* codes)
{
    if (is_leaf(node))
    {
        memcpy(codes[node->symbol].bits, path, depth);
        codes[node->symbol].length = depth;
        return;
    }

    path[depth] = 0;
    fill_codes(node->left, path, depth + 1, codes);

    path[depth] = 1;
    fill_codes(node->right, path, depth + 1, codes);
}

void huffman_build_codes(const Node* root, Code* codes)
{
    uint8_t path[MAX_CODE_LENGTH];

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        codes[i].length = 0;
    }

    if (root == NULL)
    {
        return;
    }

    // A single leaf would get an empty code, so it needs at least one bit.
    if (is_leaf(root))
    {
        codes[root->symbol].bits[0] = 0;
        codes[root->symbol].length = 1;
        return;
    }

    fill_codes(root, path, 0, codes);
}
