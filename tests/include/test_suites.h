#ifndef TEST_SUITES_H
#define TEST_SUITES_H


/**
 * Runs unit tests for the Huffman tree and codes.
 * @return Number of failed tests.
 */
int run_huffman_tests(void);

/**
 * Runs unit tests for the bit reader and writer.
 * @return Number of failed tests.
 */
int run_bit_io_tests(void);

/**
 * Runs unit tests for reading and writing the archive header.
 * @return Number of failed tests.
 */
int run_format_tests(void);

/**
 * Runs unit tests for parsing command line arguments.
 * @return Number of failed tests.
 */
int run_cli_tests(void);

#endif // TEST_SUITES_H
