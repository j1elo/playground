#! /bin/bash

NOW=`date +%Y%m%d`

#BACKUP_DIR=$HOME/tmp/backup-$NOW/
#BACKUP_FILE=$HOME/backup-$NOW.tar.gz
#
BACKUP_DIR=/tmp/backup-$NOW/
BACKUP_FILE=$HOME/Dropbox/backups/backup-$NOW.tar.gz

COPY_PATHS='
~/.profile
~/.bash_aliases
~/.config/autostart
~/.config/user-dirs.dirs
~/bin/
~/doc-dev/
~/doc-opt/
/etc/apt/sources.list
/etc/apt/sources.list.d/
/etc/openvpn/
'

mkdir -p $BACKUP_DIR

for f in $COPY_PATHS ; do
    cp --parents --recursive "`eval echo ${f//>}`" $BACKUP_DIR
done

cd $BACKUP_DIR ; cd ../
tar zcf $BACKUP_FILE $(basename $BACKUP_DIR)
echo "Backup prepared into $BACKUP_DIR and packed in $BACKUP_FILE"
