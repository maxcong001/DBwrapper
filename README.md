# DBwrapper
this project will be a wrapper of DB.

there will be the following part:
```
1. command parser(hold)
3. connection management(https://github.com/maxcong001/connection_manager)
4. timer service(doing)(https://github.com/maxcong001/timer)
6. service discovery part(hold)
```
## this project aimed to only depends on libevnet and C++11.

## will realize redis suport


NOTE:you need to run :
```
git submodule update --init --recursive
git submodule foreach --recursive git submodule init
git submodule foreach --recursive git submodule update 
```
while cloning this rep
