#!/bin/bash

DESKTOP_SOURCE_FILE=lingmo-draw.desktop
DESKTOP_TS_DIR=translations/desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_SOURCE_FILE
