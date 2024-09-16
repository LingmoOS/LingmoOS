// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletdata.h"
#include "pluginmetadata.h"

#include <QLoggingCategory>

DS_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

class DAppletDataPrivate : public QSharedData
{
public:
    QVariantMap m_metaData;
};

DAppletData::DAppletData()
    : d(new DAppletDataPrivate())
{
}

DAppletData::DAppletData(const QVariantMap &data)
    : DAppletData()
{
    d->m_metaData = data;
}

DAppletData::DAppletData(const DAppletData &other)
    : d(other.d)
{
}

DAppletData::DAppletData(const QString &pluginId)
    : DAppletData()
{
    d->m_metaData["PluginId"] = pluginId;
}

DAppletData &DAppletData::operator=(const DAppletData &other)
{
    d = other.d;
    return *this;
}

bool DAppletData::operator==(const DAppletData &other) const
{
    return id() == other.id();
}

DAppletData::~DAppletData()
{

}

bool DAppletData::isValid() const
{
    return !pluginId().isEmpty();
}

QVariant DAppletData::value(const QString &key, const QVariant &defaultValue) const
{
    if (!isValid())
        return defaultValue;

    auto root = d->m_metaData;
    if (!root.contains(key))
        return defaultValue;

    return root.value(key);
}

QString DAppletData::id() const
{
    return d->m_metaData["Id"].toString();
}

QString DAppletData::pluginId() const
{
    return d->m_metaData["PluginId"].toString();
}

QList<DAppletData> DAppletData::groupList() const
{
    QList<DAppletData> res;
    auto groups = d->m_metaData["Group"].toList();
    for (const auto &item : groups) {
        DAppletData group(item.toMap());
        res << group;
    }
    return res;
}

void DAppletData::setGroupList(const QList<DAppletData> &groupList)
{
    QList<QVariant> groups;
    for (const auto &item : std::as_const(groupList)) {
        groups << item.toMap();
    }
    d->m_metaData["Group"] = groups;
}

QVariantMap DAppletData::toMap() const
{
    return d->m_metaData;
}

void DAppletData::setId(const QString &id)
{
    d->m_metaData["Id"] = id;
}

DAppletData DAppletData::fromPluginMetaData(const DPluginMetaData &metaData)
{
    QVariantMap group;
    const auto pluginId = metaData.pluginId();
    group["PluginId"] = pluginId;
    return DAppletData{group};
}

DS_END_NAMESPACE
