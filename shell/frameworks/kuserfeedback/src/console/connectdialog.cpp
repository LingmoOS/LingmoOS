/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "connectdialog.h"
#include "ui_connectdialog.h"

#include <rest/serverinfo.h>

#include <QDebug>
#include <QIcon>
#include <QPushButton>
#include <QUrl>

using namespace KUserFeedback::Console;

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Connect"));
    connect(ui->serverName, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectDialog::serverSelected);

    setWindowIcon(QIcon::fromTheme(QStringLiteral("network-connect")));
}

ConnectDialog::~ConnectDialog()
{
}

ServerInfo ConnectDialog::serverInfo() const
{
    ServerInfo info;
    info.setName(ui->serverName->currentText());
    info.setUrl(QUrl(ui->serverUrl->text()));
    info.setUserName(ui->userName->text());
    info.setPassword(ui->password->text());
    return info;
}

void ConnectDialog::setServerInfo(const ServerInfo& serverInfo)
{
    ui->serverName->setCurrentText(serverInfo.name());
    ui->serverUrl->setText(serverInfo.url().toString());
    ui->userName->setText(serverInfo.userName());
    ui->password->setText(serverInfo.password());
}

void ConnectDialog::addRecentServerInfos(const QStringList& serverInfoNames)
{
    ui->serverName->addItems(serverInfoNames);
}

void ConnectDialog::serverSelected()
{
    auto info = ServerInfo::load(ui->serverName->currentText());
    if (info.isValid())
        setServerInfo(info);
}

#include "moc_connectdialog.cpp"
