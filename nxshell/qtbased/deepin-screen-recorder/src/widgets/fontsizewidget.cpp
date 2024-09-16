// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontsizewidget.h"

#include <QHBoxLayout>
#include <QDebug>

#include "../utils/baseutils.h"
#include "../utils/configsettings.h"

const QSize BUTTON_SIZE = QSize(20, 16);
const QSize LINE_EDIT_SIZE = QSize(43, 16);

Separator::Separator(DWidget *parent)
    : DLabel(parent)
{
    setFixedSize(1, 16);
}

Separator::~Separator() {}

FontSizeWidget::FontSizeWidget(DWidget *parent)
    : DLabel(parent)
{
    initWidget();
}

void FontSizeWidget::initWidget()
{
    setObjectName("FontSizeWidget");
//    setStyleSheet(getFileContent(":/resources/qss/fontsizewidget.qss"));
    setFixedSize(86, 18);
    m_fontSizeEdit = new DLineEdit(this);
    m_fontSizeEdit->setObjectName("FontSizeEdit");
    m_fontSizeEdit->setFixedSize(LINE_EDIT_SIZE);

    m_fontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();
    m_fontSizeEdit->setText(QString("%1").arg(m_fontSize));
    m_addSizeBtn = new DPushButton(this);
    m_addSizeBtn->setObjectName("AddSizeBtn");
    m_addSizeBtn->setFixedSize(BUTTON_SIZE);
    m_reduceSizeBtn = new DPushButton(this);
    m_reduceSizeBtn->setObjectName("ReduceSizeBtn");
    m_reduceSizeBtn->setFixedSize(BUTTON_SIZE);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(4);
    layout->addWidget(m_fontSizeEdit);
    layout->addSpacing(0);
    layout->addWidget( new Separator(this));
    layout->addWidget(m_addSizeBtn);
    layout->addWidget( new Separator(this));
    layout->addSpacing(0);
    layout->addWidget(m_reduceSizeBtn);
    layout->addStretch();
    setLayout(layout);

    connect(m_addSizeBtn, &DPushButton::clicked, [ = ] {
        adjustFontSize(true);
    });
    connect(m_reduceSizeBtn, &DPushButton::clicked, [ = ] {
        adjustFontSize(false);
    });
}

void FontSizeWidget::setFontSize(int fontSize)
{
    m_fontSize = fontSize;
}

void FontSizeWidget::adjustFontSize(bool add)
{
    if (add) {
        m_fontSize = m_fontSize + 1;
        m_fontSize = std::min(m_fontSize, 72);
    } else {
        m_fontSize = m_fontSize - 1;
        m_fontSize = std::max(9, m_fontSize);
    }

    m_fontSizeEdit->setText(QString("%1").arg(m_fontSize));
    emit fontSizeChanged(m_fontSize);

    connect(this, &FontSizeWidget::fontSizeChanged, this, [ = ](int fontSize) {
        ConfigSettings::instance()->setValue("text", "fontsize", fontSize);
    });
}

FontSizeWidget::~FontSizeWidget()
{
}
