// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef COMMON_H
#define COMMON_H

#include <DLabel>

#include <QIcon>

DWIDGET_USE_NAMESPACE

/**
 * @class Common
 * @brief 公共类
*/

const static QString appName = "deepin-diskmanager";
class Common
{
public:

    /**
     * @brief 获取图标
     * @return 返回图标
    */
    static QIcon getIcon(const QString &iconName);

    /**
     * @brief 获取label自适应后的高度
     * @param width: label的宽度
     * @param text: label显示的文本
     * @param font: label字体样式
     * @return label当前高度
     */
    static int getLabelAdjustHeight(const int &width, const QString &text, const QFont &font);

};

#endif // COMMON_H
