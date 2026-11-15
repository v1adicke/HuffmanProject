#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "bit_io.h"
#include "status.h"

#include "test_suites.h"


/**
 * Writes bits to a new temporary file and moves to its start.
 * @param bits  Bits to write, each element is 0 or 1.
 * @param count Number of bits.
 * @return Opened file, must be closed by the caller.
 */
static FILE* write_bits(const uint8_t* bits, size_t count)
{
    FILE* file = tmpfile();
    BitWriter writer;

    assert_non_null(file);
    bit_writer_init(&writer, file);

    for (size_t i = 0; i < count; i++)
    {
        assert_int_equal(bit_writer_write_bit(&writer, bits[i]), HUF_OK);
    }

    assert_int_equal(bit_writer_flush(&writer), HUF_OK);
    rewind(file);
    return file;
}

/**
 * Checks that the first bit goes to the highest bit of a byte.
 */
static void test_write_full_byte(void** state)
{
    (void) state;
    uint8_t bits[] = { 1, 0, 1, 1, 0, 0, 0, 1 };
    FILE* file = write_bits(bits, 8);

    assert_int_equal(fgetc(file), 0xB1);
    assert_int_equal(fgetc(file), EOF);
    fclose(file);
}

/**
 * Checks that flush fills the rest of the byte with zeros.
 */
static void test_flush_adds_zeros(void** state)
{
    (void) state;
    uint8_t bits[] = { 1, 1, 1 };
    FILE* file = write_bits(bits, 3);

    assert_int_equal(fgetc(file), 0xE0);
    assert_int_equal(fgetc(file), EOF);
    fclose(file);
}

/**
 * Checks that flush without bits writes nothing.
 */
static void test_flush_empty(void** state)
{
    (void) state;
    FILE* file = write_bits(NULL, 0);

    assert_int_equal(fgetc(file), EOF);
    fclose(file);
}

/**
 * Checks that the writer counts written bytes.
 */
static void test_writer_counts_bytes(void** state)
{
    (void) state;
    FILE* file = tmpfile();
    BitWriter writer;

    assert_non_null(file);
    bit_writer_init(&writer, file);

    for (size_t i = 0; i < 20; i++)
    {
        assert_int_equal(bit_writer_write_bit(&writer, 1), HUF_OK);
    }

    assert_int_equal(writer.bytes_written, 2);
    assert_int_equal(bit_writer_flush(&writer), HUF_OK);
    assert_int_equal(writer.bytes_written, 3);
    fclose(file);
}

/**
 * Checks that the reader returns the same bits that were written.
 */
static void test_read_back(void** state)
{
    (void) state;
    uint8_t bits[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0 };
    size_t count = sizeof(bits);
    FILE* file = write_bits(bits, count);
    BitReader reader;
    uint8_t bit = 0;

    bit_reader_init(&reader, file);

    for (size_t i = 0; i < count; i++)
    {
        assert_int_equal(bit_reader_read_bit(&reader, &bit), HUF_OK);
        assert_int_equal(bit, bits[i]);
    }

    assert_int_equal(reader.bytes_read, 2);
    fclose(file);
}

/**
 * Checks that reading after the end of the file is an error.
 */
static void test_read_after_end(void** state)
{
    (void) state;
    uint8_t bits[] = { 1 };
    FILE* file = write_bits(bits, 1);
    BitReader reader;
    uint8_t bit = 0;

    bit_reader_init(&reader, file);

    for (size_t i = 0; i < 8; i++)
    {
        assert_int_equal(bit_reader_read_bit(&reader, &bit), HUF_OK);
    }

    assert_int_equal(bit_reader_read_bit(&reader, &bit), HUF_ERR_BAD_ARCHIVE);
    fclose(file);
}

int run_bit_io_tests(void)
{
    const struct CMUnitTest tests[] =
    {
          cmocka_unit_test(test_write_full_byte)
        , cmocka_unit_test(test_flush_adds_zeros)
        , cmocka_unit_test(test_flush_empty)
        , cmocka_unit_test(test_writer_counts_bytes)
        , cmocka_unit_test(test_read_back)
        , cmocka_unit_test(test_read_after_end)
    };

    return cmocka_run_group_tests_name("bit_io", tests, NULL, NULL);
}
