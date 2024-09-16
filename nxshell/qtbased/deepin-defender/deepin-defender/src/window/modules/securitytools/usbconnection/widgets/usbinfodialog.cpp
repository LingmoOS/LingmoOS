// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbinfodialog.h"

#include <DTitlebar>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

UsbInfoDialog::UsbInfoDialog(QWidget *parent)
    : DFrame(parent)
    , m_blurEffect(nullptr)
    , m_mainWidgt(nullptr)
    , m_mainLayout(nullptr)
{
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_TranslucentBackground);

    m_blurEffect = new DBlurEffectWidget(this);
    m_blurEffect->setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    m_blurEffect->resize(size());
    m_blurEffect->lower();

    //close button
    QHBoxLayout *titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    DTitlebar *titleBar = new DTitlebar(this);
    titleBar->setBackgroundTransparent(true);
    titleBar->setTitle("");
    titleBar->setMenuVisible(false);
    titleBarLayout->addWidget(titleBar, 0, Qt::AlignRight | Qt::AlignTop);

    initUI();
}

void UsbInfoDialog::initUI()
{
    m_mainWidgt = new QWidget(this);
    m_mainWidgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainWidgt->move(0, 50);
    m_mainWidgt->resize(size().width(), size().height() - 50);
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setContentsMargins(10, 0, 10, 10);
    m_mainLayout->setSpacing(0);
    m_mainWidgt->setLayout(m_mainLayout);
}

void UsbInfoDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    m_mainWidgt->resize(size().width(), m_mainLayout->sizeHint().height());
    resize(size().width(), m_mainLayout->sizeHint().height() + 50);
    m_blurEffect->resize(size());
}

void UsbInfoDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->deleteLater();
}

void UsbInfoDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    m_mainWidgt->resize(size().width(), m_mainLayout->sizeHint().height());
    resize(size().width(), m_mainLayout->sizeHint().height() + 50);
    m_blurEffect->resize(size());
}
