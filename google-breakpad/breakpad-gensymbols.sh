#! /bin/sh

# Treat unset variables as errors
set -o nounset

# Exit immediately if any command returns a non-zero status
set -o errexit

# Extract debugging symbols from specified files
FILE=$1
if [ -z "$FILE" ]
then
    echo "Usage: breakpad-gensymbols.sh <binary file>"
    exit 0
fi

mkdir -p symbolstore
var/symbolstore.py var/dump_syms ./symbolstore $FILE

# Strip files from all unneeded symbols
strip --strip-unneeded $FILE
