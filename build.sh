#!/bin/bash
set -e

# Config
CXX=clang++
CXXFLAGS="-Wall -Wextra -O0 -g -fno-exceptions -fno-rtti -std=c++23"
SRC_DIR=./
BUILD_DIR=build
OUT=app
OUT_PATH="$BUILD_DIR/$OUT"

mkdir -p "$BUILD_DIR"

# Track object files
OBJECTS=()

# Compile changed .cpp files
recompiled=0
for src in $(find "$SRC_DIR" -name '*.cpp'); do
    obj="$BUILD_DIR/$(basename "${src%.cpp}.o")"
    OBJECTS+=("$obj")

    # Recompile if .o doesn't exist or .cpp is newer
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "Compiling: $src"
        $CXX $CXXFLAGS -c "$src" -o "$obj"
        recompiled=1
    fi
done

# Relink if any object was recompiled or binary doesn't exist
if [ "$recompiled" -eq 1 ] || [ ! -f "$OUT_PATH" ]; then
    echo "Linking: $OUT"
    $CXX "${OBJECTS[@]}" -o "$OUT_PATH"
else
    echo "Nothing changed, skipping link."
fi

echo "Build done: $OUT_PATH"
echo "Running program..."
echo ""
"$OUT_PATH"