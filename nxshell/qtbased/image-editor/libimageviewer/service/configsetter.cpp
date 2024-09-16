// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsetter.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

const QString CONFIG_PATH =   QDir::homePath() +
                              "/.config/deepin/deepin-image-viewer/config.conf";
const QString DB_PATH = QDir::homePath() +
                        "/.local/share/deepin/deepin-image-viewer/deepinimageviewer.db";

LibConfigSetter::LibConfigSetter(QObject *parent) : QObject(parent)
{
    if (!QFileInfo(CONFIG_PATH).exists())
        QProcess::startDetached(QString("rm %1").arg(DB_PATH));

    m_settings = new QSettings(CONFIG_PATH, QSettings::IniFormat, this);
    qDebug() << "Setting file:" << m_settings->fileName();
}

LibConfigSetter *LibConfigSetter::m_setter = nullptr;
LibConfigSetter *LibConfigSetter::instance()
{
    if (! m_setter) {
        m_setter = new LibConfigSetter();
    }

    return m_setter;
}

void LibConfigSetter::setValue(const QString &group, const QString &key,
                            const QVariant &value)
{
//    QMutexLocker locker(&m_mutex);

    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();

    emit valueChanged(group, key, value);
}

QVariant LibConfigSetter::value(const QString &group, const QString &key,
                             const QVariant &defaultValue)
{
    QMutexLocker locker(&m_mutex);

    QVariant value;
    m_settings->beginGroup(group);
    value = m_settings->value(key, defaultValue);
    m_settings->endGroup();

    return value;
}

//QStringList ConfigSetter::keys(const QString group)
//{
//    QStringList v;
//    m_settings->beginGroup(group);
//    v = m_settings->childKeys();
//    m_settings->endGroup();

//    return v;
//}
