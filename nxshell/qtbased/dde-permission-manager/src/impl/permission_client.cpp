// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "permission_client.h"
#include "permissionoption.h"

#include <QSharedPointer>
#include <QTimer>

#include "permissionpolicy.h"
#include "widgets/clientdialog.h"

DWIDGET_USE_NAMESPACE

PermissionClient::PermissionClient(QObject *parent) : QObject(parent) {}

PermissionClient::~PermissionClient() {}

QString PermissionClient::Request(const QString &title, const QString &description, const QString &prefer, const QStringList &options)
{
    QSharedPointer<ClientDialog> dialog(new ClientDialog);
    bool isSetDefaultButtom = false;
    for (auto const &option : options) {
        if (option == prefer) {
            isSetDefaultButtom = true;
            dialog->addButton(PermissionOption::getInstance()->getTs(option), true, DDialog::ButtonNormal);
        } else {
            dialog->addButton(PermissionOption::getInstance()->getTs(option), false, DDialog::ButtonNormal);
        }
    }
    if (!isSetDefaultButtom) {
        dialog->startBottomCloseTimer();
    }
    dialog->setTitle(title);
    dialog->setMessage(description);

    return options.value(dialog->exec(), "");
}

QString PermissionClient::ShowDisablePermissionDialog(const QString &title, const QString &description, const QStringList &options)
{
    QSharedPointer<ClientDialog> dialog(new ClientDialog);
    for (auto const &option : options) {
        dialog->addButton(option, false, DDialog::ButtonNormal);
    }

    dialog->setTitle(title);
    dialog->setMessage(description);

    return options.value(dialog->exec(), "");
}
