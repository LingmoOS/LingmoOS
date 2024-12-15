// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearancesyncconfig.h"

#include "dbus/appearancedbusproxy.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGlobal>

ThemeFontSyncConfig::ThemeFontSyncConfig(QString name, QString path, QSharedPointer<AppearanceManager> appearManager)
    : SyncConfig(name, path)
{
    appearanceManager = appearManager;
}

QByteArray ThemeFontSyncConfig::Get()
{
    QJsonDocument doc;
    QJsonObject syncData;
    syncData.insert("version", "1.0");
    syncData.insert("font_size", appearanceManager->getFontSize());
    syncData.insert("gtk", appearanceManager->getGtkTheme());
    syncData.insert("icon", appearanceManager->getIconTheme());
    syncData.insert("cursor", appearanceManager->getCursorTheme());
    syncData.insert("font_standard", appearanceManager->getStandardFont());
    syncData.insert("font_monospace", appearanceManager->getMonospaceFont());

    doc.setObject(syncData);

    return doc.toJson(QJsonDocument::Compact);
}

void ThemeFontSyncConfig::Set(QByteArray data)
{
    QJsonParseError err;
    QJsonDocument rootDoc = QJsonDocument::fromJson(data, &err); // 字符串格式化为JSON

    if (err.error != QJsonParseError::NoError) {
        qDebug() << "JSON格式错误";
    } else {
        QJsonObject syncData = rootDoc.object();
        bool bsuccess;
        if (!qFuzzyCompare(syncData.value("font_size").toDouble(), appearanceManager->getFontSize())) {
            bsuccess = appearanceManager->doSetFonts(syncData.value("font_size").toDouble());
            if (bsuccess) {
                appearanceManager->setFontSize(syncData.value("font_size").toDouble());
            }
        }

        if (syncData.value("gtk").toString() != appearanceManager->getGtkTheme()) {
            bsuccess = appearanceManager->doSetGtkTheme(syncData.value("gtk").toString());
            if (bsuccess) {
                appearanceManager->setGtkTheme(syncData.value("gtk").toString());
            }
        }

        if (syncData.value("icon").toString() != appearanceManager->getIconTheme()) {
            bsuccess = appearanceManager->doSetIconTheme(syncData.value("icon").toString());
            if (bsuccess) {
                appearanceManager->setIconTheme(syncData.value("icon").toString());
            }
        }

        if (syncData.value("cursor").toString() != appearanceManager->getCursorTheme()) {
            bsuccess = appearanceManager->doSetCursorTheme(syncData.value("cursor").toString());
            if (bsuccess) {
                appearanceManager->setCursorTheme(syncData.value("cursor").toString());
            }
        }

        if (syncData.value("font_standard").toString() != appearanceManager->getStandardFont()) {
            bsuccess = appearanceManager->doSetStandardFont(syncData.value("font_standard").toString());
            if (bsuccess) {
                appearanceManager->setStandardFont(syncData.value("font_standard").toString());
            }
        }

        if (syncData.value("font_monospace").toString() != appearanceManager->getMonospaceFont()) {
            bsuccess = appearanceManager->doSetMonospaceFont(syncData.value("font_monospace").toString());
            if (bsuccess) {
                appearanceManager->setMonospaceFont(syncData.value("font_monospace").toString());
            }
        }
    }
}

BackgroundSyncConfig::BackgroundSyncConfig(QString name, QString path, QSharedPointer<AppearanceManager> appearManager)
    : SyncConfig(name, path)
{
    appearanceManager = appearManager;
}

QByteArray BackgroundSyncConfig::Get()
{
    QJsonDocument doc;
    QJsonObject syncData;
    syncData.insert("version", "2.0");

    QJsonObject uploadSlideObject;
    QJsonParseError err_rpt;
    QString wallpaperSlideShow = appearanceManager->getWallpaperSlideShow();
    QJsonDocument tmpDoc = QJsonDocument::fromJson(wallpaperSlideShow.toLatin1(), &err_rpt);
    if (err_rpt.error != QJsonParseError::NoError) {
        return nullptr;
    } else {
        QVariantMap wallSlideMap = tmpDoc.object().toVariantMap();
        for (auto item : wallSlideMap.toStdMap()) {
            QStringList keyList = item.first.split("&&");
            if (keyList.size() < 2) {
                continue;
            }

            if (keyList[1].toInt() < 1) {
                return nullptr;
            }

            QString monitorName;
            QMap<QString, QString> monitorMap = appearanceManager->getMonitor();
            if (monitorMap.count(keyList[0]) != 0) {
                monitorName = monitorMap[keyList[0]];
            }
            QString key = QString("%1&&%2").arg(monitorName).arg(keyList[1]);
            uploadSlideObject.insert(key, item.second.toString());
        }
    }

    QJsonObject uploadUrisObject;
    QString wallpaperUris = appearanceManager->getWallpaperURls();
    tmpDoc = QJsonDocument::fromJson(wallpaperSlideShow.toLatin1(), &err_rpt);
    if (err_rpt.error != QJsonParseError::NoError) {
        return nullptr;
    } else {
        QVariantMap wallpaperUrisMap = tmpDoc.object().toVariantMap();
        for (auto item : wallpaperUrisMap.toStdMap()) {
            if (uploadSlideObject.value(item.first).isNull()) {
                uploadUrisObject.insert(item.first, item.second.toString());
            }
        }
    }

    syncData.insert("wallpaper_uris", uploadUrisObject);
    syncData.insert("slide_show_config", uploadSlideObject);
    doc.setObject(syncData);

    return doc.toJson(QJsonDocument::Compact);
}

