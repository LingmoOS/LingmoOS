// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DIconButton>

DWIDGET_USE_NAMESPACE
/*!
 * \~chinese \class IconButton
 * \~chinese \brief 自定义按钮类
 */
class CicleIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit CicleIconButton(QWidget *parent = nullptr);

    void updateBackground(const QColor &color);
};
