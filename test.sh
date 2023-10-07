#!/bin/sh -e
mkdir -p test/{a,b,c}
echo x > test/a/x
echo y > test/a/y
echo y > test/b/y
export SOURCE_DATE_EPOCH=1
./zip -X -r test.zip test
md5sum test.zip
echo "89057b9c9501ce122973d24b68a0522a  test.zip" | md5sum -c

