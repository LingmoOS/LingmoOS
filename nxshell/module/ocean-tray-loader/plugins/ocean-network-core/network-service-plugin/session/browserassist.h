// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BROWSERASSIST_H
#define BROWSERASSIST_H

#include <QObject>

class QProcess;

namespace network {
namespace sessionservice {

class BrowserAssist : public QObject
{
    Q_OBJECT

public:
    static void openUrl(const QString &url);

protected:
    explicit BrowserAssist(QObject *parent = nullptr);
    ~BrowserAssist();

private:
    void init();
    bool desktopIsPrepare() const;
    void openUrlAddress(const QString &url);
    QString getDisplayEnvironment() const;

private slots:
    void onServiceRegistered(const QString &service);

private:
    QStringList m_cacheUrls;
    bool m_dockIsRegister;
    QProcess *m_process;
};

}
}

#endif // SERVICE_H
