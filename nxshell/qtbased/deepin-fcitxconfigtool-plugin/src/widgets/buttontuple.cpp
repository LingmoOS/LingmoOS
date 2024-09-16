// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttontuple.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <QDebug>

namespace dcc_fcitx_configtool {
namespace widgets {

FcitxButtonTuple::FcitxButtonTuple(ButtonType type, QWidget *parent)
    : QWidget(parent)
    , m_leftButton(new FcitxLeftButton)
{
    createRightButton(type);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);
    layout->addWidget(m_leftButton);
    layout->addWidget(m_rightButton);
    setLayout(layout);

    connect(m_leftButton, &QPushButton::clicked, this, &FcitxButtonTuple::leftButtonClicked);
    connect(m_rightButton, &QPushButton::clicked, this, &FcitxButtonTuple::rightButtonClicked);
}

QPushButton *FcitxButtonTuple::leftButton()
{
    return m_leftButton;
}

QPushButton *FcitxButtonTuple::rightButton()
{
    return m_rightButton;
}

void FcitxButtonTuple::removeSpacing()
{
    if (!this->layout())
        return;
    //第二个控件为space
    if (this->layout()->itemAt(1)) {
        this->layout()->removeItem(this->layout()->itemAt(1));
    }
}

void FcitxButtonTuple::createRightButton(const FcitxButtonTuple::ButtonType type)
{
    switch (type) {
    case Save:
        m_rightButton = new DSuggestButton();
        break;
    case Delete:
        m_rightButton = new DWarningButton();
        break;
    case Normal:
        Q_FALLTHROUGH();
    default:
        m_rightButton = new DPushButton();
        break;
    }
}
} // namespace widgets
} // namespace dcc_fcitx_configtool
