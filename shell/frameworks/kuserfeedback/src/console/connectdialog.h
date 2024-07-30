/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_CONNECTDIALOG_H
#define KUSERFEEDBACK_CONSOLE_CONNECTDIALOG_H

#include <QDialog>

#include <memory>

namespace KUserFeedback {
namespace Console {

namespace Ui
{
class ConnectDialog;
}

class ServerInfo;

class ConnectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog() override;

    ServerInfo serverInfo() const;
    void setServerInfo(const ServerInfo &serverInfo);

    void addRecentServerInfos(const QStringList &serverInfoNames);

private:
    void serverSelected();

    std::unique_ptr<Ui::ConnectDialog> ui;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_CONNECTDIALOG_H
