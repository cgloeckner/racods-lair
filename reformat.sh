#!/bin/bash

cd include
clang-format -i **/*.hpp
clang-format -i **/*.inl
cd ../src
clang-format -i **/*.cpp
cd ../test_suite
clang-format -i **/*.cpp
cd ..
