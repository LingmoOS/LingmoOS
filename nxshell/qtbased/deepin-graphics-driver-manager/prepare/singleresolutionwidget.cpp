// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleresolutionwidget.h"
#include "utils.h"

#include <QLabel>
#include <QHBoxLayout>

SingleResolutionWidget::SingleResolutionWidget(ComDeepinDaemonGraphicsDriverInterface *graphicsDriver, const Resolution &resolution, QWidget *parent)
    : ResolutionWidget(graphicsDriver,  resolution,  parent)
{

}

void SingleResolutionWidget::initUI()
{
    ResolutionWidget::initUI();
    m_title->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_version->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_description->setVisible(false);
    setContentsMargins(0, 50, 0, 10);
}

void SingleResolutionWidget::onThemeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType type)
{
    if (type == Dtk::Gui::DGuiApplicationHelper::ColorType::LightType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setThemeType(type);
        m_version->setStyleSheet("QLabel {"
                                 "color: #001A2E;"
                                 "}");

        m_title->setStyleSheet("QLabel {"
                               "font-weight: 500;"
                               "color: #414d68;"
                               "}");

        m_description->setStyleSheet("QLabel {"
                                     "color: #526a7f;"
                                     "}");

    } else if (type == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType) {
        Dtk::Gui::DGuiApplicationHelper::instance()->setThemeType(type);
        m_version->setStyleSheet("QLabel {"
                                 "color: #c0c6d4;"
                                 "}");

        m_title->setStyleSheet("QLabel {"
                               "font-weight: 500;"
                               "color: #C0C6D4;"
                               "}");

        m_description->setStyleSheet("QLabel {"
                                     "color: #6d7c88;"
                                     "}");
    }
}

void SingleResolutionWidget::setChecked(const bool checked)
{
    m_checked = checked;
    m_checkedBtn->setVisible(false);
}
