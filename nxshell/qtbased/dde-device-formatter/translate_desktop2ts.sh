#!/bin/bash

DESKTOP_SOURCE_FILE=dde-device-formatter.desktop
DESKTOP_TS_DIR=translations/dde-device-formatter-desktop/

/usr/bin/deepin-desktop-ts-convert desktop2ts $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR
