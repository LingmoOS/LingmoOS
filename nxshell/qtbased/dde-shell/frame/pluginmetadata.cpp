// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmetadata.h"

#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDir>

DS_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(dsLog, "dde.shell")

class DPluginMetaDataPrivate : public QSharedData
{
public:
    QVariantMap rootObject() const
    {
        return m_metaData["Plugin"].toMap();
    }
    QString m_pluginId;
    QVariantMap m_metaData;
    QString m_pluginDir;
};

DPluginMetaData::DPluginMetaData()
    : d(new DPluginMetaDataPrivate())
{
}

DPluginMetaData::DPluginMetaData(const DPluginMetaData &other)
    : d(other.d)
{
}

DPluginMetaData &DPluginMetaData::operator=(const DPluginMetaData &other)
{
    d = other.d;
    return *this;
}

DPluginMetaData::DPluginMetaData(DPluginMetaData &&other)
    : d(other.d)
{
    other.d = nullptr;
}

DPluginMetaData &DPluginMetaData::operator=(DPluginMetaData &&other)
{
    d.swap(other.d);
    return *this;
}

bool DPluginMetaData::operator==(const DPluginMetaData &other) const
{
    return d->m_pluginId == other.pluginId();
}

DPluginMetaData::~DPluginMetaData()
{

}

bool DPluginMetaData::isValid() const
{
    return !d->m_pluginId.isEmpty();
}

QVariant DPluginMetaData::value(const QString &key, const QVariant &defaultValue) const
{
    if (!isValid())
        return defaultValue;

    auto root = d->rootObject();
    if (!root.contains(key))
        return defaultValue;

    return root.value(key);
}

QString DPluginMetaData::pluginId() const
{
    return d->m_pluginId;
}

QString DPluginMetaData::pluginDir() const
{
    return d->m_pluginDir;
}

QString DPluginMetaData::url() const
{
    auto url = value("Url").toString();
    if (url.isEmpty())
        return QString();

    return QDir(pluginDir()).absoluteFilePath(url);
}

DPluginMetaData DPluginMetaData::fromJsonFile(const QString &file)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(dsLog) << "Couldn't open" << file;
        return DPluginMetaData();
    }

    DPluginMetaData result = fromJsonString(f.readAll());
    if (!result.isValid()) {
        qCWarning(dsLog) << "error parsing" << file;
        return DPluginMetaData();
    }
    result.d->m_pluginDir = QFileInfo(f).absoluteDir().path();
    return result;
}

DPluginMetaData DPluginMetaData::fromJsonString(const QByteArray &data)
{
    QJsonParseError error;
    const QJsonObject metaData = QJsonDocument::fromJson(data).object();
    if (error.error) {
        qCWarning(dsLog) << "error parsing json data"  << error.errorString();
        return DPluginMetaData();
    }

    DPluginMetaData result;
    result.d->m_metaData = metaData.toVariantMap();
    auto root = result.d->rootObject();
    if (root.contains("Id")) {
        result.d->m_pluginId = root["Id"].toString();
    }
    return result;
}

DPluginMetaData DPluginMetaData::rootPluginMetaData()
{
    static DPluginMetaData applet = fromJsonString(R"delimiter(
        {
            "Plugin": {
                "Version": "1.0",
                "Id": "org.deepin.ds.root",
                "ContainmentType": "Root"
            }
        }
        )delimiter");
    return applet;
}

bool DPluginMetaData::isRootPlugin(const QString &pluginId)
{
    return rootPluginMetaData().pluginId() == pluginId;
}

DS_END_NAMESPACE
