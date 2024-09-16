// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include <QSharedData>
#include <QVariant>

DS_BEGIN_NAMESPACE

/**
 * @brief 插件实例数据
 */
class DPluginMetaData;
class DAppletDataPrivate;
class DS_SHARE DAppletData : public QObject
{
    Q_OBJECT
public:
    explicit DAppletData();
    explicit DAppletData(const QVariantMap &data);
    DAppletData(const DAppletData &other);
    explicit DAppletData(const QString &pluginId);
    DAppletData &operator=(const DAppletData &other);
    bool operator==(const DAppletData &other) const;
    virtual ~DAppletData() override;

    bool isValid() const;
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    QString id() const;
    void setId(const QString &id);
    QString pluginId() const;
    QList<DAppletData> groupList() const;
    void setGroupList(const QList<DAppletData> &groupList);
    QVariantMap toMap() const;

    static DAppletData fromPluginMetaData(const DPluginMetaData &metaData);

private:
    QExplicitlySharedDataPointer<DAppletDataPrivate> d;
};

DS_END_NAMESPACE
