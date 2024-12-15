// Copyright (C) 2024 Wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "accessdialog.h"
#include <QWindow>
#include <QMessageBox>
// X11的声明放在下面，防止编译报错
#include <X11/Xlib.h>

AccessDialog::AccessDialog(const QString &app_id, const QString &parent_window, const QString &title, const QString &subtitle, const QString &body, const QVariantMap &options) :
    DDialog(),
    m_titleLabel(new QLabel(this)),
    m_subtitleLabel(new QLabel(this)),
    m_bodyLabel(new QLabel(this))
{
    setAccessibleName("AccessDialog");
    setIcon(QIcon::fromTheme("dialog-warning"));
    setAttribute(Qt::WA_QuitOnClose);
    // 设置tittle
    m_titleLabel->setObjectName("TitileText");
    m_titleLabel->setAccessibleName("TitileText");
    addContent(m_titleLabel, Qt::AlignTop | Qt::AlignHCenter);
    QFont font = m_titleLabel->font();
    font.setBold(true);
    font.setPixelSize(16);
    m_titleLabel->setFont(font);
    m_titleLabel->setText(title);
    // 设置subtitle
    m_subtitleLabel->setObjectName("SubtitleText");
    m_subtitleLabel->setAccessibleName("SubtitleText");
    addContent(m_subtitleLabel, Qt::AlignTop | Qt::AlignHCenter);
    m_subtitleLabel->setText(subtitle+"\n");
    // 设置body
    m_bodyLabel->setObjectName("BodyText");
    m_bodyLabel->setAccessibleName("BodyText");
    addContent(m_bodyLabel, Qt::AlignTop | Qt::AlignHCenter);
    m_bodyLabel->setText(body);

    if (options.contains(QStringLiteral("modal"))) {
        setModal(options.value(QStringLiteral("modal")).toBool());
    }

    if (options.contains(QStringLiteral("deny_label"))) {
        addButton(options.value(QStringLiteral("deny_label")).toString(), QMessageBox::RejectRole);
    } else {
        addButton(tr("Deny Access"), QMessageBox::RejectRole);
    }


    int allowButton;
    if (options.contains(QStringLiteral("grant_label"))) {
        addButton(options.value(QStringLiteral("grant_label")).toString(), QMessageBox::AcceptRole);
    } else {
        addButton(tr("Grant Access"), QMessageBox::AcceptRole);
    }

    setWindowFlag(Qt::WindowStaysOnTopHint);
}

AccessDialog::~AccessDialog(){

}