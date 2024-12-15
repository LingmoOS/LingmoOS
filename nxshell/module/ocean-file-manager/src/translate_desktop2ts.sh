#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# ocean-computer.desktop
DESKTOP_COMPUTER_FILE=apps/ocean-desktop/data/applications/ocean-computer.desktop
DESKTOP_COMPUTER_TS_DIR=../translations/ocean-computer-desktop/
/usr/bin/lingmo-desktop-ts-convert init $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR

# ocean-home.desktop
DESKTOP_HOME_FILE=apps/ocean-desktop/data/applications/ocean-home.desktop
DESKTOP_HOME_TS_DIR=../translations/ocean-home-desktop/
/usr/bin/lingmo-desktop-ts-convert init $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR

# ocean-trash.desktop
DESKTOP_TRASH_FILE=apps/ocean-desktop/data/applications/ocean-trash.desktop
DESKTOP_TRASH_TS_DIR=../translations/ocean-trash-desktop/
/usr/bin/lingmo-desktop-ts-convert init $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR

# ocean-file-manager.desktop
DESKTOP_OCEAN_FILE_MANAGER_FILE=src/apps/ocean-file-manager/ocean-file-manager.desktop
DESKTOP_OCEAN_FILE_MANAGER_TS_DIR=../translations/ocean-file-manager-desktop/
/usr/bin/lingmo-desktop-ts-convert init $DESKTOP_OCEAN_FILE_MANAGER_FILE $DESKTOP_OCEAN_FILE_MANAGER_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts  $DESKTOP_OCEAN_FILE_MANAGER_FILE $DESKTOP_OCEAN_FILE_MANAGER_TS_DIR

# ocean-open.desktop
# DESKTOP_OCEAN_OPEN_FILE=apps/ocean-file-manager/ocean-open.desktop
# DESKTOP_OCEAN_OPEN_TS_DIR=../translations/ocean-open-desktop/
# /usr/bin/lingmo-desktop-ts-convert init $DESKTOP_OCEAN_OPEN_FILE $DESKTOP_OCEAN_OPEN_TS_DIR
# /usr/bin/lingmo-desktop-ts-convert desktop2ts $DESKTOP_OCEAN_OPEN_FILE $DESKTOP_OCEAN_OPEN_TS_DIR