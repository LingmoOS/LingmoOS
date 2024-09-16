// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    required property AppItemPalette palette
    radius: width * 0.2
    color: palette.rectIndicator
}
