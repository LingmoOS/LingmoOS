#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# ocean-cooperation.desktop
OCEAN_COOPERATION_TEMP_FILE=apps/ocean-cooperation/res/linux/ocean-cooperation.desktop.tmp
OCEAN_COOPERATION_FILE=apps/ocean-cooperation/res/linux/ocean-cooperation.desktop
OCEAN_COOPERATION_TS_DIR=../translations/ocean-cooperation-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $OCEAN_COOPERATION_FILE $OCEAN_COOPERATION_TS_DIR $OCEAN_COOPERATION_TEMP_FILE
mv $OCEAN_COOPERATION_TEMP_FILE $OCEAN_COOPERATION_FILE

# lingmo-data-transfer.desktop
LINGMO_DATA_TRANSFER_TEMP_FILE=apps/data-transfer/res/linux/lingmo-data-transfer.desktop.tmp
LINGMO_DATA_TRANSFER_FILE=apps/data-transfer/res/linux/lingmo-data-transfer.desktop
LINGMO_DATA_TRANSFER_TS_DIR=../translations/lingmo-data-transfer-desktop/

/usr/bin/lingmo-desktop-ts-convert ts2desktop $LINGMO_DATA_TRANSFER_FILE $LINGMO_DATA_TRANSFER_TS_DIR $LINGMO_DATA_TRANSFER_TEMP_FILE
mv $LINGMO_DATA_TRANSFER_TEMP_FILE $LINGMO_DATA_TRANSFER_FILE
