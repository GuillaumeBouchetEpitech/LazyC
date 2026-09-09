#!/bin/bash

echo ""
echo "*----------------*"
echo "| RUN UNIT TESTS |"
echo "*----------------*"
echo ""

# cleanup "output" folder(s)
find ./src/ -name output -type d
find ./src/ -name output -type d -exec bash -c ' rm -rf "{}" ' \;

# cleanup ".generated" folder(s)
find ./src/ -name .generated -type d -type d
find ./src/ -name .generated -type d -type d -exec bash -c ' rm -rf "{}" ' \;

./bin/unit_tests || exit 1
# valgrind --leak-check=full --error-exitcode=1 ./bin/unit_tests || exit 1

