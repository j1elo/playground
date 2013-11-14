#! /bin/sh

DUMPFILE=$1
if [ -z "$DUMPFILE" ]
then
    echo "Usage: breakpad-printstack.sh <minidump file>"
    exit 0
fi

var/minidump_stackwalk $DUMPFILE ./symbolstore 2> /dev/null
