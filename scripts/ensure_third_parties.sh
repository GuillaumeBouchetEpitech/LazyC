#!/bin/bash

ROOT_DIR="${PWD}"

echo ""
echo "*---------------*"
echo "| THIRD_PARTIES |"
echo "*---------------*"
echo ""

if [ ! -d ./third_parties ];
then
  mkdir -p ./third_parties || exit 1
fi

pushd ./third_parties || exit 1

  #
  #
  #

  if [ ! -d tree-sitter ];
  then
    git clone --depth 1 --branch v0.26.11 https://github.com/tree-sitter/tree-sitter.git ./tree-sitter
  fi

  if [ ! -f tree-sitter/libtree-sitter.a ];
  then
    pushd ./tree-sitter || exit 1
      make libtree-sitter.a
    popd || exit 1
  fi

  #
  #
  #

  if [ ! -d ./tree-sitter-lazy-c ];
  then
    mkdir -p ./tree-sitter-lazy-c
  fi

  pushd ./tree-sitter-lazy-c || exit 1

    if [ ! -f ./bindings/c/tree_sitter/tree-sitter-lazyc.h ];
    then
      cp "$ROOT_DIR/tree-sitter-configs/grammar.js" ./grammar.js || exit 1
      cp "$ROOT_DIR/tree-sitter-configs/package.json" ./package.json || exit 1
      cp "$ROOT_DIR/tree-sitter-configs/tree-sitter.json" ./tree-sitter.json || exit 1
      tree-sitter init
      tree -hD ./bindings
    fi

    if [ ! -d ./src ];
    then
      mkdir -p ./src || exit 1
      tree-sitter generate --output ./src || exit 1
      tree -hD ./src
    fi

    if [ ! -d ./build-native ];
    then
      mkdir -p ./build-native || exit 1
      tree-sitter build --output ./build-native/tree-sitter-lazy_c.a || exit 1
      tree -hD ./build-native
    fi

    # if [ ! -d ./build-wasm ];
    # then
    #   mkdir -p ./build-wasm || exit 1
    #   tree-sitter build --wasm --output ./build-wasm/tree-sitter-lazy_c.wasm || exit 1
    #   tree -hD ./build-wasm
    # fi

  popd || exit 1

  #
  #
  #

popd || exit 1
