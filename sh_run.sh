#!/bin/bash

clear && clear

rm -rf "./bin" || true

bash ./scripts/ensure_third_parties.sh || exit 1

bash ./scripts/build_transpiler.sh || exit 1

bash ./scripts/build_unit_tests.sh || exit 1

bash ./scripts/run_unit_tests.sh || exit 1

# bash ./scripts/run_transpiler_on_transpiler_sources.sh || exit 1

echo ""
echo "*------*"
echo "| DONE |"
echo "*------*"
echo ""


