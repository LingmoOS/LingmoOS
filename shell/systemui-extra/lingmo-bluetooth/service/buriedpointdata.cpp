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
#include "buriedpointdata.h"


BuriedPointData::BuriedPointData(QObject *parent)
    :QObject(parent)
{
    KyInfo();
    m_Thread = new QThread(this);
    moveToThread(m_Thread);
    connect(this,&BuriedPointData::writeInData,this,&BuriedPointData::writeInDataSlot);
    m_Thread->start();
}

bool BuriedPointData::buriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
    KyInfo();
    //QThread::sleep(5);
#ifdef EXISTS_LINGMOSDK_DIAGNOSTICS
    // 埋点数据
    char appName[] = "lingmo-bluetooth";
    QString message;
    if (settingsName != nullptr) {
        message = pluginName + "/" + settingsName;
    } else {
        message = pluginName;
    }
    QByteArray ba = message.toLatin1();
    char *messageType = ba.data();
    KBuriedPoint pt[2];
    pt[0].key = "action";
    std::string str = action.toStdString();
    pt[0].value = str.c_str();

    pt[1].key = "value";
    std::string valueStr = value.toStdString();
    pt[1].value = valueStr.c_str();

    //异常捕获，kdk_buried_point异常通过C++标准库中抛出，使用Qt的QException无法捕获
    try
    {
        if (0 != kdk_buried_point(appName , messageType , pt , 2))
        {
            return false;
        }
    }
    catch(std::exception &e)
    {
        KyWarning() << e.what();
        return false;
    }
#endif
    return true;
}

void BuriedPointData::writeInDataSlot(QString pluginName, QString settingsName, QString action, QString value)
{
    KyDebug() << settingsName << action;

    if(buriedSettings(pluginName,settingsName,action,value))
        KyDebug() << "write data successful";
    else
        KyDebug() << "write data fail" ;

}
