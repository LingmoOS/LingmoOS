// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERDISPLAY_H
#define DEFENDERDISPLAY_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>
#include <QtCore/QList>

struct DefenderDisplay {
    QString name;
    QString icon;
    QString path;
    QString download;
    QString update;
    QString total;
    QString pidname;//desktop
    double totalupdate;
    double totaldownload;
    double totalflow;
    int pid;
    bool status;
    bool operator!=(const DefenderDisplay &display);
};
Q_DECLARE_METATYPE(DefenderDisplay);

typedef QList<DefenderDisplay> DefenderDisplayList;
Q_DECLARE_METATYPE(DefenderDisplayList);

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderDisplay &defenderdisplay);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderDisplay &defenderdisplay);

void registerDefenderDisplayMetaType();
#endif
