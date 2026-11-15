#!/bin/bash

HUFFMAN=./huffman
ASSETS_DIR=tests/assets
BROKEN_DIR=tests/assets/broken
TMP_DIR=$(mktemp -d)
LEAK_EXIT_CODE=100
passed=0
failed=0

trap 'rm -rf "$TMP_DIR"' EXIT

if command -v valgrind > /dev/null
then
    RUNNER="valgrind -q --leak-check=full --errors-for-leak-kinds=all --error-exitcode=$LEAK_EXIT_CODE"
else
    echo "valgrind is not installed, memory leaks are not checked"
    RUNNER=""
fi

pass()
{
    echo "PASS $1"
    passed=$((passed + 1))
}

fail()
{
    echo "FAIL $1"
    failed=$((failed + 1))
}

# Runs huffman with the given arguments, saves stdout and stderr and returns its exit code.
run()
{
    $RUNNER $HUFFMAN "$@" > "$TMP_DIR/stdout" 2> "$TMP_DIR/stderr"
}

expect_error()
{
    local name=$1
    shift

    run "$@"
    local code=$?

    if [ $code -eq $LEAK_EXIT_CODE ]
    then
        fail "$name (memory error)"
    elif [ $code -eq 0 ]
    then
        fail "$name (exit code 0)"
    elif [ -s "$TMP_DIR/stdout" ] || [ ! -s "$TMP_DIR/stderr" ]
    then
        fail "$name (error must be printed only to stderr)"
    else
        pass "$name"
    fi
}

check_help()
{
    run "$@"

    if [ $? -eq 0 ] && grep -q "Usage" "$TMP_DIR/stdout" && [ ! -s "$TMP_DIR/stderr" ]
    then
        pass "help $*"
    else
        fail "help $*"
    fi
}

# Checks that the output has exactly three lines with numbers.
is_three_numbers()
{
    [ "$(wc -l < "$1")" -eq 3 ] && ! grep -qv '^[0-9][0-9]*$' "$1"
}

check_roundtrip()
{
    local file=$1
    local name
    name=$(basename "$file")
    local archive="$TMP_DIR/$name.huf"
    local restored="$TMP_DIR/$name.out"

    run -a -f "$file" -o "$archive"
    if [ $? -ne 0 ] || ! is_three_numbers "$TMP_DIR/stdout" || [ -s "$TMP_DIR/stderr" ]
    then
        fail "archive $name"
        return
    fi

    mapfile -t archive_stats < "$TMP_DIR/stdout"

    run -d -f "$archive" -o "$restored"
    if [ $? -ne 0 ] || ! is_three_numbers "$TMP_DIR/stdout" || [ -s "$TMP_DIR/stderr" ]
    then
        fail "dearchive $name"
        return
    fi

    mapfile -t dearchive_stats < "$TMP_DIR/stdout"

    if ! cmp -s "$file" "$restored"
    then
        fail "roundtrip $name (files differ)"
        return
    fi

    local source_size
    local archive_size
    source_size=$(stat -c %s "$file")
    archive_size=$(stat -c %s "$archive")

    if [ "${archive_stats[0]}" -ne "$source_size" ] \
        || [ $((archive_stats[1] + archive_stats[2])) -ne "$archive_size" ] \
        || [ "${dearchive_stats[0]}" -ne "${archive_stats[1]}" ] \
        || [ "${dearchive_stats[1]}" -ne "${archive_stats[0]}" ] \
        || [ "${dearchive_stats[2]}" -ne "${archive_stats[2]}" ]
    then
        fail "roundtrip $name (wrong statistics)"
        return
    fi

    pass "roundtrip $name"
}

check_long_options()
{
    local file="$ASSETS_DIR/text.txt"

    if run --archive --file "$file" --output "$TMP_DIR/long.huf" \
        && run --dearchive --file "$TMP_DIR/long.huf" --output "$TMP_DIR/long.out" \
        && cmp -s "$file" "$TMP_DIR/long.out"
    then
        pass "long options"
    else
        fail "long options"
    fi
}

check_compression()
{
    local file="$ASSETS_DIR/readme.md"

    run -a -f "$file" -o "$TMP_DIR/compression.huf"
    mapfile -t stats < "$TMP_DIR/stdout"

    if [ "${stats[1]}" -lt "${stats[0]}" ]
    then
        pass "text is compressed"
    else
        fail "text is compressed"
    fi
}

check_removed_output()
{
    local output="$TMP_DIR/broken.out"

    run -d -f "$BROKEN_DIR/cut_data.huf" -o "$output"

    if [ -e "$output" ]
    then
        fail "output is removed after error"
    else
        pass "output is removed after error"
    fi
}

echo "--- help"
check_help -h
check_help --help
check_help -a -h

echo "--- roundtrip"
for file in "$ASSETS_DIR"/*
do
    if [ -f "$file" ]
    then
        check_roundtrip "$file"
    fi
done

check_long_options
check_compression

echo "--- wrong arguments"
TEXT="$ASSETS_DIR/text.txt"
OUT="$TMP_DIR/out"
expect_error "no arguments"
expect_error "no mode" -f "$TEXT" -o "$OUT"
expect_error "both modes" -a -d -f "$TEXT" -o "$OUT"
expect_error "no input" -a -o "$OUT"
expect_error "no output" -a -f "$TEXT"
expect_error "input without value" -a -o "$OUT" -f
expect_error "output without value" -a -f "$TEXT" -o
expect_error "empty file name" -a -f "" -o "$OUT"
expect_error "repeated mode" -a -a -f "$TEXT" -o "$OUT"
expect_error "repeated input" -a -f "$TEXT" -f "$TEXT" -o "$OUT"
expect_error "unknown option" -a -x -f "$TEXT" -o "$OUT"
expect_error "extra argument" -a -f "$TEXT" -o "$OUT" extra
expect_error "same input and output" -a -f "$TEXT" -o "$TEXT"

echo "--- wrong files"
expect_error "missing input" -a -f "$TMP_DIR/missing" -o "$OUT"
expect_error "missing archive" -d -f "$TMP_DIR/missing" -o "$OUT"
expect_error "input is a directory" -a -f "$ASSETS_DIR" -o "$OUT"
expect_error "output directory does not exist" -a -f "$TEXT" -o "$TMP_DIR/missing/out"
expect_error "dearchive not an archive" -d -f "$TEXT" -o "$OUT"

for file in "$BROKEN_DIR"/*
do
    expect_error "broken archive $(basename "$file")" -d -f "$file" -o "$OUT"
done

check_removed_output

echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
