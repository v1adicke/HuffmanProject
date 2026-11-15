#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include "cli.h"
#include "status.h"

#include "test_suites.h"


/**
 * Counts arguments in an array that ends with NULL, like a real argv.
 * @param argv Arguments.
 * @return Number of arguments before NULL.
 */
static int count_args(char** argv)
{
    int count = 0;

    while (argv[count] != NULL)
    {
        count++;
    }

    return count;
}

/**
 * Checks that short options for archiving are parsed.
 */
static void test_parse_archive(void** state)
{
    (void) state;
    char* argv[] = { "huffman", "-a", "-f", "in.txt", "-o", "out.huf", NULL };
    CliOptions options;

    assert_int_equal(cli_parse(count_args(argv), argv, &options), HUF_OK);
    assert_true(options.archive);
    assert_false(options.dearchive);
    assert_false(options.help);
    assert_string_equal(options.input_path, "in.txt");
    assert_string_equal(options.output_path, "out.huf");
}

/**
 * Checks that long options for dearchiving are parsed in any order.
 */
static void test_parse_dearchive_long(void** state)
{
    (void) state;
    char* argv[] = { "huffman", "--output", "out.txt", "--file", "in.huf", "--dearchive", NULL };
    CliOptions options;

    assert_int_equal(cli_parse(count_args(argv), argv, &options), HUF_OK);
    assert_false(options.archive);
    assert_true(options.dearchive);
    assert_string_equal(options.input_path, "in.huf");
    assert_string_equal(options.output_path, "out.txt");
}

/**
 * Checks that help does not need other options.
 */
static void test_parse_help(void** state)
{
    (void) state;
    char* short_help[] = { "huffman", "-h", NULL };
    char* long_help[] = { "huffman", "-a", "--help", NULL };
    CliOptions options;

    assert_int_equal(cli_parse(count_args(short_help), short_help, &options), HUF_OK);
    assert_true(options.help);

    assert_int_equal(cli_parse(count_args(long_help), long_help, &options), HUF_OK);
    assert_true(options.help);
}

/**
 * Checks that wrong arguments are rejected.
 */
static void test_wrong_arguments(void** state)
{
    (void) state;
    char* no_arguments[] = { "huffman", NULL };
    char* no_mode[] = { "huffman", "-f", "in", "-o", "out", NULL };
    char* both_modes[] = { "huffman", "-a", "-d", "-f", "in", "-o", "out", NULL };
    char* no_input[] = { "huffman", "-a", "-o", "out", NULL };
    char* no_output[] = { "huffman", "-a", "-f", "in", NULL };
    char* no_value[] = { "huffman", "-a", "-o", "out", "-f", NULL };
    char* empty_value[] = { "huffman", "-a", "-f", "", "-o", "out", NULL };
    char* repeated[] = { "huffman", "-a", "-f", "in", "-f", "in", "-o", "out", NULL };
    char* unknown[] = { "huffman", "-a", "-x", "-f", "in", "-o", "out", NULL };
    char* same_file[] = { "huffman", "-a", "-f", "file", "-o", "file", NULL };
    CliOptions options;

    assert_int_equal(cli_parse(count_args(no_arguments), no_arguments, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(no_mode), no_mode, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(both_modes), both_modes, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(no_input), no_input, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(no_output), no_output, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(no_value), no_value, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(empty_value), empty_value, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(repeated), repeated, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(unknown), unknown, &options), HUF_ERR_ARGS);
    assert_int_equal(cli_parse(count_args(same_file), same_file, &options), HUF_ERR_ARGS);
}

/**
 * Checks that the options are not changed when parsing fails.
 */
static void test_options_untouched_on_error(void** state)
{
    (void) state;
    char* argv[] = { "huffman", "-a", "-d", "-f", "in", "-o", "out", NULL };
    CliOptions options = { false, false, false, NULL, NULL };

    assert_int_equal(cli_parse(count_args(argv), argv, &options), HUF_ERR_ARGS);
    assert_false(options.archive);
    assert_null(options.input_path);
}

int run_cli_tests(void)
{
    const struct CMUnitTest tests[] =
    {
          cmocka_unit_test(test_parse_archive)
        , cmocka_unit_test(test_parse_dearchive_long)
        , cmocka_unit_test(test_parse_help)
        , cmocka_unit_test(test_wrong_arguments)
        , cmocka_unit_test(test_options_untouched_on_error)
    };

    return cmocka_run_group_tests_name("cli", tests, NULL, NULL);
}
