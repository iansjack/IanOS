#!/bin/sh
mkdir dist
mkdir dist/IanOS
mkdir dist/IanOS/include
mkdir dist/IanOS/library
mkdir dist/IanOS/tasks

cp *.[cs] dist/IanOS
cp include/* dist/IanOS/include
cp library/*.[cs] dist/IanOS/library
cp tasks/*.[cs] dist/IanOS/tasks

cp Makefile dist/IanOS
cp library/Makefile dist/IanOS/library
cp tasks/Makefile dist/IanOS/tasks

cp *.ld dist/IanOS
cp tasks/*.ld dist/IanOS/tasks

cp createdisk.sh dist/IanOS
cp Flags.mak dist/IanOS
cp 32sect dist/IanOS
cp floppy dist/IanOS
cp IanOS dist/IanOS
cp IanOSd dist/IanOS
cp gmsl dist/IanOS
cp __gmsl dist/IanOS
cp .gdbinit dist/IanOS

cd dist
tar cf IanOS.tar IanOS
gzip IanOS.tar

