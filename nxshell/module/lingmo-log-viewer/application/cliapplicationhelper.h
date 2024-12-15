// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIAPPLICATION_H
#define CLIAPPLICATION_H

#include <QObject>
#include <QCoreApplication>

// cli程序单实例类
class CliApplicationHelper : public QObject
{
    Q_OBJECT
public:
    enum SingleScope {
        UserScope,
        GroupScope,
        WorldScope
    };

    typedef CliApplicationHelper *(*HelperCreator)();

    static void staticCleanApplication();
    static CliApplicationHelper *instance();

    explicit CliApplicationHelper();
    ~CliApplicationHelper();

    static bool setSingleInstance(const QString &key, SingleScope singleScope = UserScope);

    // 获取QLocalSever消息的等待时间
    static int waitTime;

Q_SIGNALS:
     void newProcessInstance(qint64 pid, const QStringList &arguments);
};

#endif // CLIAPPLICATION_H
