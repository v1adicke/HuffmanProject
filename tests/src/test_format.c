#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "format.h"
#include "huffman.h"
#include "status.h"

#include "test_suites.h"


/**
 * Creates a temporary file with the given content and moves to its start.
 * @param content Bytes to write.
 * @param size    Number of bytes.
 * @return Opened file, must be closed by the caller.
 */
static FILE* create_file(const char* content, size_t size)
{
    FILE* file = tmpfile();

    assert_non_null(file);
    assert_int_equal(fwrite(content, 1, size, file), size);
    rewind(file);
    return file;
}

/**
 * Reads a header from the given bytes and returns the result.
 * @param content Bytes of the header.
 * @param size    Number of bytes.
 * @return Status returned by format_read_header.
 */
static HufStatus read_header_from(const char* content, size_t size)
{
    uint64_t frequencies[SYMBOL_COUNT];
    uint64_t total = 0;
    uint64_t header_size = 0;
    FILE* file = create_file(content, size);

    HufStatus status = format_read_header(file, frequencies, &total, &header_size);

    fclose(file);
    return status;
}

/**
 * Checks that a written header is read back with the same frequencies.
 */
static void test_header_roundtrip(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    uint64_t read_frequencies[SYMBOL_COUNT];
    uint64_t written_size = 0;
    uint64_t read_size = 0;
    uint64_t total = 0;
    FILE* file = tmpfile();

    assert_non_null(file);
    frequencies[0] = 1;
    frequencies['a'] = 300;
    frequencies[255] = 5000000000;

    assert_int_equal(format_write_header(file, frequencies, &written_size), HUF_OK);
    assert_int_equal(written_size, 6 + 3 * 9);
    assert_int_equal(ftell(file), written_size);

    rewind(file);
    assert_int_equal(format_read_header(file, read_frequencies, &total, &read_size), HUF_OK);
    assert_memory_equal(frequencies, read_frequencies, sizeof(frequencies));
    assert_int_equal(total, 5000000301);
    assert_int_equal(read_size, written_size);
    fclose(file);
}

/**
 * Checks the header of an empty file.
 */
static void test_header_empty(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    uint64_t header_size = 0;
    uint64_t total = 1;
    FILE* file = tmpfile();

    assert_non_null(file);
    assert_int_equal(format_write_header(file, frequencies, &header_size), HUF_OK);
    assert_int_equal(header_size, 6);

    rewind(file);
    assert_int_equal(format_read_header(file, frequencies, &total, &header_size), HUF_OK);
    assert_int_equal(total, 0);
    fclose(file);
}

/**
 * Checks that writing to a file opened only for reading is an error.
 */
static void test_write_error(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    uint64_t header_size = 0;
    FILE* file = fopen("/dev/null", "rb");

    assert_non_null(file);
    assert_int_equal(format_write_header(file, frequencies, &header_size), HUF_ERR_WRITE);
    fclose(file);
}

/**
 * Checks headers that must be rejected.
 */
static void test_broken_headers(void** state)
{
    (void) state;

    assert_int_equal(read_header_from("", 0), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from("HUF", 3), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from("ZIPP\0\0", 6), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from("HUFF\1", 5), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from("HUFF\1\1", 6), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from("HUFF\1\0a\1\0\0", 10), HUF_ERR_BAD_ARCHIVE);
}

/**
 * Checks that a zero frequency is rejected.
 */
static void test_zero_frequency(void** state)
{
    (void) state;
    const char header[] = "HUFF\1\0a\0\0\0\0\0\0\0\0";

    assert_int_equal(read_header_from(header, sizeof(header) - 1), HUF_ERR_BAD_ARCHIVE);
}

/**
 * Checks that symbols in the wrong order or repeated symbols are rejected.
 */
static void test_symbols_order(void** state)
{
    (void) state;
    const char sorted[] = "HUFF\2\0a\1\0\0\0\0\0\0\0b\1\0\0\0\0\0\0\0";
    const char unsorted[] = "HUFF\2\0b\1\0\0\0\0\0\0\0a\1\0\0\0\0\0\0\0";
    const char repeated[] = "HUFF\2\0a\1\0\0\0\0\0\0\0a\1\0\0\0\0\0\0\0";

    assert_int_equal(read_header_from(sorted, sizeof(sorted) - 1), HUF_OK);
    assert_int_equal(read_header_from(unsorted, sizeof(unsorted) - 1), HUF_ERR_BAD_ARCHIVE);
    assert_int_equal(read_header_from(repeated, sizeof(repeated) - 1), HUF_ERR_BAD_ARCHIVE);
}

/**
 * Checks that frequencies whose sum does not fit into 64 bits are rejected.
 */
static void test_total_overflow(void** state)
{
    (void) state;
    const char header[] = "HUFF\2\0a\377\377\377\377\377\377\377\377b\1\0\0\0\0\0\0\0";

    assert_int_equal(read_header_from(header, sizeof(header) - 1), HUF_ERR_BAD_ARCHIVE);
}

int run_format_tests(void)
{
    const struct CMUnitTest tests[] =
    {
          cmocka_unit_test(test_header_roundtrip)
        , cmocka_unit_test(test_header_empty)
        , cmocka_unit_test(test_write_error)
        , cmocka_unit_test(test_broken_headers)
        , cmocka_unit_test(test_zero_frequency)
        , cmocka_unit_test(test_symbols_order)
        , cmocka_unit_test(test_total_overflow)
    };

    return cmocka_run_group_tests_name("format", tests, NULL, NULL);
}