void BackgroundSyncConfig::Set(QByteArray data)
{
    QJsonParseError err;
    QJsonDocument rootDoc = QJsonDocument::fromJson(data, &err); // 字符串格式化为JSON

    if (err.error != QJsonParseError::NoError) {
        qDebug() << "JSON格式错误";
        return;
    }

    QJsonObject syncData = rootDoc.object();
    if (syncData.value("greeter_background").toString() == appearanceManager->getBackground()) {
        appearanceManager->doSetGreeterBackground(syncData.value("greeter_background").toString());
    }

    QMap<QString, QString> monitorMap = appearanceManager->getMonitor();
    QMap<QString, QString> reverseMonitorMap;
    for (auto item : monitorMap.toStdMap()) {
        reverseMonitorMap[item.second] = item.first;
    }

    if (syncData.value("version").toString() == "1.0") {
        QJsonDocument docWallSlide;
        QJsonObject slidewConfigObject;
        QString primaryMonitor;
        if (reverseMonitorMap.count("Primary") == 1) {
            primaryMonitor = reverseMonitorMap["Primary"];
        }

        QString wallPaperSlide = appearanceManager->getWallpaperSlideShow();
        if (!wallPaperSlide.isEmpty()) {
            docWallSlide = QJsonDocument::fromJson(wallPaperSlide.toLatin1());
            for (auto item : docWallSlide.object().toVariantMap().toStdMap()) {
                slidewConfigObject.insert(item.first, item.second.toString());
            }
        }

        int workspaceCount = appearanceManager->getWorkspaceCount();
        for (int i = 0; i < workspaceCount; i++) {
            QString key = QString("%1&&%2").arg(primaryMonitor).arg(i);
            slidewConfigObject[key] = syncData.value("slide_show").toString();
        }
        QJsonDocument tempWallpaperSlideShowDoc;
        tempWallpaperSlideShowDoc.setObject(slidewConfigObject);
        bool bSuccess = appearanceManager->setWallpaperSlideShow(tempWallpaperSlideShowDoc.toJson(QJsonDocument::Compact));
        if (!bSuccess) {
            return;
        }

        if (syncData.value("slide_show").toString().isEmpty()) {
            return;
        }

        QJsonObject wallpaperURIsObject;
        QString wallpaperURIs = appearanceManager->getWallpaperURls();
        QJsonDocument docWallUris = QJsonDocument::fromJson(wallpaperURIs.toLatin1());
        if (!wallpaperURIs.isEmpty()) {
            for (auto item : docWallUris.object().toVariantMap().toStdMap()) {
                wallpaperURIsObject.insert(item.first, item.second.toString());
            }

            QSharedPointer<AppearanceDBusProxy> dbusProxy = appearanceManager->getDBusProxy();
            QJsonArray backgroundURIsArr = syncData.value("background_uris").toArray();
            for (int i = 0; i < backgroundURIsArr.size(); i++) {
                dbusProxy->SetWorkspaceBackgroundForMonitor(i + 1, primaryMonitor, backgroundURIsArr[i].toString());
                QString key = QString("%1&&%2").arg(primaryMonitor).arg(i + 1);
                wallpaperURIsObject.insert(key, backgroundURIsArr[i]);
            }

            QJsonDocument tempWallpaperSURIsDoc;
            tempWallpaperSURIsDoc.setObject(wallpaperURIsObject);
            appearanceManager->setWallpaperURls(tempWallpaperSURIsDoc.toJson(QJsonDocument::Compact));
        }
    }

    QJsonObject slideShowObjec;
    QVariantMap slideShowConfigMap = syncData.value("slide_show_config").toObject().toVariantMap();
    for (auto item : slideShowConfigMap.toStdMap()) {
        QStringList keySlice = item.first.split("&&");
        if (keySlice.size() < 2) {
            continue;
        }

        if (keySlice[1].toInt() < 1) {
            return;
        }

        QString monitorName;
        if (reverseMonitorMap.count(keySlice[0]) == 1) {
            monitorName = reverseMonitorMap[keySlice[0]];
        }
        QString key = QString("%1&&%2").arg(monitorName).arg(keySlice[1]);
        slideShowObjec.insert(key, item.second.toString());
    }

    QJsonDocument slideShowDoc(slideShowObjec);
    bool bSuccess = appearanceManager->setWallpaperSlideShow(slideShowDoc.toJson(QJsonDocument::Compact));
    if (!bSuccess) {
        return;
    }

    bSuccess = appearanceManager->setWallpaperURls(syncData.value("wallpaper_uris").toString());
    if (!bSuccess) {
        return;
    }

    QSharedPointer<AppearanceDBusProxy> dbusProxy = appearanceManager->getDBusProxy();
    int workspaceCount = appearanceManager->getWorkspaceCount();
    QVariantMap wallPaperUrisConfigMap = syncData.value("wallpaper_uris").toObject().toVariantMap();
    for (auto item : wallPaperUrisConfigMap.toStdMap()) {
        QStringList keySlice = item.first.split("&&");
        if (keySlice.size() < 2) {
            continue;
        }

        if (keySlice[1].toInt() < 1) {
            return;
        }

        if (keySlice[1].toInt() > workspaceCount) {
            continue;
        }

        QString monitorName;
        if (reverseMonitorMap.count(keySlice[0]) == 1) {
            monitorName = reverseMonitorMap[keySlice[0]];
        }

        dbusProxy->SetWorkspaceBackgroundForMonitor(keySlice[1].toInt(), monitorName, item.second.toString());
    }
}
