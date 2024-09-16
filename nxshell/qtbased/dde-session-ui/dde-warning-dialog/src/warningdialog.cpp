// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "warningdialog.h"
#include <QApplication>
#include <QIcon>

WarningDialog::WarningDialog(QWidget *parent)
    : DDialog(parent)
{
    setAccessibleName("WarningDialog");
    const auto ratio = devicePixelRatioF();
    QPixmap icon = QIcon::fromTheme("dialog-warning").pixmap(int(48 * ratio), int(48 * ratio));
    icon.setDevicePixelRatio(ratio);

    const QString title = tr("Kindly Reminder");
    const QString body = tr("This application cannot run without window effect");
    setTitle(title);
    setMessage(body);
    setIcon(icon);
    addButton(tr("OK"), true);
}

WarningDialog::~WarningDialog()
{
}
