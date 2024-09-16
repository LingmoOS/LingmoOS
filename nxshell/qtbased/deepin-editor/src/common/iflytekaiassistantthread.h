// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IFLYTEKAIASSISTANTTHREAD_H
#define IFLYTEKAIASSISTANTTHREAD_H

#include <QThread>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

class IflytekAiassistantThread : public QThread
{
    Q_OBJECT
public:
    IflytekAiassistantThread(QObject *object = nullptr);
    ~IflytekAiassistantThread();

private:
    void run();

signals:
    void sigIsRegisteredIflytekAiassistant(const bool bIsRegistIflytekAiassistant);
};

#endif // IFLYTEKAIASSISTANTTHREAD_H
