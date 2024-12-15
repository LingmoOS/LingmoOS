#!/bin/bash

DESKTOP_SOURCE_FILE=org.lingmo.ocean.control-center.desktop
DESKTOP_TS_DIR=../translations/desktop/

/usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR
