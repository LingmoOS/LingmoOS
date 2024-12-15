// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include "style.h"

DGUI_USE_NAMESPACE

CicleIconButton::CicleIconButton(QWidget *parent)
    : DIconButton(parent)
{
    setEnabledCircle(true);
    updateBackground(UI::Panel::buttonBackground);
}

void CicleIconButton::updateBackground(const QColor &color)
{
    auto pt = palette();
    pt.setBrush(QPalette::Button, color);
    setPalette(pt);
}
