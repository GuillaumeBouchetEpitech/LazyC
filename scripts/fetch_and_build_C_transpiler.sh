#!/bin/bash

#
#
#

#
#
#

#
#
#

if [ -f ./_tmp/c-transpiler/bin/exec ];
then
  echo "compiled C transpiler found -> doing nothing"

else

  echo "compiled C transpiler not found"

  if [ ! -d ./_tmp/c-transpiler ];
  then

    echo "cloning compiled C transpiler"

    mkdir -p ./_tmp
    git clone \
      --depth 1 --branch v0.0.1-the-c-prototype \
      https://github.com/GuillaumeBouchetEpitech/LazyC.git ./_tmp/c-transpiler \
      || exit 1

  fi

  echo "building compiled C transpiler"

  pushd ./_tmp/c-transpiler || exit 1

    bash ./scripts/ensure_third_parties.sh || exit 1
    bash ./scripts/build_transpiler.sh || exit 1

  popd || exit 1

  echo "compiled C transpiler built"

fi

#
#
#

#
#
#

#
#
#

if [ -f ./_tmp/v0.0.2-dirty-100x-faster-prototype/bin/exec ];
then
  echo "compiled v0.0.2-dirty-100x-faster-prototype transpiler found -> doing nothing"
  exit 0;
fi

echo "compiled v0.0.2-dirty-100x-faster-prototype transpiler not found"

if [ ! -d ./_tmp/v0.0.2-dirty-100x-faster-prototype ];
then

  echo "cloning v0.0.2-dirty-100x-faster-prototype transpiler"

  mkdir -p ./_tmp
  git clone \
    --depth 1 --branch v0.0.2-dirty-100x-faster-prototype \
    https://github.com/GuillaumeBouchetEpitech/LazyC.git ./_tmp/v0.0.2-dirty-100x-faster-prototype \
    || exit 1

fi

echo "building v0.0.2-dirty-100x-faster-prototype transpiler"

pushd ./_tmp/v0.0.2-dirty-100x-faster-prototype || exit 1

  # bash ./scripts/ensure_third_parties.sh || exit 1
  # bash ./scripts/build_transpiler.sh || exit 1

  ../c-transpiler/bin/exec \
    "--base-dir=${PWD}/src/" \
    "--input-file=${PWD}/src/main.c" \
    "--output-dir=${PWD}/output" \
    "--add-include-path=${PWD}/../c-transpiler/third_parties/tree-sitter-lazy-c/bindings/c" \
    "--add-library-path=${PWD}/../c-transpiler/third_parties/tree-sitter-lazy-c/build-native/tree-sitter-lazy_c.a" \
    "--add-include-path=${PWD}/../c-transpiler/third_parties/tree-sitter/lib/include" \
    "--add-library-path=${PWD}/../c-transpiler/third_parties/tree-sitter/libtree-sitter.a" \
    || exit 1

popd || exit 1

echo "v0.0.2-dirty-100x-faster-prototype transpiler built"

#
#
#

#
#
#

#
#
#


