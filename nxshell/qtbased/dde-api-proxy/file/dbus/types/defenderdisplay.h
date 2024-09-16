// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     luokai <luokai_cm@deepin.com>
 *
 * Maintainer: luokai <luokai_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
