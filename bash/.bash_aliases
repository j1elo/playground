## This is ~/.bash_aliases (for Debian/Ubuntu),
## you should source this from ~/.bashrc
##
## Optional setup:
## * apt-get install bash-completion lsof


# Enable automatic typo correction for directory names.
shopt -s cdspell

# Enable case-insensitive autocompletion.
shopt -s nocaseglob



# ---- SHORTHANDS ---- #
# ==================== #

alias la='ls -lAh'
alias pss='ps -eF | grep -i $1'
alias ifconfig='LANG=en_US /sbin/ifconfig'
alias apt-reverse='apt-cache --installed rdepends'

# Find text including subdirectories:
# http://askubuntu.com/questions/55325/how-to-use-grep-command-to-find-text-including-subdirectories
f-grep () { grep -Fril "$1" $PWD; }

f-ip () { host `hostname` | head -1 | awk '{ print $NF }'; }
f-delsvn () { rm -rfv `find . -name .svn -type d -o -name svn-commit.tmp*`; }
f-delgit () { rm -rfv `find . -name .git -type d -o -name .gitignore`; }
f-list-disks () { sudo fdisk -l | grep /dev/[s,h]d..*83.*Linux$ | awk '{print $1}' | while read p; do echo "${p}: $(sudo e2label $p)"; done; }
f-timewasters () { ps aux --sort=-%cpu | grep --max-count=11 --invert-match `whoami`; }
f-dirsizes () { du -k -- "$@" | sort -n; } # TODO: use -h to enable human-friendly sizes.
f-dirtasks () { lsof +D "$1"; }
f-list-users () { awk -F":" '{ print "name=" $1 " uid=" $3 " gid=" $4 " comment=" $5 " home="$6 }' /etc/passwd; }
f-list-groups () { awk -F":" '{ print "name=" $1 " gid=" $3 " users=" $4 }' /etc/group; }

f-setchmod () {
    find . -name '.svn' -prune , -type d -exec chmod 775 '{}' ';'
    find . -name '.svn' -prune , -type f -exec chmod 664 '{}' ';'
    find . -name '.svn' -prune , \( \
        -name '*.sh' \
        -o -name '*.run' \
        -o -name '*.bin' \
        -o -name '*.so' \
        -o -name '*.so*' \
        \) -exec chmod +x '{}' ';'
}

f-setunixeol () {
    find . -name '.svn' -prune , \( \
        -name '*.pro' \
        -o -name '*.pri' \
        -o -name '*.qrc' \
        -o -name '*.ui' \
        -o -name '*.h' \
        -o -name '*.hpp' \
        -o -name '*.c' \
        -o -name '*.cc' \
        -o -name '*.cpp' \
        -o -name '*.txt' \
        -o -name '*.sh' \
        -o -name '*.xml' \
        \) -exec dos2unix --keepdate '{}' ';'
}

f-extract () {
    if [ -f $1 ] ; then
        case $1 in
            *.tar) tar xvf $1 && cd $(basename "$1" .tar) ;;
            *.tar.gz) tar xzvf $1 && cd $(basename "$1" .tar.gz) ;;
            *.tgz)    tar xzvf $1 && cd $(basename "$1" .tgz) ;;
            *.gz)     gunzip $1 && cd $(basename "$1" .gz) ;;
            *.tar.bz2) tar xjvf $1 && cd $(basename "$1" .tar.bz2) ;;
            *.tbz)     tar xjvf $1 && cd $(basename "$1" .tbz) ;;
            *.tbz2)    tar xjvf $1 && cd $(basename "$1" .tbz2) ;;
            *.bz2)     bunzip2 $1 && cd $(basename "$1" .bz2) ;;
            *.tar.Z) tar xZvf $1 && cd $(basename "$1" .tar.Z) ;;
            *.taz)   tar xZvf $1 && cd $(basename "$1" .taz) ;;
            *.Z)     uncompress $1 && cd $(basename "$1" .Z) ;;
            *.tar.lz) tar --lzip -xvf $1 && cd $(basename "$1" .tar.lz) ;;
            *.tlz)    tar --lzip -xvf $1 && cd $(basename "$1" .tlz) ;;
            *.tar.xz) tar xJvf $1 && cd $(basename "$1" .tar.xz) ;;
            *.txz)    tar xJvf $1 && cd $(basename "$1" .txz) ;;
            *.xz)     unxz -dkv $1 && cd $(basename "$1" .xz) ;;
            *.rar) unrar x $1 && cd $(basename "$1" .rar) ;;
            *.zip) unzip $1 && cd $(basename "$1" .zip) ;;
            *.7z)  7z x $1 && cd $(basename "$1" .7z) ;;
            *) echo "Don't know how to extract '$1'..." ;;
        esac
    else
        echo "'$1' is not a valid file!"
    fi
}

duf () {
    du -k "$@" | sort -n | while read size fname; do for unit in k M G T P E Z Y; do if [ $size -lt 1024 ]; then echo -e "${size}${unit}\t${fname}"; break; fi; size=$((size/1024)); done; done
}



# ---- DEVELOPMENT ---- #
# ===================== #

# To force execution of these funcions, their content should be copied into ~/.profile
# This is a limitation of LightDM, which doesn't support Bash syntax for exporting functions.

# Kshow libraries
setenv-kshow () {
    export KSHOWDIR=$HOME/dev/kshowpro
    export LD_LIBRARY_PATH=$KSHOWDIR/ExternalDeps/lib/linux/64bits:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$KSHOWDIR/bin:$LD_LIBRARY_PATH
    echo "\$LD_LIBRARY_PATH set for Kshow in $KSHOWDIR"
}
export -f setenv-kshow

# Qt-4.8.4-debug
setenv-qt484-debug () {
    export QTDIR=/opt/Qt-4.8.4-debug
    export PATH=$QTDIR/bin:$PATH
    export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
    #export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig
    #export QT_PLUGIN_PATH=$QTDIR/plugins
    echo "\$PATH and \$LD_LIBRARY_PATH set for Qt in $QTDIR"
}
export -f setenv-qt484-debug

# Qt-4.8.4-release
setenv-qt484-release() {
    export QTDIR=/opt/Qt-4.8.4-release
    export PATH=$QTDIR/bin:$PATH
    export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
    #export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig
    #export QT_PLUGIN_PATH=$QTDIR/plugins
    echo "\$PATH and \$LD_LIBRARY_PATH set for Qt in $QTDIR"
}
export -f setenv-qt484-release

# Qt-5.2.1-debug
setenv-qt521-debug() {
    export QTDIR=/opt/Qt-5.2.1-debug
    export PATH=$QTDIR/bin:$PATH
    export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
    #export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig
    #export QT_PLUGIN_PATH=$QTDIR/plugins
    echo "\$PATH and \$LD_LIBRARY_PATH set for Qt in $QTDIR"
}
export -f setenv-qt521-debug

# Android SDK
#export PATH=$HOME/Android/android-sdk-linux/tools:$PATH
#export LD_LIBRARY_PATH=$HOME/Android/android-sdk-linux/tools/lib:$LD_LIBRARY_PATH



# ---- ~/.bash_aliases - LAST LINE
echo "~/.bash_aliases LOADED"
