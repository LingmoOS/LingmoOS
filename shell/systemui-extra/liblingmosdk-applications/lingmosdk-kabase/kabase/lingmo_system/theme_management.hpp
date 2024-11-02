/*
 * liblingmosdk-base's Library
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

#ifndef LINGMO_SYSTEM_THEME_MANAGEMENT_
#define LINGMO_SYSTEM_THEME_MANAGEMENT_

#include <QObject>
#include <QWidget>
#include <QAbstractButton>

namespace kdk
{
namespace kabase
{

class ThemeManagement
{
public:
    enum Property {
        MinButton = 0,                   /* 最小化按钮 */
        MaxButton,                       /* 最大化按钮 */
        CloseButton,                     /* 关闭按钮 */
        ButtonThreeStateGrey,            /* 按钮三态灰色背景 */
        ButtonThreeStateBlue,            /* 按钮三态红色背景 */
        IconDefaultHighlight,            /* 主题切换对图标进行处理 */
        IconSelectClickHighlight,        /* 选中 , 点击 对图标高亮处理 */
        IconDefaultSelectClickHighlight, /* 主题切换对图标进行处理 , 选中 , 点击 对图标高亮处理 */
        IconNonSolidColorHighlight       /* 对非纯色的 symbolic 图标处理 */
    };

    ThemeManagement();
    ~ThemeManagement();

    static bool setProperty(QObject *object, Property property);
    static bool setPaletteColor(QWidget *widget);
    static bool setMinIcon(QAbstractButton *button);
    static bool setMaxIcon(QAbstractButton *button);
    static bool setCloseIcon(QAbstractButton *button);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
