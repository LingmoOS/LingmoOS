// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipslabel.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

TipsLabel::TipsLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TipsLabel::onThemeChanged);

    onThemeChanged();
}

void TipsLabel::onThemeChanged()
{
    QPalette p(palette());
    QColor color = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
            ? QColor(0, 0, 0, 255 * 0.4) : QColor(255, 255, 255, 255 * 0.4);
    p.setColor(QPalette::Active, QPalette::WindowText, color);
    setPalette(p);
}
