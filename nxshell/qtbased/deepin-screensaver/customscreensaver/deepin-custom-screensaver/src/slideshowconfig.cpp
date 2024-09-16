// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "slideshowconfig.h"
#include "slideshowconfigdialog.h"

#include <QDir>
#include <QStandardPaths>
#include <QApplication>

class SlideshowConfigGlobal : public SlideshowConfig
{
};
Q_GLOBAL_STATIC(SlideshowConfigGlobal, slideshowConfig);


SlideshowConfig::SlideshowConfig(QObject *parent)
    : QObject(parent)
{
    QFileInfo file(confPath());
    if (file.exists())
        m_settings.reset(new QSettings(confPath(), QSettings::IniFormat));

    m_dCfg = DConfig::create(kCfgAppId, kCfgName, "", this);
}

SlideshowConfig::~SlideshowConfig()
{
}

SlideshowConfig *SlideshowConfig::instance()
{
    return slideshowConfig;
}

QString SlideshowConfig::slideshowPath() const
{
    QString path;
    path = m_dCfg->value(kSlideshowPath, "").toString();

    // 组策略值为空，尝试读配置文件
    if (path.isEmpty() && m_settings) {
        m_settings->beginGroup(kGroupSlideshowPath);
        path = m_settings->value(kKeyValue, "").toString();
        m_settings->endGroup();
    }

    if (path.isEmpty())
        path = defaultPath();

    if (path.startsWith(QStringLiteral("~")))
        path.replace(QStringLiteral("~"), QDir::homePath());

    return path;
}

void SlideshowConfig::setSlideShowPath(QString path)
{
    if (path.startsWith(QStringLiteral("~")))
        path.replace(QStringLiteral("~"), QDir::homePath());

    m_dCfg->setValue(kSlideshowPath, path);

    if (m_settings) {
        m_settings->beginGroup(kGroupSlideshowPath);
        m_settings->setValue(kKeyValue, path);
        m_settings->endGroup();
    }
}

int SlideshowConfig::intervalTime() const
{
    int intervalTime = m_dCfg->value(kIntervalTime, 0).toInt();
    if (intervalTime < 1 && m_settings) {
        m_settings->beginGroup(kGroupIntervalTime);
        intervalTime = m_settings->value(kKeyValue, 0).toInt();
        m_settings->endGroup();
    }

    if (intervalTime < kMinIntervalTime || intervalTime > kMaxIntervalTime)
        intervalTime = kDefaultTime;

    return intervalTime;
}

void SlideshowConfig::setIntervalTime(int tempTime)
{
    if (tempTime < kMinIntervalTime || tempTime > kMaxIntervalTime)
        tempTime = kDefaultTime;

    m_dCfg->setValue(kIntervalTime, tempTime);

    if (m_settings) {
        m_settings->beginGroup(kGroupIntervalTime);
        m_settings->setValue(kKeyValue, tempTime);
        m_settings->endGroup();
    }
}

bool SlideshowConfig::isShuffle() const
{
    int mode = m_dCfg->value(kPlayMode, -1).toInt();
    if (mode < 0 && m_settings) {
        m_settings->beginGroup(kGroupShuffle);
        mode = m_settings->value(kKeyValue, false).toBool() ? Shuffle : Order;
        m_settings->endGroup();
    }

    return mode == Shuffle;
}

void SlideshowConfig::setShuffle(const bool shuffle)
{
    if (m_settings) {
        m_settings->beginGroup(kGroupShuffle);
        m_settings->setValue(kKeyValue, shuffle);
        m_settings->endGroup();
    }

    m_dCfg->setValue(kPlayMode, shuffle ? Shuffle : Order);
}

bool SlideshowConfig::startCustomConfig()
{
    auto ins = new SingleInstance(qApp->applicationName() + "-config", this);
    if (ins->isSingle()) {
        SlideShowConfigDialog *configDialog = new SlideShowConfigDialog();

        connect(ins, &SingleInstance::handArguments, configDialog, [configDialog](){
            configDialog->activateWindow();;
        });

        configDialog->startConfig();
    } else {
        ins->sendNewClient();
        return false;
    }

    return true;
}

QString SlideshowConfig::defaultPath()
{
    return "~/Pictures";
}

QString SlideshowConfig::confPath()
{
    static QString confFilePath;

    if (confFilePath.isEmpty()) {
        auto configPaths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
        Q_ASSERT(!configPaths.isEmpty());

        confFilePath = configPaths.first();
        confFilePath = confFilePath
                + "/" + QApplication::organizationName()
                + "/" + kDeepinScreenSaver + "/"
                + QApplication::applicationName()
                + "/" + QApplication::applicationName() + ".conf";
    }
    return confFilePath;
}
