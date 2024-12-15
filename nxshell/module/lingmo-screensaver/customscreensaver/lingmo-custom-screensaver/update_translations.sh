#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro lingmo-custom-screensaver.pro -ts translations/lingmo-custom-screensaver.ts -no-obsolete
lupdate -pro lingmo-custom-screensaver.pro -ts translations/lingmo-custom-screensaver_zh_CN.ts -no-obsolete
