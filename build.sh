#!/bin/sh

# Building libtwitcurl
echo "Building libtwitcurl"
cd libtwitcurl
cmake .
make -j4
mv libtwitcurl.a ../src/
if [ $? -ne 0 ]
then
    echo "Failed while compiling libtwitcurl"
    exit 1
fi

# Building twitterfu
echo "Building twitterfu"
cd ../src
SOURCES=$(ls *.cpp)
for S in $SOURCES
do
    g++ -Wall -c -Werror $S
done

if [ $? -ne 0 ]
then
    echo "Failed while compiling twitterfu"
    exit 1
fi

g++ *.o libtwitcurl.a -lsqlite3 -lcurl -o twitterfu

if [ $? -ne 0 ]
then
    echo "Failed while compiling twitterfu"
    exit 1
fi


