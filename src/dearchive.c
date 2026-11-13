#include <stdint.h>
#include <stdio.h>

#include "bit_io.h"
#include "format.h"
#include "huffman.h"
#include "status.h"

#include "dearchive.h"


/**
 * Reads bits and walks the tree from the root down to a leaf.
 * @param reader Bit reader.
 * @param root   Root of the tree, must not be NULL.
 * @param symbol [out] Symbol stored in the reached leaf.
 * @return HUF_OK on success, error code otherwise.
 */
static HufStatus decode_symbol(BitReader* reader, const Node* root, uint8_t* symbol)
{
    const Node* node = root;
    uint8_t bit = 0;
    HufStatus status = HUF_OK;

    // With only one symbol the root is a leaf, but every symbol is still stored as one bit.
    if (huffman_is_leaf(root))
    {
        status = bit_reader_read_bit(reader, &bit);

        if (status != HUF_OK)
        {
            return status;
        }
    }

    while (!huffman_is_leaf(node))
    {
        status = bit_reader_read_bit(reader, &bit);

        if (status != HUF_OK)
        {
            return status;
        }

        if (bit == 0)
        {
            node = node->left;
        }
        else
        {
            node = node->right;
        }
    }

    *symbol = node->symbol;
    return HUF_OK;
}

/**
 * Decodes the compressed data that follows the header.
 * @param input      Archive, positioned right after the header.
 * @param output     Output file.
 * @param root       Root of the tree, NULL only if total_size is 0.
 * @param total_size Number of bytes to decode.
 * @param data_size  [out] Number of bytes of compressed data.
 * @return HUF_OK on success, error code otherwise.
 */
static HufStatus decode_data
(
      FILE* input
    , FILE* output
    , const Node* root
    , uint64_t total_size
    , uint64_t* data_size
)
{
    BitReader reader;
    HufStatus status = HUF_OK;

    bit_reader_init(&reader, input);

    for (uint64_t i = 0; i < total_size && status == HUF_OK; i++)
    {
        uint8_t symbol = 0;

        status = decode_symbol(&reader, root, &symbol);

        if (status == HUF_OK && fputc(symbol, output) == EOF)
        {
            status = HUF_ERR_WRITE;
        }
    }

    if (status != HUF_OK)
    {
        return status;
    }

    if (fgetc(input) != EOF)
    {
        return HUF_ERR_BAD_ARCHIVE;
    }

    if (ferror(input))
    {
        return HUF_ERR_READ;
    }

    *data_size = reader.bytes_read;
    return HUF_OK;
}

/**
 * Decompresses an already opened archive.
 * @param input  Archive.
 * @param output Output file.
 * @param stats  [out] Sizes of the restored data, compressed data and header.
 * @return HUF_OK on success, error code otherwise.
 */
static HufStatus decompress(FILE* input, FILE* output, ArchiveStats* stats)
{
    uint64_t frequencies[SYMBOL_COUNT];
    Node* root = NULL;
    uint64_t total_size = 0;
    uint64_t header_size = 0;
    uint64_t data_size = 0;

    HufStatus status = format_read_header(input, frequencies, &total_size, &header_size);

    if (status != HUF_OK)
    {
        return status;
    }

    status = huffman_build_tree(frequencies, &root);

    if (status != HUF_OK)
    {
        return status;
    }

    status = decode_data(input, output, root, total_size, &data_size);
    huffman_free_tree(root);

    if (status != HUF_OK)
    {
        return status;
    }

    stats->original_size = total_size;
    stats->data_size = data_size;
    stats->extra_size = header_size;
    return HUF_OK;
}

HufStatus dearchive_file(const char* input_path, const char* output_path, ArchiveStats* stats)
{
    FILE* input = fopen(input_path, "rb");

    if (input == NULL)
    {
        return HUF_ERR_OPEN_INPUT;
    }

    FILE* output = fopen(output_path, "wb");

    if (output == NULL)
    {
        fclose(input);
        return HUF_ERR_OPEN_OUTPUT;
    }

    HufStatus status = decompress(input, output, stats);

    fclose(input);

    if (fclose(output) != 0 && status == HUF_OK)
    {
        status = HUF_ERR_WRITE;
    }

    if (status != HUF_OK)
    {
        remove(output_path);
    }

    return status;
}
