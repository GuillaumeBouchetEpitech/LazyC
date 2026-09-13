#!/bin/bash


if [ -f ./_tmp/c-transpiler/bin/exec ];
then
  echo "compiled C transpiler found -> doing nothing"
  exit 0;
fi

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
