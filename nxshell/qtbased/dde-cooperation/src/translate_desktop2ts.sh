#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# dde-cooperation.desktop
DDE_COOPERATION_FILE=apps/dde-cooperation/setup/linux/dde-cooperation.desktop
DDE_COOPERATION_TS_DIR=../translations/dde-cooperation-desktop/
/usr/bin/deepin-desktop-ts-convert init $DDE_COOPERATION_FILE $DDE_COOPERATION_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts $DDE_COOPERATION_FILE $DDE_COOPERATION_TS_DIR

# deepin-data-transfer.desktop
DEEPIN_DATA_TRANSFER_FILE=apps/data-transfer/setup/linux/deepin-data-transfer.desktop
DEEPIN_DATA_TRANSFER_TS_DIR=../translations/deepin-data-transfer-desktop/
/usr/bin/deepin-desktop-ts-convert init $DEEPIN_DATA_TRANSFER_FILE $DEEPIN_DATA_TRANSFER_TS_DIR
/usr/bin/deepin-desktop-ts-convert desktop2ts $DEEPIN_DATA_TRANSFER_FILE $DEEPIN_DATA_TRANSFER_TS_DIR