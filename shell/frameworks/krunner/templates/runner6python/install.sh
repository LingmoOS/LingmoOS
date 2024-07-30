#!/bin/bash

# Standalone install script for copying files

set -e

prefix="${XDG_DATA_HOME:-$HOME/.local/share}"
krunner_dbusdir="$prefix/krunner/dbusplugins"
services_dir="$prefix/dbus-1/services/"

mkdir -p $krunner_dbusdir
mkdir -p $services_dir

cp %{APPNAMELC}.desktop $krunner_dbusdir
printf "[D-BUS Service]\nName=org.kde.%{APPNAMELC}\nExec=\"$PWD/main.py\"" > $services_dir/org.kde.%{APPNAMELC}.service

kquitapp6 krunner

