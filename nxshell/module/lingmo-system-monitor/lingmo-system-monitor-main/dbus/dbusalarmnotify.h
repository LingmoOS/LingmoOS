// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSALARMNOTIFY_H
#define DBUSALARMNOTIFY_H

#include <QObject>
#include <QMutex>

class DBusAlarmNotify : public QObject
{
    Q_OBJECT
public:
    static DBusAlarmNotify &getInstance();
    /**
     * @brief showAlarmNotify 显示警告提示
     * @param allArguments 警告提示信息
     * @return
     */
    void showAlarmNotify(const QStringList &allArguments);

private:
    DBusAlarmNotify(QObject *parent = nullptr);
    ~DBusAlarmNotify() {}
    DBusAlarmNotify(const DBusAlarmNotify &) = delete;
    DBusAlarmNotify &operator=(const DBusAlarmNotify &) = delete;
    void showAlarmNotify(QString topic, QString msg, int timeout);

private:
    static QMutex mutex;
    static QAtomicPointer<DBusAlarmNotify> instance;
};

#endif // DBUS_OBJECT_H
