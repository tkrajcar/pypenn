#! /bin/sh

# Backup stuff.

EXCLUDE_LIST="backup.exclude"
DESTNAME="kilmush.`date +%Y-%m-%d.%H`"
mysqldump -e -R e_mush -ue_mush -pWn5qnHyUhXmMTXbA > mush.sql
tar -z -c -v -f "../backups/${DESTNAME}.tgz" -X "${EXCLUDE_LIST}" *
