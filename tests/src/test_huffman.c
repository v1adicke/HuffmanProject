#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <cmocka.h>

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
 * Counts how many bits all symbols take with the given codes.
 * @param frequencies Array of SYMBOL_COUNT counters.
 * @param codes       Array of SYMBOL_COUNT codes.
 * @return Total number of bits.
 */
static uint64_t count_bits(const uint64_t* frequencies, const Code* codes)
{
    uint64_t bits = 0;

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        bits += frequencies[i] * codes[i].length;
    }

    return bits;
}

/**
 * Builds a tree and a code table from frequencies.
 * @param frequencies Array of SYMBOL_COUNT counters.
 * @param codes       [out] Array of SYMBOL_COUNT codes.
 */
static void build_codes(const uint64_t* frequencies, Code* codes)
{
    Node* root = NULL;

    assert_int_equal(huffman_build_tree(frequencies, &root), HUF_OK);
    huffman_build_codes(root, codes);
    huffman_free_tree(root);
}

/**
 * Checks that counting works on a small file.
 */
static void test_count_frequencies(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT];
    uint64_t total = 0;
    FILE* file = create_file("aaab", 4);

    assert_int_equal(huffman_count_frequencies(file, frequencies, &total), HUF_OK);
    assert_int_equal(total, 4);
    assert_int_equal(frequencies['a'], 3);
    assert_int_equal(frequencies['b'], 1);
    assert_int_equal(frequencies['c'], 0);
    fclose(file);
}

/**
 * Checks that an empty file gives zero counters.
 */
static void test_count_frequencies_empty(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT];
    uint64_t total = 1;
    FILE* file = create_file("", 0);

    assert_int_equal(huffman_count_frequencies(file, frequencies, &total), HUF_OK);
    assert_int_equal(total, 0);

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        assert_int_equal(frequencies[i], 0);
    }

    fclose(file);
}

/**
 * Checks that no symbols give an empty tree.
 */
static void test_build_tree_empty(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Node not_null;
    Node* root = &not_null;

    assert_int_equal(huffman_build_tree(frequencies, &root), HUF_OK);
    assert_null(root);
}

/**
 * Checks that the root keeps the sum of all frequencies.
 */
static void test_build_tree_root_frequency(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Node* root = NULL;

    frequencies['x'] = 5;
    frequencies['y'] = 7;
    frequencies['z'] = 1;

    assert_int_equal(huffman_build_tree(frequencies, &root), HUF_OK);
    assert_non_null(root);
    assert_int_equal(root->frequency, 13);
    assert_false(huffman_is_leaf(root));
    huffman_free_tree(root);
}

/**
 * Checks that a single symbol gets a one bit code.
 */
static void test_codes_single_symbol(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Code codes[SYMBOL_COUNT];

    frequencies['q'] = 100;
    build_codes(frequencies, codes);

    assert_int_equal(codes['q'].length, 1);
    assert_int_equal(codes['q'].bits[0], 0);
    assert_int_equal(codes['a'].length, 0);
}

/**
 * Checks the code lengths on the example from Wikipedia, where the best result is 224 bits.
 */
static void test_codes_are_optimal(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Code codes[SYMBOL_COUNT];

    frequencies['a'] = 45;
    frequencies['b'] = 13;
    frequencies['c'] = 12;
    frequencies['d'] = 16;
    frequencies['e'] = 9;
    frequencies['f'] = 5;
    build_codes(frequencies, codes);

    assert_int_equal(count_bits(frequencies, codes), 224);
    assert_int_equal(codes['a'].length, 1);
}

/**
 * Checks that no code is the beginning of another code.
 */
static void test_codes_are_prefix_free(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Code codes[SYMBOL_COUNT];

    for (size_t i = 0; i < 40; i++)
    {
        frequencies[i] = i * i + 1;
    }

    build_codes(frequencies, codes);

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        for (size_t j = 0; j < SYMBOL_COUNT; j++)
        {
            if (i != j && codes[i].length > 0 && codes[i].length <= codes[j].length)
            {
                assert_memory_not_equal(codes[i].bits, codes[j].bits, codes[i].length);
            }
        }
    }
}

/**
 * Checks that equal frequencies of all bytes give 8 bit codes.
 */
static void test_codes_uniform(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT];
    Code codes[SYMBOL_COUNT];

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        frequencies[i] = 10;
    }

    build_codes(frequencies, codes);

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        assert_int_equal(codes[i].length, 8);
    }
}

/**
 * Checks that the same frequencies always give the same codes, the dearchiver depends on it.
 */
static void test_codes_are_repeatable(void** state)
{
    (void) state;
    uint64_t frequencies[SYMBOL_COUNT] = { 0 };
    Code first[SYMBOL_COUNT];
    Code second[SYMBOL_COUNT];

    for (size_t i = 0; i < SYMBOL_COUNT; i += 3)
    {
        frequencies[i] = i % 7 + 1;
    }

    build_codes(frequencies, first);
    build_codes(frequencies, second);

    for (size_t i = 0; i < SYMBOL_COUNT; i++)
    {
        assert_int_equal(first[i].length, second[i].length);
        assert_memory_equal(first[i].bits, second[i].bits, first[i].length);
    }
}

int run_huffman_tests(void)
{
    const struct CMUnitTest tests[] =
    {
          cmocka_unit_test(test_count_frequencies)
        , cmocka_unit_test(test_count_frequencies_empty)
        , cmocka_unit_test(test_build_tree_empty)
        , cmocka_unit_test(test_build_tree_root_frequency)
        , cmocka_unit_test(test_codes_single_symbol)
        , cmocka_unit_test(test_codes_are_optimal)
        , cmocka_unit_test(test_codes_are_prefix_free)
        , cmocka_unit_test(test_codes_uniform)
        , cmocka_unit_test(test_codes_are_repeatable)
    };

    return cmocka_run_group_tests_name("huffman", tests, NULL, NULL);
}
