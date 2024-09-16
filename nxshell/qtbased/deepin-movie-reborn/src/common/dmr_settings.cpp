// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtCore>
#include <QtGui>

#include <DStandardPaths>

#include "dmr_settings.h"
#include "compositing_manager.h"
#include "utils.h"
#include <qsettingbackend.h>

namespace dmr {
using namespace Dtk::Core;
Settings *Settings::m_pTheSettings = nullptr;
Settings &Settings::get()
{
    if (!m_pTheSettings) {
        m_pTheSettings = new Settings;
    }

    return *m_pTheSettings;
}

Settings::Settings()
    : QObject(nullptr)
{
    m_sConfigPath = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_sConfigPath += "/config.conf";
    qInfo() << "configPath" << m_sConfigPath;
    QSettingBackend *pBackend = new QSettingBackend(m_sConfigPath);
    if(!CompositingManager::isMpvExists()) {
        m_pSettings = DSettings::fromJsonFile(":/resources/data/GstSettings.json");
    } else {
#if defined (__mips__) || defined (__sw_64__) || defined ( __aarch64__)
    m_pSettings = DSettings::fromJsonFile(":/resources/data/lowEffectSettings.json");
#else
    m_pSettings = DSettings::fromJsonFile(":/resources/data/settings.json");
#endif
    }
    m_pSettings->setBackend(pBackend);

    connect(m_pSettings, &DSettings::valueChanged,
    [ = ](const QString & key, const QVariant & value) {
        if (key.startsWith("shortcuts."))
            emit shortcutsChanged(key, value);
        else if (key.startsWith("base.play.playmode"))
            emit defaultplaymodechanged(key, value);
        else if (key.startsWith("base.decode.select")) {
            //设置解码模式
            emit setDecodeModel(key, value);
            //刷新解码模式
            emit refreshDecode();
            //崩溃检测
            crashCheck();
        }
        else if (key.startsWith("base.play.hwaccel"))
            emit hwaccelModeChanged(key, value);
        else if (key.startsWith("base.play.mute"))
            emit baseMuteChanged(key, value);
        else if (key.startsWith("base."))
            emit baseChanged(key, value);
        else if (key.startsWith("subtitle."))
            emit subtitleChanged(key, value);
    });

    qInfo() << "keys" << m_pSettings->keys();

    QStringList playmodeDatabase;
    playmodeDatabase << tr("Order play")
                     << tr("Shuffle play")
                     << tr("Single play")
                     << tr("Single loop")
                     << tr("List loop");
    auto playmodeFamily = m_pSettings->option("base.play.playmode");
    playmodeFamily->setData("items", playmodeDatabase);

    QStringList hwaccelDatabase;
    hwaccelDatabase << tr("Auto")
                    << tr("Open")
                    << tr("Close");
    auto hwaccelFamily = m_pSettings->option("base.play.hwaccel");
    hwaccelFamily->setData("items", hwaccelDatabase);

    QFontDatabase fontDatabase;
    QPointer<DSettingsOption> fontFamliy = m_pSettings->option("subtitle.font.family");
    if(fontFamliy) {
        fontFamliy->setData("items", fontDatabase.families());
    }

    QFileInfo fi("/dev/mwv206_0");      //景嘉微显卡默认不勾选预览
    QFileInfo jmfi("/dev/jmgpu");
    if ((fi.exists() || jmfi.exists()) && utils::check_wayland_env()) {
        setInternalOption("mousepreview", false);
    }
}

QString Settings::flag2key(Settings::Flag f)
{
    switch (f) {
    case Settings::Flag::ClearWhenQuit:
        return "emptylist";
#ifndef __aarch64__
    case Settings::Flag::ShowThumbnailMode:
        return "showInthumbnailmode";
#endif
    case Settings::Flag::ResumeFromLast:
        return "resumelast";
    case Settings::Flag::AutoSearchSimilar:
        return "addsimilar";
    case Settings::Flag::PreviewOnMouseover:
        return "mousepreview";
    case Settings::Flag::MultipleInstance:
        return "multiinstance";
    case Settings::Flag::PauseOnMinimize:
        return "pauseonmin";
    }

    return "";
}

bool Settings::isSet(Flag flag) const
{
    bool bRet = false;
    QList<QPointer<DSettingsGroup> > listSubGroups = m_pSettings->group("base")->childGroups();
    QList<QPointer<DSettingsGroup> >::iterator itor = std::find_if(listSubGroups.begin(), listSubGroups.end(), [ = ](GroupPtr grp) {
        return grp->key() == "base.play";
    });

    if (itor != listSubGroups.end()) {
        QList<QPointer<DSettingsOption> > sub = (*itor)->childOptions();
        QString sKey = flag2key(flag);

        QList<QPointer<DSettingsOption> >::iterator p = std::find_if(sub.begin(), sub.end(), [ = ](OptionPtr opt) {
            QString sOptKey = opt->key();
            sOptKey.remove(0, sOptKey.lastIndexOf('.') + 1);
            return sOptKey == sKey;
        });

        bRet = (p != sub.end() && (*p)->value().toBool());
    }
    return bRet;
}

QStringList Settings::commonPlayableProtocols() const
{
    //from mpv and combined with stream media protocols
    return {
        "http", "https", "bd", "ytdl", "smb", "dvd", "dvdread", "tv", "pvr",
        "dvb", "cdda", "lavf", "av", "avdevice", "fd", "fdclose", "edl",
        "mf", "null", "memory", "hex", "rtmp", "rtsp", "hls", "mms", "rtp",
        "rtcp"
    };
}

bool Settings::iscommonPlayableProtocol(const QString &sScheme) const
{
//    for (auto pro : commonPlayableProtocols()) {
//        if (pro == sScheme)
//            return true;
//    }
//    return false;

    QStringList list = commonPlayableProtocols();
    bool result = std::any_of(list.begin(), list.end(), [&](QString & _pro) {
        return _pro == sScheme;
    });

    return result;
}

QString Settings::screenshotLocation()
{
    QString sSavePath = settings()->value("base.screenshot.location").toString();
    if (sSavePath.size() && sSavePath[0] == '~') {
        sSavePath.replace(0, 1, QDir::homePath());
    }

    if (!QFileInfo(sSavePath).exists()) {
        QDir dir;
        dir.mkpath(sSavePath);
    }

    return sSavePath;
}

QString Settings::screenshotNameTemplate()
{
    QString strMovie = QObject::tr("Movie");
    QString path = screenshotLocation() + QDir::separator() + strMovie +
            QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".jpg");
    return path;
}

