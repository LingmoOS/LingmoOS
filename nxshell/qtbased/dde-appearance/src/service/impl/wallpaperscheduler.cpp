// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperscheduler.h"
#include "../modules/api/utils.h"
#include "../modules/common/commondefine.h"

#include <QDebug>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

WallpaperScheduler::WallpaperScheduler(BGCHANGEFUNC func)
    :stopScheduler(false)
    , lastSetBgTime(QDateTime::currentDateTimeUtc())
{
    this->bgChangeFunc =func;
    connect(&changeTimer, &QTimer::timeout, this, &WallpaperScheduler::handleChangeTimeOut);
}

void WallpaperScheduler::setInterval(QString monitorSpace, qint64 interval)
{
    if (interval <= 0) {
        stop();
        return;
    }

    this->monitorSpace = monitorSpace;
    this->interval = interval * 1000;
    stopScheduler = false;
    QDateTime curr = QDateTime::currentDateTimeUtc();

    qint64 elapsed = lastSetBgTime.secsTo(curr);
    if (elapsed < this->interval) {
        changeTimer.start(static_cast<int>(this->interval - elapsed));
    } else {
        handleChangeTimeOut();
    }
}

void WallpaperScheduler::setLastChangeTime(QDateTime date)
{
    lastSetBgTime = date;
}

void WallpaperScheduler::stop()
{
    stopScheduler = true;
    changeTimer.stop();
}

void WallpaperScheduler::handleChangeTimeOut()
{
    QDateTime curr = QDateTime::currentDateTimeUtc();
    if(bgChangeFunc)
    {
        bgChangeFunc(this->monitorSpace,curr);
    }

    if(!stopScheduler)
    {
        changeTimer.start(static_cast<int>(interval));
    }
}


WallpaperLoop::WallpaperLoop()
    :fileChange(true)
    ,rander(QRandomGenerator::global())
{

}

QStringList WallpaperLoop::getShowed()
{
    return showedList;
}

QStringList WallpaperLoop::getNotShowed()
{
    QStringList retList;
    if(fileChange)
    {
       QVector<Background> bgs = backgrounds.listBackground();
       for(auto iter : bgs)
       {
           allList.push_back(utils::deCodeURI(iter.getId()));
       }
    }

    for(auto iter : allList)
    {
        if(!showedList.contains(iter))
        {
            retList.push_back(iter);
        }
    }
    return retList;
}

QString WallpaperLoop::getNext()
{
    QString next = getNextShow();
    if(!next.isEmpty())
    {
        return next;
    }

    if(!allList.isEmpty())
    {
        reset();
        next = getNext();
    }

    return next;
}

QString WallpaperLoop::getNextShow()
{
    QStringList notShowList = getNotShowed();
    if(notShowList.empty())
    {
        return "";
    }

    int index = rander->bounded(notShowList.size());
    QString nextWallpaper = notShowList[index];

    showedList.push_back(notShowList[index]);

    return nextWallpaper;
}

void WallpaperLoop::reset()
{
    showedList.clear();
}

void WallpaperLoop::addToShow(QString file)
{
    file = utils::deCodeURI(file);

    showedList.push_back(file);
}

void WallpaperLoop::notifyFileChange()
{
    fileChange = true;
}

WallpaperLoopConfigManger::WallpaperLoopConfigManger()
{

}

WallpaperLoopConfigManger::WallpaperLoopConfigMap WallpaperLoopConfigManger::loadWSConfig(QString fileName)
{
    wallpaperLoopConfigMap.clear();

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<fileName<<" open fail";
        return wallpaperLoopConfigMap;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(),&err);
    if(err.error != QJsonParseError::NoError)
    {
        qDebug()<<fileName<<" parse fail";
        return wallpaperLoopConfigMap;
    }

    QJsonObject obj = doc.object();

    for(auto key :obj.keys())
    {
        WallpaperLoopConfig config;
        wallpaperLoopConfigMap[key]=config;

        QJsonObject wlConfigObj = obj[key].toObject();
        for (auto it = wlConfigObj.begin(); it != wlConfigObj.end(); it++) {
            if (it.key() == "LastChange") {
                wallpaperLoopConfigMap[key].lastChange = QDateTime::fromString(it.value().toString(), "yyyy-MM-dd hh:mm:ss");
            } else if (it.key() == "Showed") {
                QJsonArray arr = it.value().toArray();
                for (auto iter : arr) {
                    wallpaperLoopConfigMap[key].showedList.push_back(iter.toString());
                }
            }
        }
    }

    return wallpaperLoopConfigMap;
}

void WallpaperLoopConfigManger::setShowed(QString monitorSpace, QStringList showedList)
{
    if(wallpaperLoopConfigMap.count(monitorSpace) == 0)
    {
        wallpaperLoopConfigMap[monitorSpace] = WallpaperLoopConfig();
    }
    wallpaperLoopConfigMap[monitorSpace].showedList = showedList;
}

void WallpaperLoopConfigManger::setLastChange(QString monitorSpace, QDateTime date)
{
    if(wallpaperLoopConfigMap.count(monitorSpace) == 0)
    {
        wallpaperLoopConfigMap[monitorSpace] = WallpaperLoopConfig();
    }
    wallpaperLoopConfigMap[monitorSpace].lastChange = date;
}

bool WallpaperLoopConfigManger::save(QString fileName)
{
    QJsonDocument doc;
    QJsonObject obj;
    for(auto wallPaperConfig : wallpaperLoopConfigMap.toStdMap())
    {
        QJsonObject config;
        config["LastChange"] = wallPaperConfig.second.lastChange.toString("yyyy-MM-dd hh:mm:ss");

        QJsonArray showedArr;
        for(auto showedFile : wallPaperConfig.second.showedList)
        {
            showedArr.push_back(showedFile);
        }
        config["Showed"] = showedArr;

        obj[wallPaperConfig.first] = config;
    }

    doc.setObject(obj);
    QByteArray text = doc.toJson(QJsonDocument::Compact);

    QDir dir(fileName.left(fileName.lastIndexOf("/")));
    if(!dir.exists())
    {
         if(!dir.mkpath(dir.path()))
         {
             qDebug()<< "mkpath"<<dir.path()<<"fail";
             return false;
         }
    }

    QFile file(fileName);

    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        qDebug()<<fileName<<" open fail";
        return false;
    }

    file.write(text);
    file.close();

    return true;
}

bool WallpaperLoopConfigManger::isValidWSPolicy(QString policy)
{
    if(policy == WSPOLICYLOGIN || policy == WSPOLICYWAKEUP || policy.isEmpty())
    {
        return true;
    }
    bool ok;
    policy.toUInt(&ok);

    return ok;
}
