/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef APP_H
#define APP_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QDebug>
#include <QDBusServiceWatcher>
#include "lingmo-log4qt.h"
#include "CSingleton.h"

class Application
{
public:
    explicit Application(QString, QString, int);
    ~Application();

    QString dbusid(){ return m_dbusid; }
    QString username(){ return m_username; }
    int type(){ return m_type; }

protected:
    QString m_dbusid;
    QString m_username;
    int m_type = 0;
};


class ApplicationMng: public QObject
{
public:
    ApplicationMng();
    ~ApplicationMng();

public:
    int add(QString, QString, int);

    int remove(QString);

    int getControlNum(void);

    int getAllNum(void) { return m_apps.size(); }

    bool existDbusid(QString);

    QStringList getRegisterClient(void);
    //int unknownDbusid(QString, );

    QString getUserDbusid(QString, int);
private:
    void update(void);

protected slots:
    void serviceUnregistered(const QString &service);


protected:
    QMap<QString, Application *> m_apps;
    QDBusServiceWatcher * m_watch = nullptr;

    friend class SingleTon<ApplicationMng>;
};
typedef SingleTon<ApplicationMng>  APPMNG;

#endif // APP_H
