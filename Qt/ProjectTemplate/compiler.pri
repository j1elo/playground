*g++* {
    # Enable support for C++11 / C++14
    greaterThan(QT_MAJOR_VERSION, 4) {
        CONFIG *= c++11 c++14
    } else {
        QMAKE_CXXFLAGS *= -std=c++0x
    }

    # Enable all opt-in warnings, and make all warnings into errors
    QMAKE_CXXFLAGS *= -Wall -Wextra -Werror

    # Include debug symbols in Release builds
    QMAKE_CFLAGS_RELEASE   *= -g
    QMAKE_CXXFLAGS_RELEASE *= -g

    # Disable code assertions
    QMAKE_CFLAGS_RELEASE   *= -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
}

win32-msvc* {
    # Disable deprecation of *printf functions
    DEFINES *= _CRT_SECURE_NO_WARNINGS

    # Disable code assertions
    QMAKE_CFLAGS_RELEASE   *= /DNDEBUG
    QMAKE_CXXFLAGS_RELEASE *= /DNDEBUG
}

#CONFIG *= VALGRIND
VALGRIND {
    # Valgrind: compile in Release, with debug symbols and optimized debugging
    CONFIG -= debug
    CONFIG *= release
    QMAKE_CXXFLAGS_RELEASE *= -g
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2 -O3 -O0 -Os
    QMAKE_CXXFLAGS_RELEASE *= -O -fno-inline
}
