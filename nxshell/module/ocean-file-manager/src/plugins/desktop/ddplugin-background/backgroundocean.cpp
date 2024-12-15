// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundocean.h"

#include <DConfig>

#include <QJsonDocument>

DDP_BACKGROUND_USE_NAMESPACE
DCORE_USE_NAMESPACE

BackgroundOCEAN::BackgroundOCEAN(QObject *parent)
    : BackgroundService(parent)
{
    fmDebug() << "create org.lingmo.ocean.Appearance1";
    interface = new InterFace("org.lingmo.ocean.Appearance1", "/org/lingmo/ocean/Appearance1",
                          QDBusConnection::sessionBus(), this);
    interface->setTimeout(200);
    fmDebug() << "create org.lingmo.ocean.Appearance1 end";

    apperanceConf = DConfig::create("org.lingmo.ocean.appearance", "org.lingmo.ocean.appearance", "", this);
    connect(apperanceConf, &DConfig::valueChanged, this, &BackgroundOCEAN::onAppearanceValueChanged);
}

BackgroundOCEAN::~BackgroundOCEAN()
{
    if (interface) {
        interface->deleteLater();
        interface = nullptr;
    }
}

QString BackgroundOCEAN::getBackgroundFromOCEAN(const QString &screen)
{
    QString path;
    if (screen.isEmpty())
        return path;

    fmDebug() << "Get background by OCEAN GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
    QDBusPendingReply<QString> reply = interface->GetCurrentWorkspaceBackgroundForMonitor(screen);
    reply.waitForFinished();

    if (reply.error().type() != QDBusError::NoError) {
        fmWarning() << "Get background failed by OCEAN_DBus"
                   << reply.error().type() << reply.error().name() << reply.error().message();
    } else {
        path = reply.argumentAt<0>();
    }

    return path;
}

QString BackgroundOCEAN::getBackgroundFromConfig(const QString &screen)
{
    QString path;
    QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    QFile oceanFile(configPath + "/ocean-appearance/config.json");
    if (!oceanFile.open(QFile::ReadOnly | QIODevice::Text)) {
        fmWarning() << "config file doesn't exist";
        return path;
    }

    //Judge whether the configuration file is valid
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(oceanFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        fmCritical() << "config file is invailid :" << error.errorString();
        return path;
    }

    //Find the background path based on the workspace and screen name
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (int i = 0; i < arr.size(); i++) {
            if (arr.at(i).isObject()) {
                QJsonValue type = arr.at(i).toObject().value("type");
                QJsonValue info = arr.at(i).toObject().value("wallpaperInfo");
                if (type.toString() == ("index+monitorName") && info.isArray()) {
                    QJsonArray val = info.toArray();
                    for (int j = 0; j < val.size(); j++) {
                        if (val.at(j).isObject()) {
                            QString wpIndex = val.at(j).toObject().value("wpIndex").toString();
                            int index = wpIndex.indexOf("+");
                            if (index <= 0) {
                                continue;
                            }

                            int workspaceIndex = wpIndex.left(index).toInt();
                            QString screenName = wpIndex.mid(index+1);
                            if (workspaceIndex != currentWorkspaceIndex || screenName != screen) {
                                continue;
                            }

                            path = val.at(j).toObject().value("uri").toString();
                            break;
                        }
                    }
                }
            }
        }
    }
    oceanFile.close();

    return path;
}

void BackgroundOCEAN::onAppearanceValueChanged(const QString &key)
{
    if (key == QString("Wallpaper_Uris")) {
        fmDebug() << "appearance Wallpaper_Uris changed...";
        emit backgroundChanged();
    }
}

QString BackgroundOCEAN::background(const QString &screen)
{
    QString path;

    if (!screen.isEmpty()) {

        //1.Get the background from OCEAN
        path = getBackgroundFromOCEAN(screen);
        fmDebug() << "getBackgroundFromOCEAN path :" << path << "screen" << screen;

        if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
            // 2.Parse background from config file
            path = getBackgroundFromConfig(screen);
            fmWarning() << "getBackgroundFormConfig path :" << path << "screen" << screen;

            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                // 3.Use the default background
                path = getDefaultBackground();
                fmCritical() << "getDefaultBackground path :" << path << "screen" << screen;
            }
        }
    } else {
        fmDebug() << "Get background path terminated screen:" << screen << interface;
    }

    return path;
}

QString BackgroundOCEAN::getDefaultBackground()
{
    return BackgroundService::getDefaultBackground();
}
