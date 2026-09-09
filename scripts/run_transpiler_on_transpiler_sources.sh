#!/bin/bash

# CURR_FILE_PATH=$(realpath "${0}")
# CURR_FOLDER_PATH=$(dirname "${CURR_FILE_PATH}")
# PROJECT_FOLDER_PATH=$(dirname "${CURR_FOLDER_PATH}")

# echo "PROJECT_FOLDER_PATH -> ${PROJECT_FOLDER_PATH}"
# exit 0

# realpath $1

echo ""
echo "*--------------------------------------*"
echo "| RUN TRANSPILER ON TRANSPILER SOURCES |"
echo "*--------------------------------------*"
echo ""

# rm -rf "${PROJECT_FOLDER_PATH}/assets/samples/small-sample/output"

# "${PROJECT_FOLDER_PATH}/bin/exec" \
#   "--base-dir=${PROJECT_FOLDER_PATH}/assets/samples/small-sample/src/" \
#   "--input-file=${PROJECT_FOLDER_PATH}/assets/samples/small-sample/src/main.c" \
#   "--output-dir=${PROJECT_FOLDER_PATH}/assets/samples/small-sample/output" \
#   "--add-include-path=${PROJECT_FOLDER_PATH}/assets/samples/small-sample/src/" \
#   || exit 1


# ./bin/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.c" \
#   "--output-dir=${PWD}/output" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
#   || exit 1

./output/bin/exec \
  "--base-dir=${PWD}/src/" \
  "--input-file=${PWD}/src/main.c" \
  "--output-dir=${PWD}/output" \
  "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
  "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
  "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
  "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
  || exit 1

# ./last-known-good/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.c" \
#   "--output-dir=${PWD}/output" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
#   || exit 1
