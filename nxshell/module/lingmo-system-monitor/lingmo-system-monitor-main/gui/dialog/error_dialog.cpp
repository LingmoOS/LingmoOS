// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "error_dialog.h"

#include "gui/ui_common.h"

#include <DApplicationHelper>
#include <DFontSizeManager>

#include <QApplication>
#include <QMap>
#include <QDebug>

// Show error dialog
void ErrorDialog::show(QWidget *parent, const QString &errMsg, const QString &detail)
{
    auto *dlg = new ErrorDialog(errMsg, detail, parent);
    dlg->exec();
}

// Initialize UI elements
void ErrorDialog::initUI()
{
    // set dialog icon
    setIcon(QIcon::fromTheme("dialog-warning"));
    // set dialog attribute
    setAttribute(Qt::WA_DeleteOnClose);
    // set maximum width to avoid displaying extra wide dialog
    setMaximumWidth(720);

    m_detailLabel = new DLabel(this);
    // global palette of current theme
    auto pa = DApplicationHelper::instance()->palette(m_detailLabel);
    // get TextTips color from palette
    auto color = pa.color(DPalette::TextTips);
    pa.setColor(DPalette::Foreground, color);
    // set palette for error detail label
    m_detailLabel->setPalette(pa);
    // bind font size
    DFontSizeManager::instance()->bind(m_detailLabel, DFontSizeManager::T6, QFont::Medium);
    // word wrap on extra long description text
    m_detailLabel->setWordWrap(true);

    // set error message and error detail text
    setMessage(m_errMsg);
    m_detailLabel->setText(m_detail);
    m_detailLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    addSpacing(5);

    addContent(m_detailLabel, Qt::AlignHCenter);
    addButton(QApplication::translate("Error.Dialog", "OK", "button"), true);
}

// Private constructor
ErrorDialog::ErrorDialog(const QString &errMsg, const QString &detail, QWidget *parent)
    : DDialog(parent), m_errMsg(errMsg), m_detail(detail)
{
    initUI();
}
