#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "huffman.h"
#include "status.h"

#include "format.h"

#define SYMBOL_COUNT_SIZE 2
#define SYMBOL_SIZE 1
#define FREQUENCY_SIZE 8


/**
 * Calculates the size of a header.
 * @param symbol_count Number of used symbols.
 * @return Header size in bytes.
 */
static uint64_t get_header_size(uint64_t symbol_count)
{
    return ARCHIVE_MAGIC_SIZE + SYMBOL_COUNT_SIZE + symbol_count * (SYMBOL_SIZE + FREQUENCY_SIZE);
}

/**
 * Writes an unsigned number in little-endian order.
 * @param output     File opened for writing in binary mode.
 * @param value      Number to write.
 * @param byte_count How many lowest bytes of the number to write.
 * @return HUF_OK on success, HUF_ERR_WRITE if writing failed.
 */
static HufStatus write_number(FILE* output, uint64_t value, size_t byte_count)
{
    for (size_t i = 0; i < byte_count; i++)
    {
        uint8_t byte = (uint8_t) ((value >> (8 * i)) & 0xFF);

        if (fputc(byte, output) == EOF)
        {
            return HUF_ERR_WRITE;
        }
    }

    return HUF_OK;
}

/**
 * Reads an unsigned number in little-endian order.
 * @param input      File opened for reading in binary mode.
 * @param byte_count How many bytes to read.
 * @param value      [out] Read number.
 * @return HUF_OK on success, HUF_ERR_BAD_ARCHIVE if the file ended, HUF_ERR_READ if reading failed.
 */
static HufStatus read_number(FILE* input, size_t byte_count, uint64_t* value)
{
    uint64_t result = 0;

    for (size_t i = 0; i < byte_count; i++)
    {
        int byte = fgetc(input);

        if (byte == EOF)
        {
            if (ferror(input))
            {
                return HUF_ERR_READ;
            }

            return HUF_ERR_BAD_ARCHIVE;
        }

        result |= (uint64_t) byte << (8 * i);
    }

    *value = result;
    return HUF_OK;
}

HufStatus format_write_header(FILE* output, const uint64_t* frequencies, uint64_t* header_size)
{
    uint64_t symbol_count = 0;
    HufStatus status = HUF_OK;

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        if (frequencies[i] > 0)
        {
            symbol_count++;
        }
    }

    if (fwrite(ARCHIVE_MAGIC, 1, ARCHIVE_MAGIC_SIZE, output) != ARCHIVE_MAGIC_SIZE)
    {
        return HUF_ERR_WRITE;
    }

    status = write_number(output, symbol_count, SYMBOL_COUNT_SIZE);

    for (size_t i = 0; i < SYMBOL_COUNT && status == HUF_OK; i++)
    {
        if (frequencies[i] > 0)
        {
            status = write_number(output, i, SYMBOL_SIZE);

            if (status == HUF_OK)
            {
                status = write_number(output, frequencies[i], FREQUENCY_SIZE);
            }
        }
    }

    if (status != HUF_OK)
    {
        return status;
    }

    *header_size = get_header_size(symbol_count);
    return HUF_OK;
}

HufStatus format_read_header(FILE* input, uint64_t* frequencies, uint64_t* total_size, uint64_t* header_size)
{
    char magic[ARCHIVE_MAGIC_SIZE];
    uint64_t symbol_count = 0;
    uint64_t total = 0;
    uint64_t next_symbol = 0;
    HufStatus status = HUF_OK;

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        frequencies[i] = 0;
    }

    if (fread(magic, 1, ARCHIVE_MAGIC_SIZE, input) != ARCHIVE_MAGIC_SIZE)
    {
        if (ferror(input))
        {
            return HUF_ERR_READ;
        }

        return HUF_ERR_BAD_ARCHIVE;
    }

    if (memcmp(magic, ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE) != 0)
    {
        return HUF_ERR_BAD_ARCHIVE;
    }

    status = read_number(input, SYMBOL_COUNT_SIZE, &symbol_count);

    if (status != HUF_OK)
    {
        return status;
    }

    if (symbol_count > SYMBOL_COUNT)
    {
        return HUF_ERR_BAD_ARCHIVE;
    }

    for (uint64_t i = 0; i < symbol_count; i++)
    {
        uint64_t symbol = 0;
        uint64_t frequency = 0;

        status = read_number(input, SYMBOL_SIZE, &symbol);

        if (status == HUF_OK)
        {
            status = read_number(input, FREQUENCY_SIZE, &frequency);
        }

        if (status != HUF_OK)
        {
            return status;
        }

        // Symbols are written in increasing order, so a repeated or smaller symbol means a broken header.
        if (symbol < next_symbol || frequency == 0 || frequency > UINT64_MAX - total)
        {
            return HUF_ERR_BAD_ARCHIVE;
        }

        frequencies[symbol] = frequency;
        total += frequency;
        next_symbol = symbol + 1;
    }

    *total_size = total;
    *header_size = get_header_size(symbol_count);
    return HUF_OK;
}
