// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "appletdata.h"
#include "pluginmetadata.h"

#include <DObject>
#include <QVariant>

DS_BEGIN_NAMESPACE
/**
 * @brief 插件项，单个插件实例
 */
class DAppletPrivate;
class DPluginLoader;
class DS_SHARE DApplet : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(QString pluginId READ pluginId CONSTANT FINAL)
    Q_PROPERTY(DApplet *parent READ parentApplet CONSTANT FINAL)
    Q_PROPERTY(QObject *rootObject READ rootObject NOTIFY rootObjectChanged)
    D_DECLARE_PRIVATE(DApplet)
    friend class DPluginLoader;
public:
    explicit DApplet(QObject *parent = nullptr);
    virtual ~DApplet() override;

    QString id() const;
    QString pluginId() const;
    QObject *rootObject() const;
    DApplet *parentApplet() const;

    DPluginMetaData pluginMetaData() const;
    DAppletData appletData() const;
    void setAppletData(const DAppletData &data);
    void setRootObject(QObject *root);

    virtual bool load();
    virtual bool init();

Q_SIGNALS:
    void rootObjectChanged();

protected:
    explicit DApplet(DAppletPrivate &dd, QObject *parent = nullptr);

private:
    void setMetaData(const DPluginMetaData &metaData);
};

DS_END_NAMESPACE
