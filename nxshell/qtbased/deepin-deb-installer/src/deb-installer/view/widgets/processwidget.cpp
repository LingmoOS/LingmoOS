// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DProgressBar>
#include <DLabel>
#include <DFontSizeManager>

ProcessWidget::ProcessWidget(QWidget *parent)
    : QWidget(parent)
{
    mainIcon = new Dtk::Widget::DLabel;

    mainLabel = new Dtk::Widget::DLabel;
    Dtk::Widget::DFontSizeManager::instance()->bind(mainLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    mainLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);

    processBar = new Dtk::Widget::DProgressBar;
    processBar->setFixedSize(300, 8);

    processTextLabel = new Dtk::Widget::DLabel(this);
    Dtk::Widget::DFontSizeManager::instance()->bind(processTextLabel, Dtk::Widget::DFontSizeManager::T8, QFont::Medium);
    processTextLabel->setForegroundRole(Dtk::Gui::DPalette::TextTips);

    auto allLayer = new QVBoxLayout;
    allLayer->setContentsMargins(11, 63, 11, 11);
    allLayer->setSpacing(15);
    allLayer->addWidget(mainIcon, 0, Qt::AlignCenter);
    allLayer->addWidget(mainLabel, 0, Qt::AlignCenter);
    allLayer->addWidget(processBar, 0, Qt::AlignCenter);
    // allLayer->addWidget(processTextLabel);
    allLayer->addStretch();

    setLayout(allLayer);
}

void ProcessWidget::setIcon(const QIcon &icon)
{
    mainIcon->setPixmap(icon.pixmap(64, 64));
}

void ProcessWidget::setMainText(const QString &text)
{
    mainLabel->setText(text);
}

void ProcessWidget::setProcessText(const QString &text)
{
    processText = text;
}

void ProcessWidget::setProgress(int current, int all)
{
    auto currentText = processText.arg(current).arg(all);
    processTextLabel->setText(currentText);
    processTextLabel->adjustSize();
    processTextLabel->move((this->width() - processTextLabel->width()) / 2, processBar->y() + 30);
    processBar->setRange(0, all);
    processBar->setValue(current);
}
