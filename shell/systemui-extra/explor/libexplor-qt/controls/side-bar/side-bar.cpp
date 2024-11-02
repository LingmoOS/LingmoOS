/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "side-bar.h"
#include "global-settings.h"

using namespace Peony;

SideBar *last_resize_sidebar = nullptr;

SideBar::SideBar(QWidget *parent) : QDockWidget(parent)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);
//    auto palette = palette();
//    palette.setColor(QPalette::Window, Qt::transparent);
//    setPalette(palette);
    setTitleBarWidget(new QWidget(this));
    titleBarWidget()->setFixedHeight(0);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0, 0, 0, 0);
}

SideBar::~SideBar()
{
    if (last_resize_sidebar == this) {
        GlobalSettings::getInstance()->setValue(DEFAULT_SIDEBAR_WIDTH, this->width());
        last_resize_sidebar = nullptr;
    }
}

QSize SideBar::sizeHint() const
{
    auto size = QWidget::sizeHint();
    auto width = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_SIDEBAR_WIDTH).toInt();
    //fix width value abnormal issue
    if (width <= 0)
        width = 210;
    size.setWidth(width);
    return size;
}

void SideBar::resizeEvent(QResizeEvent *event)
{
    QDockWidget::resizeEvent(event);
    last_resize_sidebar = this;
}

FMWindowIface *SideBar::getWindowIface()
{
    return dynamic_cast<FMWindowIface *>(topLevelWidget());
}
