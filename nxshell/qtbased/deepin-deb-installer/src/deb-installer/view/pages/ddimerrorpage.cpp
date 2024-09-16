// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddimerrorpage.h"

#include <DLabel>
#include <DFontSizeManager>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>

DdimErrorPage::DdimErrorPage(QWidget *parent)
    : QWidget(parent)
    , errorMessageLabel(new Dtk::Widget::DLabel)
    , errorPicLabel(new Dtk::Widget::DLabel)
    , confimButton(new QPushButton(tr("OK")))
{
    setFocusPolicy(Qt::NoFocus);
    auto allLayout = new QVBoxLayout;
    allLayout->addWidget(errorPicLabel, 0, Qt::AlignCenter | Qt::AlignBottom);
    allLayout->addWidget(errorMessageLabel, 0, Qt::AlignCenter | Qt::AlignBottom);
    allLayout->addWidget(confimButton, 0, Qt::AlignCenter | Qt::AlignBottom);
    setLayout(allLayout);

    errorPicLabel->setScaledContents(true);  // 消除屏幕缩放锯齿
    errorPicLabel->setPixmap(QPixmap(":/icons/deepin/builtin/light/icons/di_fail_96px.png"));
    auto picContentsMargins = errorPicLabel->contentsMargins();
    picContentsMargins.setBottom(10);

    auto labelContentsMargins = errorMessageLabel->contentsMargins();
    labelContentsMargins.setBottom(50);
    errorMessageLabel->setContentsMargins(labelContentsMargins);
    Dtk::Widget::DFontSizeManager::instance()->bind(errorMessageLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    errorMessageLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);

    auto allContentsMargins = allLayout->contentsMargins();
    allContentsMargins.setBottom(20);
    allContentsMargins.setTop(55);
    allLayout->setContentsMargins(allContentsMargins);
    allLayout->setSpacing(0);

    confimButton->setFocusPolicy(Qt::StrongFocus);
    confimButton->setFixedSize(120, 36);
    confimButton->setDefault(true);
    connect(confimButton, &QPushButton::clicked, this, &DdimErrorPage::comfimPressed);
}

void DdimErrorPage::setErrorMessage(const QString &message)
{
    errorMessageLabel->setText(message);
}

void DdimErrorPage::showEvent(QShowEvent *event)
{
    // 每次展示时固定默认焦点，缩放后弹出也是
    confimButton->setFocus();
    QWidget::showEvent(event);
}
