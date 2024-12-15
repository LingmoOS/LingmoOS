#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# ocean-computer.desktop
DESKTOP_COMPUTER_TEMP_FILE=apps/ocean-desktop/data/applications/ocean-computer.desktop.tmp
DESKTOP_COMPUTER_FILE=apps/ocean-desktop/data/applications/ocean-computer.desktop
DESKTOP_COMPUTER_TS_DIR=../translations/ocean-computer-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_COMPUTER_FILE $DESKTOP_COMPUTER_TS_DIR $DESKTOP_COMPUTER_TEMP_FILE
mv $DESKTOP_COMPUTER_TEMP_FILE $DESKTOP_COMPUTER_FILE

# ocean-trash.desktop
DESKTOP_TRASH_TEMP_FILE=apps/ocean-desktop/data/applications/ocean-trash.desktop.tmp
DESKTOP_TRASH_FILE=apps/ocean-desktop/data/applications/ocean-trash.desktop
DESKTOP_TRASH_TS_DIR=../translations/ocean-trash-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_TRASH_FILE $DESKTOP_TRASH_TS_DIR $DESKTOP_TRASH_TEMP_FILE
mv $DESKTOP_TRASH_TEMP_FILE $DESKTOP_TRASH_FILE

# ocean-home.desktop
DESKTOP_HOME_TEMP_FILE=apps/ocean-desktop/data/applications/ocean-home.desktop.tmp
DESKTOP_HOME_FILE=apps/ocean-desktop/data/applications/ocean-home.desktop
DESKTOP_HOME_TS_DIR=../translations/ocean-home-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_HOME_FILE $DESKTOP_HOME_TS_DIR $DESKTOP_HOME_TEMP_FILE
mv $DESKTOP_HOME_TEMP_FILE $DESKTOP_HOME_FILE

# ocean-file-manager.desktop
DESKTOP_OCEAN_FILE_MANAGER_TEMP_FILE=src/apps/ocean-file-manager/ocean-file-manager.desktop.tmp
DESKTOP_OCEAN_FILE_MANAGER_FILE=src/apps/ocean-file-manager/ocean-file-manager.desktop
DESKTOP_OCEAN_FILE_MANAGER_TS_DIR=../translations/ocean-file-manager-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_OCEAN_FILE_MANAGER_FILE $DESKTOP_OCEAN_FILE_MANAGER_TS_DIR $DESKTOP_OCEAN_FILE_MANAGER_TEMP_FILE
mv $DESKTOP_OCEAN_FILE_MANAGER_TEMP_FILE $DESKTOP_OCEAN_FILE_MANAGER_FILE

# ocean-open.desktop
# DESKTOP_OCEAN_OPEN_TEMP_FILE=apps/ocean-file-manager/ocean-open.desktop.tmp
# DESKTOP_OCEAN_OPEN_FILE=apps/ocean-file-manager/ocean-open.desktop
# DESKTOP_OCEAN_OPEN_TS_DIR=../translations/ocean-open-desktop/

# /usr/bin/lingmo-desktop-ts-convert ts2desktop $DESKTOP_OCEAN_OPEN_FILE $DESKTOP_OCEAN_OPEN_TS_DIR $DESKTOP_OCEAN_OPEN_TEMP_FILE
# mv $DESKTOP_OCEAN_OPEN_TEMP_FILE $DESKTOP_OCEAN_OPEN_FILE