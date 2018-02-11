#!/bin/bash
#work_path=$(dirname $0)
work_path=$(dirname $(readlink -f $0))
echo 'base working dir is :' $work_path
cd $work_path
mkdir -p build&&cd build&&cmake ..&&make&&sudo make install
cd $work_path
./build/bin/DBwrapper_example

