#!/bin/bash

set -e

URL="https://packages.lingmo.org/release/"
LOGS_TYPE="changelogs.md"
ALL_SAVE="/var/update/"
DEFAULT_LOGS_SAVE= "/var/update/cache/"

SYS_LANGUAGE=echo $LANG

if [ ! -f "$ALL_SAVE" ]; then
    mkdir $ALL_SAVE
    mkdir $DEFAULT_LOGS_SAVE
fi

wget -P -O $DEFAULT_LOGS_SAVE $URL$SYS_LANGUAGE"/"$LOGS_TYPE