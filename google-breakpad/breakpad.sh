#! /bin/bash

# Qt startup script for X11 applications
# See: http://qt-project.org/doc/qt-5.1/qtdoc/deployment-x11.html

appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
    dirname=$PWD/$dirname
fi

export LD_LIBRARY_PATH=$dirname:$LD_LIBRARY_PATH
$dirname/$appname "$@" &> $dirname/$appname.log
