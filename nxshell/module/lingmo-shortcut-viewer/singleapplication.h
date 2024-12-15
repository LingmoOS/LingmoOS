// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include <DApplication>

DWIDGET_USE_NAMESPACE

class SingleApplication : public DApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication();
    void initConnect();

    void newClientProcess(const QString &key, const QByteArray &message);
    static QString userServerName(const QString &key);
    static QString userID();
    static QString UserID;

    static void processArgs(const QStringList &list);

public slots:
    bool setSingleInstance(const QString &key);
    void handleConnection();
    void readData();

private:
    QLocalServer *m_localServer;
};

#endif   // SINGLEAPPLICATION_H
