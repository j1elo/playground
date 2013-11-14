#! /bin/bash

# Generate Google Breakpad distribution folder from original source folder

SRC=$PWD/google-breakpad.svn
BLD=$PWD/google-breakpad.build
DST=/opt/google-breakpad

svn checkout http://google-breakpad.googlecode.com/svn/trunk/ $SRC

mkdir -p $BLD
cd $BLD
$SRC/configure --prefix=$DST
make
make install-strip
cd -

# 'make install' doesn't install headers, so we need to copy them
COPY_PATHS='
client/linux/handler/exception_handler.h
client/linux/crash_generation/crash_generation_client.h
client/linux/handler/minidump_descriptor.h
common/using_std_string.h
client/linux/minidump_writer/minidump_writer.h
client/linux/minidump_writer/linux_dumper.h
common/memory.h
third_party/lss/linux_syscall_support.h
google_breakpad/common/minidump_format.h
google_breakpad/common/breakpad_types.h
google_breakpad/common/minidump_cpu_*.h
google_breakpad/common/minidump_exception_*.h
common/scoped_ptr.h
'

mkdir -p $DST/include/

cd $SRC/src/
for f in $COPY_PATHS ; do
    cp --parents --recursive "`eval echo ${f//>}`" $DST/include/
done
cd -

# 'make install' doesn't strip unneeded symbols from libraries
strip --strip-unneeded $DST/lib/*
