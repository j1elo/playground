#include "CrashHandler.h"
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QString>
#include <QtDebug>

#include <cstdio> // fprintf

#if defined(Q_OS_LINUX)
#include <client/linux/handler/exception_handler.h>
#elif defined(Q_OS_WIN32)
#include <client/windows/handler/exception_handler.h>
#elif defined(Q_OS_MAC)
#include <client/mac/handler/exception_handler.h>
#endif


google_breakpad::ExceptionHandler* CrashHandler::pHandler = NULL;
bool CrashHandler::bReportCrashesToSystem = false;


// ==================================================================
// DumpCallback
// ==================================================================

#if defined(Q_OS_LINUX)
bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
#elif defined(Q_OS_WIN32)
bool DumpCallback(const wchar_t* _dump_dir, const wchar_t* _minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool success)
#elif defined(Q_OS_MAC)
bool DumpCallback(const char* _dump_dir, const char* _minidump_id, void* context, bool success)
#endif
{
    /*
    NO STACK USE, NO HEAP USE HERE !!!
    Creating QString's, using qDebug, etc. - everything is crash-unfriendly.

    If you must do some work in the callback, the Breakpad source contains
    some simple reimplementations of libc functions [1], to avoid calling directly
    into libc, as well as a header file for making Linux system calls [2] (in src/third_party/lss)
    to avoid calling into other shared libraries.
    [1]: https://code.google.com/p/google-breakpad/source/browse/trunk/src/common/linux/linux_libc_support.h
    [2]: https://code.google.com/p/linux-syscall-support/

    Here you would want to handle the minidump in some way, likely by sending it
    to a server for analysis. The Breakpad source tree contains some HTTP upload source [3]
    that you might find useful, as well as a minidump upload tool [4].
    [3]: https://code.google.com/p/google-breakpad/source/browse/#svn/trunk/src/common/linux
    [4]: https://code.google.com/p/google-breakpad/source/browse/#svn/trunk/src/tools/linux/symupload
    */

    Q_UNUSED(context);

    //qDebug("[DumpCallback] CRASH CAUGHT");
    fprintf(stderr, "[DumpCallback] CRASH CAUGHT\n");

#if defined(Q_OS_LINUX)
    //qDebug("[DumpCallback] Dump path: %s", descriptor.path());
    fprintf(stderr, "[DumpCallback] Dump path: %s\n", descriptor.path());
#elif defined(Q_OS_WIN32)
    Q_UNUSED(_dump_dir);
    Q_UNUSED(_minidump_id);
    Q_UNUSED(assertion);
    Q_UNUSED(exinfo);
#endif

    return CrashHandler::bReportCrashesToSystem ? succeeded : true;
}


// ==================================================================
// CrashHandler
// ==================================================================

CrashHandler* CrashHandler::instance()
{
    static CrashHandler globalHandler;
    return &globalHandler;
}

void CrashHandler::init(const QString& reportPath)
{
    if (pHandler != NULL)
        return;

#if defined(Q_OS_LINUX)
    std::string reportPathStr = reportPath.toStdString();
    google_breakpad::MinidumpDescriptor descriptor(reportPathStr);
    pHandler = new google_breakpad::ExceptionHandler(
                /*MinidumpDescriptor*/ descriptor,
                /*FilterCallback*/ 0,
                /*MinidumpCallback*/ DumpCallback,
                /*callback_context*/ 0,
                /*install_handler*/ true,
                /*server_fd*/ -1
                );
#elif defined(Q_OS_WIN32)
    std::wstring reportPathStr = (const wchar_t*)reportPath.utf16();
    pHandler = new google_breakpad::ExceptionHandler(
                reportPathStr,
                /*FilterCallback*/ 0,
                DumpCallback,
                /*context*/ 0,
                true
                );
#elif defined(Q_OS_MAC)
    std::string reportPathStr = reportPath.toStdString();
    pHandler = new google_breakpad::ExceptionHandler(
                reportPathStr,
                /*FilterCallback*/ 0,
                DumpCallback,
                /*context*/ 0,
                true,
                NULL
                );
#endif

    qDebug("[CrashHandler::init] succesfully initialized");
}

bool CrashHandler::writeMinidump()
{
    bool ok = pHandler->WriteMinidump();
    if (ok) {
        qDebug("[CrashHandler::writeMinidump] writeMinidump() SUCCEED.");
    }
    else {
        qWarning("[CrashHandler::writeMinidump] writeMinidump() FAIL.");
    }
    return ok;
}

CrashHandler::CrashHandler()
{}

CrashHandler::~CrashHandler()
{
    delete pHandler;
}
