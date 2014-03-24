# ~/.profile: executed by the command interpreter for login shells.
# This file is not read by bash(1), if ~/.bash_profile or ~/.bash_login
# exists.
# see /usr/share/doc/bash/examples/startup-files for examples.
# the files are located in the bash-doc package.

# the default umask is set in /etc/profile; for setting the umask
# for ssh logins, install and configure the libpam-umask package.
#umask 022

# if running bash
if [ -n "$BASH_VERSION" ]; then
    # include .bashrc if it exists
    if [ -f "$HOME/.bashrc" ]; then
        . "$HOME/.bashrc"
    fi
fi

# set PATH so it includes user's private bin if it exists
if [ -d "$HOME/bin" ] ; then
    PATH="$HOME/bin:$PATH"
fi



# ---- SOFTWARE SETUP ---- #
# ======================== #

## Optional setup:
## * Personal scripts should go in $HOME/bin/
## * apt-get install ccache

# make
export MAKEFLAGS="-j12"

# ccache
export PATH=/usr/lib/ccache:$PATH

# Valgrind
export PATH=/opt/valgrind-3.9.0/bin:$PATH

# cppcheck
export PATH=/opt/cppcheck-1.62/bin:$PATH

# meld
export PATH=/opt/meld-1.8.3/bin:$PATH
export XDG_DATA_DIRS=/opt/meld-1.8.3/share:$XDG_DATA_DIRS

# GammaRay
export PATH=/opt/GammaRay/bin:$PATH
export LD_LIBRARY_PATH=/opt/GammaRay/lib:$LD_LIBRARY_PATH

# KDESvn
export PATH=/opt/kdesvn-20140109/bin:$PATH
export LD_LIBRARY_PATH=/opt/kdesvn-20140109/lib:$LD_LIBRARY_PATH
export XDG_DATA_DIRS=/opt/kdesvn-20140109/share:$XDG_DATA_DIRS

# Smb4K
export PATH=/opt/smb4k/bin:$PATH
export LD_LIBRARY_PATH=/opt/smb4k/lib:$LD_LIBRARY_PATH

# Redshift
export PATH=/opt/redshift-1.8/bin:$PATH
export XDG_DATA_DIRS=/opt/redshift-1.8/share:$XDG_DATA_DIRS
export PYTHONPATH=/opt/redshift-1.8/lib/python2.7/site-packages:$PYTHONPATH

# OpenCV
export LD_LIBRARY_PATH=/opt/opencv-2.4.7/lib:$LD_LIBRARY_PATH

# Charm time tracker
export PATH=/opt/charm/bin:$PATH
export XDG_DATA_DIRS=/opt/charm/share:$XDG_DATA_DIRS

# Firefox portable
export PATH=/opt/FirefoxPortable:$PATH

# Qt Creator
export PATH=/opt/qtcreator-3.0.0/bin:$PATH
