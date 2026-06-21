#!/bin/bash
# Generate compile_commands.json for GameAK.
# Usage: scripts/gen_compile_commands.sh <output> <project-dir> <compiler> <cxxflags> <cppflags>

set -euo pipefail

OUTPUT="${1:?missing OUTPUT}"
PROJECT_DIR="${2:?missing PROJECT_DIR}"
COMPILER="${3:?missing COMPILER}"
CXXFLAGS="${4:?missing CXXFLAGS}"
CPPFLAGS="${5:?missing CPPFLAGS}"

echo "[" > "$OUTPUT"
first=1

# Find all source files
find "$PROJECT_DIR/Src" -name '*.cpp' -o -name '*.c' | sort | while read -r src; do
    rel="${src#$PROJECT_DIR/}"
    obj="$PROJECT_DIR/build/objs/${rel%.cpp}.o"
    if [ $first -eq 1 ]; then
        first=0
    else
        echo "," >> "$OUTPUT"
    fi
    cat >> "$OUTPUT" <<ENDJSON
  {
    "directory": "$PROJECT_DIR",
    "command": "$COMPILER $CXXFLAGS $CPPFLAGS -c -o $obj $src",
    "file": "$rel"
  }
ENDJSON
done

# Add test source
if [ -f "$PROJECT_DIR/Tests/test_runtime.cpp" ]; then
    echo "," >> "$OUTPUT"
    obj="$PROJECT_DIR/build/objs/Tests/test_runtime.o"
    cat >> "$OUTPUT" <<ENDJSON
  {
    "directory": "$PROJECT_DIR",
    "command": "$COMPILER $CXXFLAGS $CPPFLAGS -c -o $obj $PROJECT_DIR/Tests/test_runtime.cpp",
    "file": "Tests/test_runtime.cpp"
  }
ENDJSON
fi

echo "]" >> "$OUTPUT"
