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

#ifndef PEONYJSONOPERATION_H
#define PEONYJSONOPERATION_H

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QDebug>

namespace Peony {

class BWListInfo;

class PeonyJsonOperation:public QObject
{
    Q_OBJECT

public:
    explicit PeonyJsonOperation(QObject *parent=nullptr);
    ~PeonyJsonOperation();

public:
    void setConfigFile(QString configFile);
    int  loadConfigFile(BWListInfo *bwListInfo);
    void releaseConfigFile();

private:
    int getBWListInfo(QJsonDocument &jsonDoc, BWListInfo *bwListInfo);
    int bwPkgInfoParse(QJsonValue &bwPkgValue, BWListInfo *bwListInfo);
    int entryInfoParse(QJsonValue &entryInfoValue, BWListInfo *bwListInfo);
    QString   m_configFile;
};

}

#endif // PEONYJSONOPERATION_H
