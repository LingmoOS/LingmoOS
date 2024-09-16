// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "spinnerwidget.h"
#include "../core/utils.h"

#include <DSpinner>
#include <DFontSizeManager>

#include <QLabel>
#include <QLayout>

DWIDGET_USE_NAMESPACE

SpinnerWidget::SpinnerWidget(QWidget *parent)
    : QWidget(parent)
    , m_messageLabel(new SimpleLabel)
    , m_spinner(new DSpinner(this))
{
    initUI();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [this](DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::DarkType)
            m_messageLabel->setColor(QColor("#FFFFFF"));
        else
            m_messageLabel->setColor(QColor("#414D68"));
    });
}

void SpinnerWidget::initUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    m_spinner->setMinimumSize(32, 32);
    DFontSizeManager::instance()->bind(m_messageLabel, DFontSizeManager::T5, QFont::Medium);
    if (isDarkMode())
        m_messageLabel->setColor(QColor("#FFFFFF"));
    else
        m_messageLabel->setColor(QColor("#414D68"));

    vLayout->addStretch(0);
    vLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    vLayout->addSpacing(10);
    vLayout->addWidget(m_messageLabel, 0, Qt::AlignCenter);
    vLayout->addStretch(0);
    setLayout(vLayout);
}

void SpinnerWidget::setMinimumSize(int width, int height) {
    m_spinner->setMinimumSize(width, height);
}
void SpinnerWidget::setLabelSize(int size) {
    DFontSizeManager::instance()->bind(m_messageLabel, static_cast<DFontSizeManager::SizeType>(size));
}

void SpinnerWidget::setMessage(const QString &msg)
{
    m_messageLabel->setText(msg);
}

void SpinnerWidget::start()
{
    m_spinner->start();
}

void SpinnerWidget::stop()
{
    m_spinner->stop();
}
