/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "explor-main-window-style.h"

static PeonyMainWindowStyle *global_instance = nullptr;

PeonyMainWindowStyle *PeonyMainWindowStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new PeonyMainWindowStyle;
    }
    return global_instance;
}

PeonyMainWindowStyle::PeonyMainWindowStyle(QObject *parent) : QProxyStyle()
{

}

int PeonyMainWindowStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_LayoutVerticalSpacing:
    case PM_LayoutTopMargin:
    case PM_LayoutLeftMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
    case PM_LayoutHorizontalSpacing:
    case PM_DockWidgetFrameWidth:
    case PM_DockWidgetTitleMargin:
    case PM_DockWidgetTitleBarButtonMargin:
        return 0;
    case PM_DockWidgetHandleExtent:
        return 2;
    case PM_DockWidgetSeparatorExtent:
        return 2;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}
