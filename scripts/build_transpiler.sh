#!/bin/bash

# CURR_FILE_PATH=$(realpath "${0}")
# CURR_FOLDER_PATH=$(dirname "${CURR_FILE_PATH}")
# PROJECT_FOLDER_PATH=$(dirname "${CURR_FOLDER_PATH}")

echo ""
echo "*------------------*"
echo "| BUILD TRANSPILER |"
echo "*------------------*"
echo ""

mkdir -p "./cmake-build.native.exec"
cmake -B "./cmake-build.native.exec" \
  -DCMAKE_BUILD_TARGET=exec \
  -DCMAKE_BUILD_TYPE=debug \
  -DCMAKE_C_COMPILER_LAUNCHER=/usr/bin/sccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=/usr/bin/sccache || exit 1
mold -run cmake --build "./cmake-build.native.exec" --parallel 5 || exit 1
