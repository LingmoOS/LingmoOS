// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMargins>
#include <QColor>
#include <QSize>

namespace dwclock {
namespace UI {
namespace clock {
    static const QMargins defaultMargins(0, 0, 0, 0);
    static const QSize settingsDialogSize(360, 320);
    static const int spacingCityNameAndTimezoneOffset = 4;
    static const int spacingCityNameAndClock = 15;
    static const int cellSpacing = 20;
    static const QColor panelBackground(247, 247, 247, 0.6 * 255);
    static const QColor middleClockBackground(0, 0, 0, 0.05 * 255);
    static const int spacingBackgroundAndClock = 8;
    static const int smallSpacing = 10;
    static const int middleSpacing = 8;
}
namespace edit {
    static const QSize chooseDialogSize(360, 500);
    static const QColor choosePanelBackground(247, 247, 247, 0.8 * 255);
    static const int spacingTitle = 10;
    static const int spacingDesc = 6;
    static const QSize itemSize(340, 36);
    static const int sectionTitleIndent = 10;
}
}
}
