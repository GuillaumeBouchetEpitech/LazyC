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







# if [ ! -f ./bin/exec ];
# then
#   echo "no ./bin/exec binary!!!"
#   exit 1
# fi

echo ""
echo "####################"
echo "# BUILD TRANSPILER #"
echo "####################"
echo ""

rm -rf "${PWD}/output"

# cleanup ".generated" folder(s)
find ./src/ -name .generated -type d -type d
find ./src/ -name .generated -type d -type d -exec bash -c ' rm -rf "{}" ' \;

# gdb -ex run --args ./bin/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.c" \
#   "--output-dir=${PWD}/output" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
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

# ./_tmp/c-transpiler/bin/exec \
./_tmp/v0.0.2-dirty-100x-faster-prototype/output/bin/exec \
  "--base-dir=${PWD}/src/" \
  "--input-file=${PWD}/src/main.c" \
  "--output-dir=${PWD}/output" \
  "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
  "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
  "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
  "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
  || exit 1

echo ""
echo "#######"
echo "# RUN #"
echo "#######"
echo ""

rm -rf "${PWD}/output-first"

# cleanup ".generated" folder(s)
find ./src/ -name .generated -type d -type d
find ./src/ -name .generated -type d -type d -exec bash -c ' rm -rf "{}" ' \;

# # gdb -ex run --args ./output/bin/exec \
# #   "--base-dir=${PWD}/src/" \
# #   "--input-file=${PWD}/src/main.c" \
# #   "--output-dir=${PWD}/output-first" \
# #   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
# #   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
# #   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
# #   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
# #   || exit 1

./output/bin/exec \
  "--base-dir=${PWD}/src/" \
  "--input-file=${PWD}/src/main.c" \
  "--output-dir=${PWD}/output-first" \
  "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
  "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
  "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
  "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
  || exit 1

echo ""
echo "####################"
echo "# BUILD UNIT-TESTS #"
echo "####################"
echo ""

rm -rf "${PWD}/output-second"

# cleanup ".generated" folder(s)
find ./src/ -name .generated -type d -type d
find ./src/ -name .generated -type d -type d -exec bash -c ' rm -rf "{}" ' \;

# ./output-first/bin/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.tests.c" \
#   "--output-dir=${PWD}/output-second-test" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
#   --handle-tests=1 \
#   || exit 1
./output-first/bin/exec \
  "--base-dir=${PWD}/src/" \
  "--input-file=${PWD}/src/main.tests.c" \
  "--output-dir=${PWD}/output-second" \
  "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
  "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
  "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
  "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
  --handle-tests=1 \
  || exit 1

echo ""
echo "##################"
echo "# RUN UNIT-TESTS #"
echo "##################"
echo ""

# copy the tests-assets folders
find ./src -name tests-assets -type d
find ./src -name tests-assets -type d -exec bash -c ' cp -rf "{}" "./output-second/{}" ' \;

# cleanup "output" folder(s)
find ./output-second/src/ -name output -type d
find ./output-second/src/ -name output -type d -exec bash -c ' rm -rf "{}" ' \;

./output-second/bin/exec


echo ""
echo "################################"
echo "# COPYING UNIT-TESTS ARTIFACTS #"
echo "################################"
echo ""

pushd ./output-second || exit 1

  # find ./src -name "*.debug" -type f -or -name "*.actual-matches" -type f
  find ./src -name "*.debug" -type f -or -name "*.actual-matches" -type f -exec bash -c ' cp -rf "{}" "../{}" ' \;

popd || exit 1

echo ""
echo "########"
echo "# DONE #"
echo "########"
echo ""

exit 0

# find ./src -name tests-assets -type f # -exec bash -c ' cp -rf "{}" "./output-second/{}" ' \;



# if [ ! -f ./last-known-good/exec ];
# then
#   echo "no ./last-known-good/exec binary!!!"
#   exit 1
# fi

# gdb -ex run --args ./last-known-good/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.c" \
#   "--output-dir=${PWD}/output" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
#   || exit 1



# ./output/bin/exec \
#   "--base-dir=${PWD}/src/" \
#   "--input-file=${PWD}/src/main.c" \
#   "--output-dir=${PWD}/output" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter-lazy-c/bindings/c" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
#   "--add-include-path=${PWD}/third_parties/tree-sitter/lib/include" \
#   "--add-library-path=${PWD}/third_parties/tree-sitter/libtree-sitter.a" \
#   || exit 1



