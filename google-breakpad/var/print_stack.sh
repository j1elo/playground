#! /bin/sh

DUMPFILE=$1
if [ -z "$DUMPFILE" ]
then
    echo "Usage: print_stack.sh <minidump file>"
    exit 1
fi

./minidump_stackwalk $DUMPFILE ./symbolstore 2> /dev/null
