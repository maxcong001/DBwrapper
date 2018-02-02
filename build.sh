#!/bin/bash
#work_path=$(dirname $0)
work_path=$(dirname $(readlink -f $0))
echo 'base working dir is :' $work_path
cd $work_path
git clone https://github.com/Cylix/tacopie.git&&cd tacopie&&mkdir -p build&&cd build&&cmake ..&&make&&sudo make install
cd $work_path
git clone --recursive https://github.com/Cylix/cpp_redis.git&&cd cpp_redis&&mkdir -p build&&cd build&&cmake ..&&make&&sudo make install
cd $work_path

