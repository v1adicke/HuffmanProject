#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "bit_io.h"
#include "format.h"
#include "huffman.h"
#include "status.h"

#include "archive.h"


/**
 * Reads the input again from the start and writes the code of every byte.
 * @param input       Input file.
 * @param output      Output file, positioned right after the header.
 * @param codes       Code table.
 * @param source_size Size of the input counted on the first pass.
 * @param data_size   [out] Number of bytes of compressed data.
 * @return HUF_OK on success, error code otherwise.
 */
static HufStatus encode_data
(
      FILE* input
    , FILE* output
    , const Code* codes
    , uint64_t source_size
    , uint64_t* data_size
)
{
    uint8_t buffer[READ_BUFFER_SIZE];
    uint64_t total = 0;
    size_t read_count = 0;
    BitWriter writer;
    HufStatus status = HUF_OK;

    if (fseek(input, 0, SEEK_SET) != 0)
    {
        return HUF_ERR_READ;
    }

    bit_writer_init(&writer, output);

    do
    {
        read_count = fread(buffer, 1, READ_BUFFER_SIZE, input);
        total += read_count;

        for (size_t i = 0; i < read_count && status == HUF_OK; i++)
        {
            const Code* code = &codes[buffer[i]];

            for (size_t j = 0; j < code->length && status == HUF_OK; j++)
            {
                status = bit_writer_write_bit(&writer, code->bits[j]);
            }
        }
    }
    while (read_count == READ_BUFFER_SIZE && status == HUF_OK);

    if (status != HUF_OK)
    {
        return status;
    }

    // The file could change between the two passes, then the codes do not fit it.
    if (ferror(input) || total != source_size)
    {
        return HUF_ERR_READ;
    }

    status = bit_writer_flush(&writer);

    if (status != HUF_OK)
    {
        return status;
    }

    *data_size = writer.bytes_written;
    return HUF_OK;
}

/**
 * Compresses an already opened file.
 * @param input  Input file.
 * @param output Output file.
 * @param stats  [out] Sizes of the source data, compressed data and header.
 * @return HUF_OK on success, error code otherwise.
 */
static HufStatus compress(FILE* input, FILE* output, ArchiveStats* stats)
{
    uint64_t frequencies[SYMBOL_COUNT];
    Code codes[SYMBOL_COUNT];
    Node* root = NULL;
    uint64_t source_size = 0;
    uint64_t header_size = 0;
    uint64_t data_size = 0;

    HufStatus status = huffman_count_frequencies(input, frequencies, &source_size);

    if (status != HUF_OK)
    {
        return status;
    }

    status = huffman_build_tree(frequencies, &root);

    if (status != HUF_OK)
    {
        return status;
    }

    huffman_build_codes(root, codes);
    huffman_free_tree(root);

    status = format_write_header(output, frequencies, &header_size);

    if (status != HUF_OK)
    {
        return status;
    }

    status = encode_data(input, output, codes, source_size, &data_size);

    if (status != HUF_OK)
    {
        return status;
    }

    stats->original_size = source_size;
    stats->data_size = data_size;
    stats->extra_size = header_size;
    return HUF_OK;
}

HufStatus archive_file(const char* input_path, const char* output_path, ArchiveStats* stats)
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

    HufStatus status = compress(input, output, stats);

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
