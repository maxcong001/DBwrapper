#!/bin/bash
#work_path=$(dirname $0)
work_path=$(dirname $(readlink -f $0))
echo 'base working dir is :' $work_path
git clone --recursive https://github.com/Cylix/cpp_redis.git&&cd cpp_redis&&mkdir -p build&&cd build&&make&&make install

mkdir -p build&&cd build&&cmake ..&&make&&make install