//cppcheck 单元测试使用
QString Settings::screenshotNameSeqTemplate()
{
    return tr("%1/Movie%2(%3).jpg").arg(screenshotLocation())
           .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
}

void Settings::onSetCrash()
{
    settings()->setOption(QString("set.start.crash"),false);
    settings()->sync();
}

void Settings::setGeneralOption(const QString &sOpt, const QVariant &var)
{
    settings()->setOption(QString("base.general.%1").arg(sOpt), var);
    settings()->sync();
}

void Settings::crashCheck()
{
    //重置崩溃检测状态位
    settings()->setOption(QString("set.start.crash"),true);
    settings()->sync();
    //崩溃检测
    QTimer::singleShot(1000, this, &Settings::onSetCrash);
}

QVariant Settings::generalOption(const QString &sOpt)
{
    return settings()->getOption(QString("base.general.%1").arg(sOpt));
}

QVariant Settings::internalOption(const QString &sOpt)
{
    return settings()->getOption(QString("base.play.%1").arg(sOpt));
}

void Settings::setInternalOption(const QString &sOpt, const QVariant &var)
{
    settings()->setOption(QString("base.play.%1").arg(sOpt), var);
    settings()->sync();
}

QString Settings::forcedInterop()
{
    return internalOption("forced_interop").toString();
}

bool Settings::disableInterop()
{
    return internalOption("disable_interop").toBool();
}

}
