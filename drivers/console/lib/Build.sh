#!/bin/ksh


make $* || exit 2
mv *.o ../objs
