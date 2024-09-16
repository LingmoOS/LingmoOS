// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGIN_H
#define PLUGIN_H

#include "kpluginmetadata.h"

#include <QObject>
#include <QJsonObject>

class Plugin : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged MEMBER m_enabled)
    Q_PROPERTY(bool readWrite READ isReadWrite CONSTANT)
    Q_PROPERTY(QStringList readOnlyExecutables READ readOnlyExecutables CONSTANT)
    Q_PROPERTY(QStringList readWriteExecutables READ readWriteExecutables CONSTANT)
    Q_PROPERTY(KPluginMetaData metaData READ metaData MEMBER m_metaData CONSTANT)

public:
    explicit Plugin(QObject *parent = nullptr, const KPluginMetaData &metaData = KPluginMetaData());

    int priority() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);
    bool isReadWrite() const;
    QStringList readOnlyExecutables() const;
    QStringList readWriteExecutables() const;
    KPluginMetaData metaData() const;
    bool hasRequiredExecutables() const;
    bool isValid() const;

Q_SIGNALS:
    void enabledChanged();

private:
    static bool findExecutables(const QStringList &executables);

    bool m_enabled;
    KPluginMetaData m_metaData;
};

#endif
