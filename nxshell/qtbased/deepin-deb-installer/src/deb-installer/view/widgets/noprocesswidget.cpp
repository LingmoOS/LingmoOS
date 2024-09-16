// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "noprocesswidget.h"
#include "utils/utils.h"

#include <DSpinner>
#include <DLabel>
#include <DFontSizeManager>
#include <DPalette>

#include <QVBoxLayout>

NoProcessWidget::NoProcessWidget(QWidget *parent)
    : QWidget(parent)
{
    spinner = new Dtk::Widget::DSpinner;
    spinner->setFixedSize(24, 24);

    actionTextLabel = new Dtk::Widget::DLabel;
    Dtk::Widget::DFontSizeManager::instance()->bind(actionTextLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);

    Dtk::Gui::DPalette pe;
    pe.setColor(QPalette::WindowText, Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    actionTextLabel->setPalette(pe);

    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(spinner, 0, Qt::AlignHCenter | Qt::AlignBottom);
    allLayer->addWidget(actionTextLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    setLayout(allLayer);
}

void NoProcessWidget::start()
{
    spinner->start();
}

void NoProcessWidget::stop()
{
    spinner->stop();
}

bool NoProcessWidget::event(QEvent *e)
{
    if (QEvent::PaletteChange == e->type()) {
        // 调色板更新时调整高亮颜色
        Dtk::Gui::DPalette pe;
        pe.setColor(QPalette::WindowText, Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        actionTextLabel->setPalette(pe);
    }

    return QWidget::event(e);
}

void NoProcessWidget::setActionText(const QString &text)
{
    actionTextLabel->setText(text);
}
