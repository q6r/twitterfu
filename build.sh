#!/bin/sh

# Building libtwitcurl
echo "Building libtwitcurl"
if [ ! -f ./src/libtwitcurl.a ]
then
    cd libtwitcurl
    cmake .
    make -j4
    mv libtwitcurl.a ../src/
    if [ $? -ne 0 ]
    then
        echo "Failed while compiling libtwitcurl"
        exit 1
    fi
    cd ..
fi

# Building twitterfu
echo "Building twitterfu"
cd src/
SOURCES=$(ls *.cpp)
for S in $SOURCES
do
    g++ -Wall -c -Werror -O4 -std=gnu++11 $S
done

if [ $? -ne 0 ]
then
    echo "Failed while compiling twitterfu"
    exit 1
fi

g++ *.o libtwitcurl.a -lsqlite3 -lcurl -O4 -o twitterfu

if [ $? -ne 0 ]
then
    echo "Failed while compiling twitterfu"
    exit 1
fi


