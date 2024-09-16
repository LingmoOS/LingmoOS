// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themewidget.h"
#include "unionimage/baseutils.h"

//#include "application.h"
#include "accessibility/ac-desktop-define.h"

ThemeWidget::ThemeWidget(const QString &darkFile, const QString &lightFile,
                         QWidget *parent)
    : QWidget(parent)
{

    m_darkStyle = Libutils::base::getFileContent(darkFile);
    m_lightStyle = Libutils::base::getFileContent(lightFile);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    onThemeChanged(themeType);
    setObjectName(THEME_WIDGET);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
                     this, &ThemeWidget::onThemeChanged);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ThemeWidget::onThemeChanged);
}

ThemeWidget::~ThemeWidget() {}

void ThemeWidget::onThemeChanged(DGuiApplicationHelper::ColorType theme)
{
    if (theme == DGuiApplicationHelper::ColorType::DarkType) {
        m_deepMode = true;
    } else {
        m_deepMode = false;
    }
}
