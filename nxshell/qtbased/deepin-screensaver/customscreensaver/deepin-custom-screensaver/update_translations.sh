#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# this file is used to auto update ts file.

lupdate -pro deepin-custom-screensaver.pro -ts translations/deepin-custom-screensaver.ts -no-obsolete
lupdate -pro deepin-custom-screensaver.pro -ts translations/deepin-custom-screensaver_zh_CN.ts -no-obsolete
