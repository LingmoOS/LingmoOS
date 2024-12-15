#!/bin/bash

DESKTOP_SOURCE_FILE=ocean-device-formatter.desktop
DESKTOP_TS_DIR=translations/ocean-device-formatter-desktop/

/usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR
