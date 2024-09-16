// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSize>
#include <QColor>
#include <QMargins>

namespace Notify {
namespace UI {

namespace Panel{
    static const QColor buttonBackground(0, 0, 0, 0.1 * 255);
    static const QSize buttonSize(24, 24);
    static const QSize settingsIconSize(12, 12);
    static const QSize clearIconSize(settingsIconSize);
    static const QSize collapseIconSize(settingsIconSize);
    static const int noNotifyLabelFontAlpha(0.6 * 255);
    static const int settingSpacingBetweenMenu(4);
}
namespace Bubble {
    static const QSize buttonSize(24, 24);
    static const QSize settingsIconSize(10, 10);
    static const QSize clearIconSize(settingsIconSize);
    static const QSize collapseIconSize(settingsIconSize);
}
}
}
using namespace Notify;
