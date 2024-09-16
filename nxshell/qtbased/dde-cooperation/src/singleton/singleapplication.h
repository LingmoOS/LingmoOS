// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#if defined(_WIN32) || defined(_WIN64)
#    include <QApplication>
typedef QApplication CrossApplication;
#else
#    include <DApplication>
typedef DTK_WIDGET_NAMESPACE::DApplication CrossApplication;
#endif

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
QT_END_NAMESPACE

namespace deepin_cross {

#if defined(qApp)
#    undef qApp
#endif
#define qApp (static_cast<SingleApplication *>(CrossApplication::instance()))

class SingleApplication : public CrossApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication() override;

signals:
    void onArrivedCommands(const QStringList &args);

public slots:
    bool checkProcess(const QString &key);
    bool setSingleInstance(const QString &key);
    void handleConnection();
    void readData();
    void closeServer();
    void helpActionTriggered();
    void onDeliverMessage(const QString &app, const QStringList &msg);

protected:
    void initConnect();

private:
    static bool sendMessage(const QString &key, const QByteArray &message);
    static QString userServerName(const QString &key);

private:
    QLocalServer *localServer { nullptr };
};
}

#endif   // SINGLEAPPLICATION_H
