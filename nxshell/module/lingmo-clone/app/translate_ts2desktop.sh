#!/bin/bash

DESKTOP_TEMP_FILE=lingmo-clone.desktop.tmp
DESKTOP_SOURCE_FILE=lingmo-clone.desktop
DESKTOP_TS_DIR=translations/desktop/

lingmo-desktop-ts-convert ts2desktop $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_TEMP_FILE
mv $DESKTOP_TEMP_FILE $DESKTOP_SOURCE_FILE
