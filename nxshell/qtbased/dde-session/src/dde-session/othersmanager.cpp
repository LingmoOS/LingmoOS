// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "othersmanager.h"
#include "utils/utils.h"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

OthersManager::OthersManager()
{

}

void OthersManager::init()
{
    qInfo() << "others manager init";

#ifndef QT_DEBUG
    launchWmChooser();
#endif

    qInfo() << "others manager init finished";
}

/**
 * @brief OthersManager::launchWmChooser
 * @note 选择特效模式
 */
void OthersManager::launchWmChooser()
{
    const QString &config = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + "/deepin/deepin-wm-switcher/config.json";
    if (!Utils::IS_WAYLAND_DISPLAY && inVM()) {
        if (!QFile(config).exists())
            return;

        EXEC_COMMAND("dde-wm-chooser", QStringList() << "-c" << config);
    }

    // kwin存在的情况
    if (!QStandardPaths::findExecutable("deepin-kwin_x11").isEmpty() && QFile(config).exists()) {
        QFile configFile(config);
        if (!configFile.open(QIODevice::ReadOnly)) {
            qWarning() << "failed to open file: " << config;
            return;
        }

        QJsonParseError jsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(configFile.readAll(), &jsonError);
        configFile.close();
        if (jsonError.error != QJsonParseError::NoError) {
            qWarning() << "failed to parse json file:" << config <<", error type: " << jsonError.error;
            return;
        }

        QJsonObject rootObj = jsonDocument.object();
        const QString &lastWm = rootObj.value("last_wm").toString();
        bool compositingEnabled = (lastWm == "deepin-wm");

        // 更新kwinrc的配置文件
        const QString &kwinRc = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin-kwinrc";
        QSettings settings(kwinRc, QSettings::IniFormat);
        settings.beginGroup("Compositing");
        settings.setValue("Enabled", compositingEnabled);

        qDebug() << "update compositing enabled: " << compositingEnabled;
    }
}

/**
 * @brief OthersManager::inVM
 * @return 当前运行环境是否是虚拟机
 */
bool OthersManager::inVM()
{
    QProcess p;
    p.start("systemd-detect-virt", QStringList() << "-v" << "-q");
    p.waitForFinished();

    return  p.exitCode() != 0;
}
