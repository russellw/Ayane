#!/bin/bash
set -e
export TPTP=/mnt/c/TPTP
# if make gives an error, check that
# this script file is saved with UNIX line endings
make prof
# http://www.thegeekstuff.com/2012/08/gprof-tutorial/
./ayane $*
gprof ayane gmon.out | more
