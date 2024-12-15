#!/bin/bash
# this file is used to auto update ts file.

lupdate -pro ocean-device-formatter.pro -ts translations/ocean-device-formatter.ts -no-obsolete
lupdate -pro ocean-device-formatter.pro -ts translations/ocean-device-formatter_zh_CN.ts -no-obsolete
