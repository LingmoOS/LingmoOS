// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "closeButton.h"

#include <QHBoxLayout>


CloseButton::CloseButton(QWidget *parent) :
        QWidget(parent)
{

    setObjectName("closeButton");
    m_label = new QLabel;
    m_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_label->setText("×");
    m_label->setObjectName("label");

    m_mainWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout;
    m_mainWidget->setObjectName("mainWidget");
    m_mainWidget->setLayout(mainLayout);
    mainLayout->addWidget(m_label);
    mainLayout->setAlignment(m_label, Qt::AlignHCenter);


    auto *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_mainWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(centralLayout);

    onThemeChanged(Dtk::Gui::DGuiApplicationHelper::instance()->themeType());
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged, this, &CloseButton::onThemeChanged);
}

CloseButton::~CloseButton()
{

}

void CloseButton::mousePressEvent(QMouseEvent *event)
{
    Q_EMIT clicked();
}

void CloseButton::onThemeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType type)
{
    if (type == Dtk::Gui::DGuiApplicationHelper::ColorType::LightType) {

        m_mainWidget->setStyleSheet("QWidget#mainWidget{"
                              "background-color: rgba(255, 255, 255, 0.8);"
                              "}""QWidget#mainWidget:hover{background-color: rgba(255, 255, 255, 0.01)}");

        m_label->setStyleSheet("QLabel {"
                               "font-size: 18px;"
                               "font-weight: 100;"
                               "color: #626e88;"
                               "}""QLabel#label:disabled{color: #aaa5a5}");


    } else if (type == Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType) {

        m_mainWidget->setStyleSheet("QWidget#mainWidget{"
                                    "background-color: rgba(100, 100, 100, 0.05);"
                                    "}""QWidget#mainWidget:hover{background-color: rgba(100, 100, 100, 0.2)}");

        m_label->setStyleSheet("QLabel {"
                               "font-size: 18px;"
                               "font-weight: 100;"
                               "color: #c0c6d4;"
                               "}""QLabel#label:disabled{color: #454343}");

    }
}
