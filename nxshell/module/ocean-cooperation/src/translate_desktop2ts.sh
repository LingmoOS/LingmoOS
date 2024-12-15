#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# ocean-cooperation.desktop
OCEAN_COOPERATION_FILE=apps/ocean-cooperation/setup/linux/ocean-cooperation.desktop
OCEAN_COOPERATION_TS_DIR=../translations/ocean-cooperation-desktop/
/usr/bin/lingmo-desktop-ts-convert init $OCEAN_COOPERATION_FILE $OCEAN_COOPERATION_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts $OCEAN_COOPERATION_FILE $OCEAN_COOPERATION_TS_DIR

# lingmo-data-transfer.desktop
LINGMO_DATA_TRANSFER_FILE=apps/data-transfer/setup/linux/lingmo-data-transfer.desktop
LINGMO_DATA_TRANSFER_TS_DIR=../translations/lingmo-data-transfer-desktop/
/usr/bin/lingmo-desktop-ts-convert init $LINGMO_DATA_TRANSFER_FILE $LINGMO_DATA_TRANSFER_TS_DIR
/usr/bin/lingmo-desktop-ts-convert desktop2ts $LINGMO_DATA_TRANSFER_FILE $LINGMO_DATA_TRANSFER_TS_DIR