#!/bin/bash
set -e

echo "Building..."
cmake -B build -S .
cmake --build build --parallel

echo "Testing units..."
./build/MarkdownTests

echo ""
echo "Workflow test with file..."
./build/MarkdownToHTML --in tests/tests.md --out tests/actual.html
if diff tests/expected.html tests/actual.html > /dev/null 2>&1; then
    echo "  PASS: output matches expected.html"
else
    echo "  FAIL: output differs from expected.html"
    diff tests/expected.html tests/actual.html || true
    exit 1
fi

echo ""
echo "Tests passed!!!"
