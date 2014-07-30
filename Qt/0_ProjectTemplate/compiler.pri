*g++* {
    # Enable support for C++11 language revision
    QMAKE_CXXFLAGS *= -std=c++0x

    # Disable some warnings, make all the others into errors
    QMAKE_CXXFLAGS *= -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
    #QMAKE_CXXFLAGS *= -Werror

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
