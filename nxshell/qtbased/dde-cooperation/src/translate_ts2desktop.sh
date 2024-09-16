#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# dde-cooperation.desktop
DDE_COOPERATION_TEMP_FILE=apps/dde-cooperation/res/linux/dde-cooperation.desktop.tmp
DDE_COOPERATION_FILE=apps/dde-cooperation/res/linux/dde-cooperation.desktop
DDE_COOPERATION_TS_DIR=../translations/dde-cooperation-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DDE_COOPERATION_FILE $DDE_COOPERATION_TS_DIR $DDE_COOPERATION_TEMP_FILE
mv $DDE_COOPERATION_TEMP_FILE $DDE_COOPERATION_FILE

# deepin-data-transfer.desktop
DEEPIN_DATA_TRANSFER_TEMP_FILE=apps/data-transfer/res/linux/deepin-data-transfer.desktop.tmp
DEEPIN_DATA_TRANSFER_FILE=apps/data-transfer/res/linux/deepin-data-transfer.desktop
DEEPIN_DATA_TRANSFER_TS_DIR=../translations/deepin-data-transfer-desktop/

/usr/bin/deepin-desktop-ts-convert ts2desktop $DEEPIN_DATA_TRANSFER_FILE $DEEPIN_DATA_TRANSFER_TS_DIR $DEEPIN_DATA_TRANSFER_TEMP_FILE
mv $DEEPIN_DATA_TRANSFER_TEMP_FILE $DEEPIN_DATA_TRANSFER_FILE
