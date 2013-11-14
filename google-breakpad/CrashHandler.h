#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H
#pragma once

#include <QtCore/QString>

namespace google_breakpad
{
class ExceptionHandler;
}

class CrashHandler
{
public:
    static CrashHandler* instance();
    void init(const QString& reportPath);
    bool writeMinidump();

    /* ???.
     * Default: false. */
    static bool bReportCrashesToSystem;

private:
    CrashHandler();
    ~CrashHandler();
    Q_DISABLE_COPY(CrashHandler)

    static google_breakpad::ExceptionHandler* pHandler;
};

#endif // CRASHHANDLER_H
