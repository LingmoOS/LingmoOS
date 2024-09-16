// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERAPPFLOW_H
#define DEFENDERAPPFLOW_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>
#include <QtCore/QList>

struct DefenderAppFlow {
    int id = 0;
    QString name;
    QString sname;
    QString timeymdh;
    double downloaded = 0;
    double uploaded = 0;
    double apptotal = 0;//单个APP总流量

    bool operator!=(const DefenderAppFlow &appFlow);
};

Q_DECLARE_METATYPE(DefenderAppFlow);
//DefenderAppFlowList注册
typedef QList<DefenderAppFlow> DefenderAppFlowList;
Q_DECLARE_METATYPE(DefenderAppFlowList);

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderAppFlow &appFlow);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderAppFlow &appFlow);

void registerDefenderAppFlowMetaType();

#endif

