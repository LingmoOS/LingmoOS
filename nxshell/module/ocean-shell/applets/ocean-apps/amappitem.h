// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitem.h"
#include "applicationinterface.h"

namespace apps
{
class AMAppItem : public Application, public AppItem
{
    Q_OBJECT

public:
    explicit AMAppItem(const QDBusObjectPath &path, QObject *parent = nullptr);
    explicit AMAppItem(const QDBusObjectPath &path, const ObjectInterfaceMap &source, QObject *parent = nullptr);

    void launch(const QString &action = {}, const QStringList &fields = {}, const QVariantMap &options = {}) override;
    void setAutoStart(bool autoStart) override;
    void setOnDesktop(bool on) override;

private:
    QString getLocaleOrDefaultValue(const QStringMap &value, const QString &targetKey, const QString &fallbackKey);
    void updateActions(const QStringList &actions, const PropMap &actionName);

private Q_SLOTS:
    void onPropertyChanged(const QDBusMessage &msg);
};
}
