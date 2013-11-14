google-breakpad crash reporting
===============================

https://code.google.com/p/google-breakpad/

Breakpad has three main components:
* The client is a library that you include in your application. It can write minidump files capturing the current threads' state and the identities of the currently loaded executable and shared libraries. You can configure the client to write a minidump when a crash occurs, or when explicitly requested.
* The symbol dumper is a program that reads the debugging information produced by the compiler and produces a symbol file, in Breakpad's own format.
* The processor is a program that reads a minidump file, finds the appropriate symbol files for the versions of the executables and shared libraries the minidump mentions, and produces a human-readable C/C++ stack trace.


Producing symbols for your application:
$ google-breakpad/src/tools/linux/dump_syms/dump_syms ./test > test.sym

In order to use these symbols with the minidump_stackwalk tool, you will need to place them in a specific directory structure. The first line of the symbol file contains the information you need to produce this directory structure, for example (your output will vary):
$ head -n1 test.sym
MODULE Linux x86_64 6EDC6ACDB282125843FD59DA9C81BD830 test
$ mkdir -p ./symbols/test/6EDC6ACDB282125843FD59DA9C81BD830
$ mv test.sym ./symbols/test/6EDC6ACDB282125843FD59DA9C81BD830

Processing the minidump to produce a stack trace
minidump_stackwalk can take a minidump plus its corresponding text-format symbols and produce a symbolized stacktrace.
$ google-breakpad/src/processor/minidump_stackwalk minidump.dmp ./symbols


Preparation:
/opt/google-breakpad/bin/dump_syms $TARGET > $TARGET.sym
SUM=`head -n1 $TARGET.sym | awk '{print $4}'`
mkdir -p symbols/$TARGET/$SUM
mv $TARGET.sym symbols/$TARGET/$SUM/
strip --strip-all $TARGET

Run:
rm -f /tmp/*.dmp
./google-breakpad-test
/opt/google-breakpad/bin/minidump_stackwalk $MINIDUMP ./symbols/ 2>/dev/null
