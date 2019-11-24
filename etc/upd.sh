#!/bin/bash
cd /Users/sal/GIT/meataxe64
make clean
rm -rf mtx64v2b.old
mv mtx64v2b mtx64v2b.old
cp -r /Users/sal/Dropbox/mtx64v2b mtx64v2b
cd src
rm -rf mtx64.old
mv mtx64 mtx64.old
mv ../mtx64v2b/src mtx64
cd ..
./configure --with-gaproot=../gap/build/default
make
