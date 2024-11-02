/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */
#include "explor-json-operation.h"
#include "bw-list-info.h"
#include <QFile>
#include <QFileInfo>
#include <errno.h>

using namespace Peony;

PeonyJsonOperation::PeonyJsonOperation(QObject *parent):QObject(parent)
{
    m_configFile = nullptr;
}

PeonyJsonOperation::~PeonyJsonOperation()
{
    m_configFile = nullptr;
}

void PeonyJsonOperation::setConfigFile(QString configFile)
{
    //qDebug()<<"the configfile :" << configFile;
    m_configFile = configFile;
}

int PeonyJsonOperation::loadConfigFile(BWListInfo *bwListInfo)
{
    //1、judge file exsit?
    QFileInfo file(m_configFile);
    if (!file.exists()){
//        qWarning("config file %s do not exist", m_configFile);
        return -EEXIST;
    }

    //2、read json file
    QFile jsonFile(m_configFile);
    if (!jsonFile.open(QIODevice::ReadOnly)){
//        qWarning("open file %s failed, errno[%d]", m_configFile, errno);
        return -errno;
    }
    QByteArray jsonCtxt = jsonFile.readAll();
    jsonFile.close();

    if (jsonCtxt.isEmpty()){
        qWarning("get json context failed, or is empty");
        return -EINVAL;
    }

    //3、load json file
    QJsonParseError jsonerror;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonCtxt, &jsonerror);
    if (jsonDoc.isNull() || jsonerror.error != QJsonParseError::NoError){
//        qWarning("json load file %s failed, errno[%d]", m_configFile, jsonerror.error);
        return -EINVAL;
    }

    getBWListInfo(jsonDoc, bwListInfo);

    return 0;
}

void PeonyJsonOperation::releaseConfigFile()
{

}

int PeonyJsonOperation::getBWListInfo(QJsonDocument &jsonDoc, BWListInfo *bwListInfo)
{
    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("lingmo-explor")) {
        qWarning("the json file is not lingmo-explor");
        return -EINVAL;
    }

    QJsonObject explorObj = jsonObj.value("lingmo-explor").toObject();
    if (!explorObj.contains("mode")) {
        qWarning("the json file does not contain model");
        return -EINVAL;
    }

    QString model = explorObj.value("mode").toString();
    qDebug()<<"bw list mode:"<< model;
    if (BW_LIST_BLACK == model) {
        bwListInfo->setBWListModel(model);
        if (explorObj.contains(BW_LIST_BLACK)) {
            QJsonValue bPkgValue = explorObj.value(BW_LIST_BLACK);
            return bwPkgInfoParse(bPkgValue, bwListInfo);
        }
    } else if (BW_LIST_WHITE == model){
        bwListInfo->setBWListModel(model);
        if (explorObj.contains(BW_LIST_WHITE)) {
            QJsonValue wPkgValue = explorObj.value(BW_LIST_WHITE);
            return bwPkgInfoParse(wPkgValue, bwListInfo);
        }
    } else if (BW_LIST_NORMAL == model){
        bwListInfo->setBWListModel(model);
    } else {
//        qWarning("the mode %s is undefined", model);
        return -EINVAL;
    }

    return 0;
}

int PeonyJsonOperation::bwPkgInfoParse(QJsonValue &bwPkgValue, BWListInfo *bwListInfo)
{
    if (!bwPkgValue.isArray()) {
        qWarning("the pkg info format is not array");
        return -EINVAL;
    }

    QJsonArray pkgInfoArray = bwPkgValue.toArray();
    int pkgCnt = pkgInfoArray.size();
    for (int i = 0; i < pkgCnt; i++)
    {
        QJsonValue pkgElementValue = pkgInfoArray.at(i);
        if (!pkgElementValue.isObject()) {
            qWarning("the format of pkg element is not object");
            return -EINVAL;
        }

        QJsonObject pkgElementObj = pkgElementValue.toObject();
        if (!pkgElementObj.contains("entries")) {
            qWarning("the pkg info do not contain element entries");
            return -EINVAL;
        }

        QJsonValue entryInfoValue = pkgElementObj.value("entries");
        entryInfoParse(entryInfoValue, bwListInfo);
    }

    return 0;
}

int PeonyJsonOperation::entryInfoParse(QJsonValue &entryInfoValue, BWListInfo *bwListInfo)
{
    if (!entryInfoValue.isArray()) {
        qWarning("the app config is not array");
        return -EINVAL;
    }

    QJsonArray entryInfoArray = entryInfoValue.toArray();
    int entryCnt = entryInfoArray.size();
    for (int i = 0; i < entryCnt; i++) {
        QJsonValue appValue = entryInfoArray.at(i);
        if (!appValue.isObject()) {
            qWarning("the app config array element is not object");
            return -EINVAL;
        }

        QJsonObject appObj = appValue.toObject();
        if (!appObj.contains("path")) {
            qWarning("the config do not contain element path");
            return -EINVAL;
        }

        QJsonValue pathValue = appObj.value("path");
        if (!pathValue.isString()){
            qWarning("path element is not string");
            return -EINVAL;
        }

        QString appPath = pathValue.toString();
        bwListInfo->addBWListElement(appPath);
    }

    return 0;
}
