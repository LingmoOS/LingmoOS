// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <optional>
#include <QDebug>
#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QtQml>

class LauncherCall : public QObject
{
    Q_OBJECT
public:
    explicit LauncherCall() {}
    ~LauncherCall() override {}

    Q_INVOKABLE inline LauncherCall *program(const QString &program)
    {
        m_program = program;
        return this;
    }
    Q_INVOKABLE inline LauncherCall *arguments(const QStringList &arguments)
    {
        m_arguments = arguments;
        return this;
    }
    Q_INVOKABLE inline LauncherCall *timeout(const int timeout)
    {
        m_timeout = timeout;
        return this;
    }
    Q_INVOKABLE inline LauncherCall *workingDir(const QString &workingDir)
    {
        m_workingDir = workingDir;
        return this;
    }
    Q_INVOKABLE QVariantMap call();
    Q_INVOKABLE void asyncCall(const QJSValue &jsCallback);
    Q_INVOKABLE void startDetached();

private:
    QString m_program;
    QStringList m_arguments;
    QString m_workingDir;
    std::optional<int> m_timeout;
};
