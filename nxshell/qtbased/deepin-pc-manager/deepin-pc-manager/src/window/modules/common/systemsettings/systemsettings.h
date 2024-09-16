// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusVariant>
#include <QObject>

class DBusInvokerInterface;

namespace def {
class SystemSettings : public QObject
{
    Q_OBJECT
public:
    explicit SystemSettings(QObject *parent = nullptr);
    virtual ~SystemSettings() override;

    bool setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key);

Q_SIGNALS:
    void valueChanged(const QString &key, const QVariant &value);

public Q_SLOTS:
    void onValueChanged(const QString &key, const QDBusVariant &value);

private:
    DBusInvokerInterface *m_netMonitorDBusInvoker;
};
} // namespace def
