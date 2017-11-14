#!/bin/bash
# This script requires FCgiIO.cpp  FCgiIO.h and get_post.h be present in the same 
# directory as compile.sh
# If you cannot run this try chmod +x compile.sh 
CC=g++
COPTS="-std=c++11   -DBIG_JOINS=1 -fno-strict-aliasing -g -DNDEBUG -L/usr/lib/x86_64-linux-gnu  -lpthread -lz -lm -ldl -lhiredis  -lfcgi  -lcurl"
# Change the required .cpp files as necesary
CPP="main.cpp bs_tcp_functions.cpp"
# The name of the o/p executable
OP="main"


# Print shit out

echo -e -n '\E[37;32m'"\033[1mCompiling ...\033[0m"

# The actual execution
$CC -o $OP $CPP $COPTS

# This is bad code, need to implement an if block here.
echo -n "  $OP created  " 
echo -e '\E[37;33m'"\033[1mDone!!\033[0m"