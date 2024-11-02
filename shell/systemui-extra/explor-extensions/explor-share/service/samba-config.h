/*
 * Copyright (C) 2022, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#ifndef SAMBACONFIG_H
#define SAMBACONFIG_H

#include <QObject>

class SambaConfigPrivate;

class SambaConfig : public QObject
{
    Q_OBJECT
    Q_CLASSINFO ("D-Bus Interface", DBUS_NAME)
public:
    const static SambaConfig* getInstance ();

private:
    SambaConfig(SambaConfig&):QObject(nullptr){};
    ~SambaConfig(){};
    explicit SambaConfig(QObject *parent = nullptr);

    bool launchSmbd();
    bool isSmbdLaunched ();
    bool launchNmbd();
    bool isNmbdLaunched();
    bool userIsInSambaGroup ();
    bool addUserInGroup ();
    bool isContainLegitimacyChar(QString passwd);

public Q_SLOTS:
    bool init (QString name, int pid, int uid);
    void finished ();

    bool hasPasswd ();
    bool setPasswd (QString passwd);

Q_SIGNALS:

private:
    SambaConfigPrivate*         d_ptr;
    Q_DECLARE_PRIVATE(SambaConfig)
};

#endif // SAMBACONFIG_H
