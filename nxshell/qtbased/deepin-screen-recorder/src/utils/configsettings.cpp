// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsettings.h"
#include "saveutils.h"
#include "../utils.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QTemporaryFile>
#include <QDebug>

/*
RECORD_TYPE_GIF = 0;
RECORD_TYPE_MP4 = 1;
RECORD_TYPE_MKV = 2;

RECORD_MOUSE_NULL = 0;
RECORD_MOUSE_CURSE = 1;
RECORD_MOUSE_CHECK = 2;
RECORD_MOUSE_CURSE_CHECK = 3;

RECORD_AUDIO_NULL = 0;
RECORD_AUDIO_MIC = 1;
RECORD_AUDIO_SYSTEMAUDIO = 2;
RECORD_AUDIO_MIC_SYSTEMAUDIO = 3;
*/
ConfigSettings::ConfigSettings(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings("deepin/deepin-screen-recorder", "deepin-screen-recorder");
    //qDebug() << "config file path: " << m_settings->fileName();
}

ConfigSettings *ConfigSettings::m_configSettings = nullptr;
const QMap<QString, QMap<QString, QVariant>> ConfigSettings::m_defaultConfig = {
    {"rectangle", {{"color_index", 3}, {"line_width", 0}}},
    {"oval", {{"color_index", 3}, {"line_width", 0}}},
    {"line", {{"color_index", 3}, {"line_width", 0}}},
    {"arrow", {{"color_index", 3}, {"line_width", 0}}},
    {"pen", {{"color_index", 3}, {"line_width", 0}}},
    // 模糊类型， 模糊、马赛克；模糊形状，矩形，圆形；模糊强度
    {"effect", {{"isBlur", false}, {"isOval", 0}, {"radius", 0}, {"line_width", 1}}},
    {"text", {{"color_index", 3}, {"fontsize", 0}}},
    // 截图保存选项，格式，保存位置选项
    //save_dir: 普通路径
    //save_dir_change: 指定路径是否需要改变
    {"shot", {{"format", 0}, {"save_op", 0}, {"save_cursor", 0}, {"save_dir", ""},  {"save_dir_change", false}, {"border_index", 0}}},
    // 录屏保存选项
    // curor 0 不录制鼠标，及不录制鼠标点击,1 录制鼠标,2 录制鼠标点击,3 录制鼠标，及录制鼠标点击,
    // audio 0 不录制任何音频,1 麦克风音频, 2 录制系统音频,3 录制混音,
    // save_op 保存位置视频目录 0, 桌面 1
    {"recorder", {{"format", 1}, {"frame_rate", 24}, {"save_op", 0}, {"save_dir", ""}, {"cursor", 0}, {"audio", 0}}},
};
ConfigSettings *ConfigSettings::instance()
{
    if (!m_configSettings) {
        m_configSettings = new ConfigSettings();
    }

    return m_configSettings;
}

void ConfigSettings::setValue(const QString &group, const QString &key, const QVariant &val)
{
    if (!m_defaultConfig.contains(group) || !m_defaultConfig[group].contains(key)) {
        qDebug() << __FUNCTION__ << __LINE__ <<
                 "default config file is not contains group(" << group <<
                 ") or group is not contains key(" << key << ")";
        return;
    }

    if (val.type() == QVariant::Int) {
        emit shapeConfigChanged(group, key, val.toInt());
    }

    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup(group);
    m_settings->setValue(key, val);
    m_settings->endGroup();
    m_settings->sync();
}

QVariant ConfigSettings::getValue(const QString &group, const QString &key)
{
    QMutexLocker locker(&m_mutex);
    QVariant value;
    m_settings->beginGroup(group);

    if (m_settings->contains(key)) {
        value = m_settings->value(key);
    } else {
        value = getDefaultValue(group, key);
    }
    m_settings->endGroup();
    return value;
}

QVariant ConfigSettings::getDefaultValue(const QString &group, const QString &key)
{
    QVariant value;
    if (m_defaultConfig.contains(group) && m_defaultConfig[group].contains(key)) {
        value.setValue(m_defaultConfig[group][key]);
    } else {
        qWarning() << __FUNCTION__ << __LINE__ <<
                   "ERROR! config file is not contains group(" << group <<
                   ") or group is not contains key(" << key << ")";
    }
    return  value;
}

QStringList ConfigSettings::keys(const QString &group)
{
    QStringList v;
    m_settings->beginGroup(group);
    v = m_settings->childKeys();
    m_settings->endGroup();
    return v;
}

ConfigSettings::~ConfigSettings()
{
}
