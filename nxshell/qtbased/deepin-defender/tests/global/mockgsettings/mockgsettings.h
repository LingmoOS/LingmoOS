// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCK_GSETTINGS_H
#define MOCK_GSETTINGS_H

#include "invokers/deepindefendersettingsInvoker.h"
#include "invokers/invokerinterface.h"

#include <QObject>
#include <QDBusMessage>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MockGSettingInterface : public QObject
    , public SettingsInvokerInterface
{
    Q_OBJECT
public:
    explicit MockGSettingInterface(const QByteArray &schema_id = QByteArray(), const QByteArray &path = QByteArray(), QObject *parent = nullptr)
        : QObject(parent)
    {
        Q_UNUSED(schema_id);
        Q_UNUSED(path);
        Q_UNUSED(parent);
    }

    virtual QVariant GetValue(const QString &key) const override
    {
        Q_UNUSED(key);
        return QVariant(QVariant::Type::Invalid);
    }

    virtual void SetValue(const QString &key, const QVariant &value) override
    {
        Q_UNUSED(key);
        Q_UNUSED(value);
    }
};

class MockGSettingsImpl : public MockGSettingInterface
{
    Q_OBJECT
public:
    MOCK_CONST_METHOD1(GetValue, QVariant(const QString &key));
    MOCK_METHOD2(SetValue, void(const QString &key, const QVariant &value));
};

#endif
