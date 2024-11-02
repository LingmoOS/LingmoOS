/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "global-fstabdata.h"
#include <QApplication>

#include <QFile>
#include <QDebug>

#define PATH "/etc/fstab"

using namespace Peony;

static GlobalFstabData *global_instance = nullptr;

GlobalFstabData *GlobalFstabData::getInstance()
{
    if(!global_instance){
        global_instance = new GlobalFstabData;
    }
    return global_instance;
}

GlobalFstabData::GlobalFstabData(QObject *parent)
{
    m_fstabDataLists = this->readFstabFile();
    m_fstab_data_watcher = new QFileSystemWatcher(this);
    m_fstab_data_watcher->addPath(PATH);
    connect(m_fstab_data_watcher, &QFileSystemWatcher::fileChanged, [=](const QString& uri){
        m_fstabDataLists.clear();
        m_fstabDataLists = this->readFstabFile();
        m_fstab_data_watcher->addPath(uri);
    });
}

bool GlobalFstabData::isMountPoints(const QString &name, const QString &mountPoint)
{
    if(isEmptyList()){
        return false;
    }

    QList<fstabData*>::iterator iter = m_fstabDataLists.begin();
    while(iter != m_fstabDataLists.end()){
        //qDebug() << "======data=====" << (*iter)->getName() << (*iter)->getMountPoint();
        if((*iter)->getName() == name && (*iter)->getMountPoint() == mountPoint){
            return true;
        }
        iter++;
    }
    return false;
}

bool GlobalFstabData::getUuidState()
{
    return m_isUuid;
}

bool GlobalFstabData::isEmptyList()
{
    return m_fstabDataLists.isEmpty();
}

const QList<fstabData *> GlobalFstabData::readFstabFile()
{
    bool isUuid = false;
    bool isUnixDevice = false;
    QList<fstabData *> tmpListData;
    QFile file(PATH);

    for(int i = 0; i < 2; i++){
        if(!file.open(QIODevice::ReadOnly)){
            //qDebug() << "Can't open the file";
            return tmpListData;
        }

        while(!file.atEnd()){
            QByteArray line = file.readLine();
            QString str(line);
            QStringList tmpLists;

            if(!m_isUuid){
                if(str.startsWith("/dev")){
                    isUnixDevice = true;
                }
            }else{
                if(str.startsWith("UUID=")){
                    str = str.remove("UUID=");
                    isUuid = true;
                }
            }

            if(isUuid || isUnixDevice){
                fstabData *tmpData = new fstabData();
                tmpLists = str.split("\t");

                QList<QString>::iterator it = tmpLists.begin();
                QList<QString>::iterator itEnd = tmpLists.end();
                for(int i = 0 ; it != itEnd; ++it, ++i){
                    if(i == 0){
                        tmpData->setName((*it).trimmed());
                    }else if(i == 1){
                        tmpData->setMountPoint((*it).trimmed());
                    }else{
                        break;
                    }
                }
                isUuid = false;
                isUnixDevice = false;
                tmpListData.append(tmpData);
                tmpLists.clear();
            }
        }
        file.close();

        if(tmpListData.isEmpty()){
            m_isUuid = true;
        }
    }
    return tmpListData;
}


fstabData::fstabData(QObject *parent)
{
    m_name = "";
    m_mountPoint = "";
}

const QString fstabData::getName()
{
   return m_name;
}

const QString fstabData::getMountPoint()
{
    return m_mountPoint;
}

void fstabData::setName(const QString &name)
{
    if(!name.isEmpty()){
        m_name = name;
    }else{
        m_name = "";
    }
}

void fstabData::setMountPoint(const QString &mountPoint)
{
    if(!mountPoint.isEmpty()){
        m_mountPoint = mountPoint;
    }else{
        m_mountPoint = "";
    }
}
