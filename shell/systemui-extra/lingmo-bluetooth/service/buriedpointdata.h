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

#ifndef BURIEDPOINTDATA_H
#define BURIEDPOINTDATA_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <lingmo-log4qt.h>

#include "CSingleton.h"

#ifdef EXISTS_LINGMOSDK_DIAGNOSTICS
//埋点
#include <lingmosdk/lingmosdk-base/libkydiagnostics.h>
#endif

class BuriedPointData : public QObject
{
    Q_OBJECT
public:
    BuriedPointData(QObject *parent = nullptr);

Q_SIGNALS:
    void writeInData(QString, QString, QString, QString);

public slots:
    void writeInDataSlot(QString , QString , QString , QString);

protected:
    bool buriedSettings(QString, QString, QString, QString);

    QThread * m_Thread = nullptr;

    friend class SingleTon<BuriedPointData>;
};
typedef SingleTon<BuriedPointData>  BPDMNG;

#endif // BURIEDPOINTDATA_H
