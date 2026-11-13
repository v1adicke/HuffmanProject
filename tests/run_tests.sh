#!/bin/bash

HUFFMAN=./huffman
ASSETS_DIR=tests/assets
TMP_DIR=$(mktemp -d)
passed=0
failed=0

trap 'rm -rf "$TMP_DIR"' EXIT

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

check_roundtrip()
{
    local file=$1
    local name
    name=$(basename "$file")

    if $HUFFMAN -a -f "$file" -o "$TMP_DIR/$name.huf" > /dev/null \
        && $HUFFMAN -d -f "$TMP_DIR/$name.huf" -o "$TMP_DIR/$name.out" > /dev/null \
        && cmp -s "$file" "$TMP_DIR/$name.out"
    then
        pass "roundtrip $name"
    else
        fail "roundtrip $name"
    fi
}

for file in "$ASSETS_DIR"/*
do
    check_roundtrip "$file"
done

echo "passed: $passed, failed: $failed"
[ "$failed" -eq 0 ]
