#!/bin/bash

DESKTOP_TEMP_FILE=deepin-clone.desktop.tmp
DESKTOP_SOURCE_FILE=deepin-clone.desktop
DESKTOP_TS_DIR=translations/desktop/

deepin-desktop-ts-convert ts2desktop $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_TEMP_FILE
mv $DESKTOP_TEMP_FILE $DESKTOP_SOURCE_FILE
