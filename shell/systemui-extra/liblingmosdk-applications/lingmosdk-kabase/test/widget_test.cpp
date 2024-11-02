/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QDebug>

#include "window_management.hpp"
#include "theme_management.hpp"
#include "widget_test.hpp"

Widget::Widget()
{
    /* 设置窗管协议 */
    kdk::kabase::WindowManagement::setWindowMotifHint(winId());

    /* 设置主题窗口按钮灰色背景 */
    kdk::kabase::ThemeManagement::setProperty(this, kdk::kabase::ThemeManagement::Property::ButtonThreeStateBlue);
}

Widget::~Widget() {}
