#include "CrashHandler.h"
#include <iostream>

int buggyFunc()
{
//    delete reinterpret_cast<QString*>(0xFEE1DEAD);

    volatile int* a = (int*)(NULL);
    *a = 1;

    return 0;
}

int main(int argc, char *argv[])
{
    std::cout << "App start" << std::endl;

#if defined(Q_OS_LINUX)
    CrashHandler::instance()->init("/tmp");
#elif defined(Q_OS_WIN32)
    CrashHandler::instance()->init(Init("C:\\Temp");
#elif defined(Q_OS_MAC)
    CrashHandler::instance()->init("/Users/User/dump");
#endif

    buggyFunc();
    return 0;
}
