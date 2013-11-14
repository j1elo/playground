#! /bin/sh

# Script following
# https://code.google.com/p/google-breakpad/wiki/LinuxStarterGuide
# exactly to see if Breakpad works

set -e
set -x

TARGET=test

mkdir google-breakpad-test
cd google-breakpad-test

rm -rf google-breakpad
svn checkout http://google-breakpad.googlecode.com/svn/trunk/ google-breakpad
cd google-breakpad
./configure
make
cd ..

cat > $TARGET.cc <<_EOF_
#include <stdio.h>
#include "client/linux/handler/exception_handler.h"

static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
                         void* context,
                         bool succeeded)
{
    printf("Dump path: %s\n", descriptor.path());
    return succeeded;
}

void crash_xyzzy()
{
    volatile int* a = (int*)(NULL);
    *a = 1;
}

int main(int argc, char* argv[])
{
    google_breakpad::MinidumpDescriptor descriptor("/tmp");
    google_breakpad::ExceptionHandler eh(descriptor,
                                         NULL,
                                         dumpCallback,
                                         NULL,
                                         true,
                                         -1);
    crash_xyzzy();
    return 0;
}
_EOF_

g++ -I google-breakpad/src -O0 -g $TARGET.cc -o $TARGET google-breakpad/src/client/linux/libbreakpad_client.a -pthread

google-breakpad/src/tools/linux/dump_syms/dump_syms $TARGET > $TARGET.sym
SUM=`head -n1 $TARGET.sym | awk '{print $4}'`
mkdir -p symbols/$TARGET/$SUM
mv $TARGET.sym symbols/$TARGET/$SUM

rm -f /tmp/*.dmp
if ./$TARGET
then
    echo "$TARGET did not crash.  #fail"
    exit 1
fi
if ! test -f /tmp/*.dmp
then
    echo "No dump produced.  #fail"
    exit 1
fi

google-breakpad/src/processor/minidump_stackwalk /tmp/*.dmp ./symbols > stackwalk.log
if ! grep xyzzy stackwalk.log
then
    echo "xyzzy not found.  #fail"
    exit 1
else
    echo "#PASS"
fi
