#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "huffman.h"
#include "status.h"

#include "format.h"


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

    status = write_number(output, symbol_count, 2);

    for (size_t i = 0; i < SYMBOL_COUNT && status == HUF_OK; i++)
    {
        if (frequencies[i] > 0)
        {
            status = write_number(output, i, 1);

            if (status == HUF_OK)
            {
                status = write_number(output, frequencies[i], 8);
            }
        }
    }

    if (status != HUF_OK)
    {
        return status;
    }

    *header_size = ARCHIVE_MAGIC_SIZE + 2 + symbol_count * 9;
    return HUF_OK;
}
