#include <stdint.h>
#include <stdio.h>

#include "status.h"

#include "bit_io.h"


void bit_writer_init(BitWriter* writer, FILE* file)
{
    writer->file = file;
    writer->buffer = 0;
    writer->bit_count = 0;
    writer->bytes_written = 0;
}

HufStatus bit_writer_write_bit(BitWriter* writer, uint8_t bit)
{
    writer->buffer = (uint8_t) ((writer->buffer << 1) | bit);
    writer->bit_count++;

    if (writer->bit_count == 8)
    {
        if (fputc(writer->buffer, writer->file) == EOF)
        {
            return HUF_ERR_WRITE;
        }

        writer->bytes_written++;
        writer->buffer = 0;
        writer->bit_count = 0;
    }

    return HUF_OK;
}

HufStatus bit_writer_flush(BitWriter* writer)
{
    while (writer->bit_count != 0)
    {
        HufStatus status = bit_writer_write_bit(writer, 0);

        if (status != HUF_OK)
        {
            return status;
        }
    }

    return HUF_OK;
}

void bit_reader_init(BitReader* reader, FILE* file)
{
    reader->file = file;
    reader->buffer = 0;
    reader->bits_left = 0;
    reader->bytes_read = 0;
}

HufStatus bit_reader_read_bit(BitReader* reader, uint8_t* bit)
{
    if (reader->bits_left == 0)
    {
        int byte = fgetc(reader->file);

        if (byte == EOF)
        {
            if (ferror(reader->file))
            {
                return HUF_ERR_READ;
            }

            return HUF_ERR_BAD_ARCHIVE;
        }

        reader->buffer = (uint8_t) byte;
        reader->bits_left = 8;
        reader->bytes_read++;
    }

    reader->bits_left--;
    *bit = (reader->buffer >> reader->bits_left) & 1;
    return HUF_OK;
}
