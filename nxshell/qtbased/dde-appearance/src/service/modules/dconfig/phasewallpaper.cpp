// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "phasewallpaper.h"

#include <modules/api/utils.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

const QString appearanceProcessId = "org.deepin.dde.appearance";
const QString appearanceDconfJson = "org.deepin.dde.appearance";
const QString allWallpaperUrisKey = "All_Wallpaper_Uris";

PhaseWallPaper::PhaseWallPaper() { }

PhaseWallPaper::~PhaseWallPaper() { }

QString phaseWPType(const QString &index, const QString &strMonitorName)
{
    QString shouldGetWPType = "";
    if (index != "" && strMonitorName != "") {
        shouldGetWPType = "index+monitorName";
    } else if (index != "" && strMonitorName == "") {
        shouldGetWPType = "onlyIndex";
    } else {
        return "";
    }

    return shouldGetWPType;
}

std::optional<QJsonArray> PhaseWallPaper::setWallpaperUri(const QString &index, const QString &strMonitorName, const QString &uri)
{
    QString wpIndexKey = generateWpIndexKey(index, strMonitorName);
    QString shouldGetWPType = phaseWPType(index, strMonitorName);
    if (shouldGetWPType == "") {
        qWarning() << QString("set wall paper type error, index [%1] strMonitorName [%2]").arg(index, strMonitorName);
        return std::nullopt;
    }

    QVariant v = DconfigSettings::ConfigValue(appearanceProcessId, appearanceDconfJson, allWallpaperUrisKey, "");
    if (!v.isValid()) {
        return std::nullopt;
    }

    QString url;
    if (!uri.isEmpty())
        url = utils::enCodeURI(uri, "file://");

    bool shouldAddWPTypeInfo = true;
    QJsonArray allWallpaperUri = v.toJsonArray();

    for (QJsonArray::Iterator it1 = allWallpaperUri.begin(); it1 != allWallpaperUri.end(); ++it1) {
        QJsonObject wpTypeObj = it1[0].toObject();
        if (!wpTypeObj.contains("type")) {
            continue;
        }

        QString wpType = wpTypeObj["type"].toString();
        if (wpType != shouldGetWPType) {
            continue;
        }

        shouldAddWPTypeInfo = false;
        if (!wpTypeObj.contains("wallpaperInfo")) {
            continue;
        }

        bool shouldAddWPInfo = !url.isEmpty();
        QJsonArray wpInfoArray = wpTypeObj["wallpaperInfo"].toArray();
        for (QJsonArray::Iterator it2 = wpInfoArray.begin(); it2 != wpInfoArray.end();) {
            QJsonObject wpInfoObj = it2[0].toObject();
            if (!wpInfoObj.contains("uri") || !wpInfoObj.contains("wpIndex")) {
                ++it2;
                continue;
            }

            if (wpInfoObj["wpIndex"] == wpIndexKey) {
                wpInfoObj["uri"] = url;
                shouldAddWPInfo = false;
                shouldAddWPTypeInfo = false;
                if (url.isEmpty()) {
                    it2 = wpInfoArray.erase(it2);
                    continue;
                }
            } else {
                ++it2;
                continue;
            }

            it2[0] = wpInfoObj;
            shouldAddWPTypeInfo = false;
            ++it2;
        }

        wpTypeObj["wallpaperInfo"] = wpInfoArray;
        if (shouldAddWPInfo) {
            QJsonObject obj = { { "uri", url }, { "wpIndex", wpIndexKey } };
            QJsonArray array = wpTypeObj["wallpaperInfo"].toArray();
            array.append(obj);
            wpTypeObj["wallpaperInfo"] = array;
        }
        it1[0] = wpTypeObj;
    }

    if (shouldAddWPTypeInfo) {
        QJsonObject obj1 = { { "uri", url }, { "wpIndex", wpIndexKey } };
        QJsonArray array = { obj1 };
        QJsonObject obj2 = { { "type", shouldGetWPType }, { "wallpaperInfo", array } };

        allWallpaperUri.append(obj2);
    }

    utils::writeWallpaperConfig(allWallpaperUri.toVariantList());
    DconfigSettings::ConfigSaveValue(appearanceProcessId, appearanceDconfJson, allWallpaperUrisKey, allWallpaperUri.toVariantList());
    return allWallpaperUri;
}

QString PhaseWallPaper::getWallpaperUri(const QString &index, const QString &strMonitorName)
{
    QString wpIndexKey = generateWpIndexKey(index, strMonitorName);
    QString shouldGetWPType = phaseWPType(index, strMonitorName);
    if (shouldGetWPType == "") {
        qWarning() << QString("set wall paper type error, index [%1] strMonitorName [%2]").arg(index, strMonitorName);
        return QString();
    }

    QVariant v = DconfigSettings::ConfigValue(appearanceProcessId, appearanceDconfJson, allWallpaperUrisKey, "");
    if (!v.isValid()) {
        return QString();
    }

    QJsonArray allWallpaperUri = v.toJsonArray();
    for (auto wallpaper : allWallpaperUri) {
        QJsonObject wpObj = wallpaper.toObject();
        if (!wpObj.contains("type")) {
            continue;
        }

        QString wpType = wpObj["type"].toString();
        if (wpType != shouldGetWPType) {
            continue;
        }

        if (!wpObj.contains("wallpaperInfo")) {
            continue;
        }

        QJsonArray wpInfoArray = wpObj["wallpaperInfo"].toArray();
        for (auto wpInfo : wpInfoArray) {
            QJsonObject obj = wpInfo.toObject();
            if (!obj.contains("uri") || !obj.contains("wpIndex")) {
                continue;
            }

            if (obj["wpIndex"] == wpIndexKey) {
                auto wallpaper = obj["uri"].toString();
                auto wallpaperPath = QUrl(wallpaper).toLocalFile();
                QFile file(wallpaperPath);
                return file.exists() ? wallpaper : QString();
            }
        }
    }

    return QString();
}

// 删除多余工作区的配置，保证新建的工作区壁纸随机
void PhaseWallPaper::resizeWorkspaceCount(int size)
{
    QVariant v = DconfigSettings::ConfigValue(appearanceProcessId, appearanceDconfJson, allWallpaperUrisKey, "");
    if (!v.isValid()) {
        return;
    }

    QJsonArray allWallpaperUri = v.toJsonArray();

    bool bSave = false;
    for (QJsonArray::Iterator it1 = allWallpaperUri.begin(); it1 != allWallpaperUri.end(); ++it1) {
        QJsonObject wpTypeObj = it1[0].toObject();
        if (!wpTypeObj.contains("wallpaperInfo")) {
            continue;
        }

        QJsonArray wpInfoArray = wpTypeObj["wallpaperInfo"].toArray();
        for (QJsonArray::Iterator it2 = wpInfoArray.begin(); it2 != wpInfoArray.end();) {
            QJsonObject wpInfoObj = it2[0].toObject();
            if (wpInfoObj.contains("wpIndex")) {
                bool ok;
                int index = wpInfoObj["wpIndex"].toString().split("+").first().toInt(&ok);
                if (ok && index > size) {
                    it2 = wpInfoArray.erase(it2);
                    bSave = true;
                    continue;
                }
            }
            ++it2;
        }

        wpTypeObj["wallpaperInfo"] = wpInfoArray;
        it1[0] = wpTypeObj;
    }

    if (bSave) {
        DconfigSettings::ConfigSaveValue(appearanceProcessId, appearanceDconfJson, allWallpaperUrisKey, allWallpaperUri.toVariantList());
    }
}
