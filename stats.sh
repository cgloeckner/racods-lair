#!/bin/bash

cd include
LOC=$((`( find ./ -name '*.hpp' -print0 | xargs -0 cat ) | wc -l` + `( find ./ -name '*.inl' -print0 | xargs -0 cat ) | wc -l`))
echo "include/    $LOC LoC"

cd ../src
LOC=$((`( find ./ -name '*.cpp' -print0 | xargs -0 cat ) | wc -l`))
echo "src/        $LOC LoC"

cd ../test_suite
LOC=$((`( find ./ -name '*.cpp' -print0 | xargs -0 cat ) | wc -l`))
echo "test_suite/ $LOC LoC"

cd ..
