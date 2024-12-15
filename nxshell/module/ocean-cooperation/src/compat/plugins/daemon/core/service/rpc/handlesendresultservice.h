// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HANDLESENDRESULTSERVICE_H
#define HANDLESENDRESULTSERVICE_H

#include <QObject>


class HandleSendResultService : public QObject
{
    Q_OBJECT
public:
    explicit HandleSendResultService(QObject *parent = nullptr);
    ~HandleSendResultService() = default;
public slots:
    void handleSendResultMsg(const QString appName, const QString msg);
private:
    void handleLogin(const QString &appName, const QString &msg);
};

#endif // HANDLESENDRESULTSERVICE_H
