#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "status.h"

#include "huffman.h"

#define READ_BUFFER_SIZE 4096


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
