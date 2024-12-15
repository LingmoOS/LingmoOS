// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "containment.h"

DS_USE_NAMESPACE

class ExampleAppletProxy : public QObject
{
    Q_OBJECT
public:
    ExampleAppletProxy(QObject *parent = nullptr)
        : QObject(parent)
    {

    }
    Q_INVOKABLE QString call(const QString &id)
    {
        return id + QString("-done");
    }
};

class ExampleContainment : public DContainment
{
    Q_OBJECT
public:
    explicit ExampleContainment(QObject *parent = nullptr);
    ~ExampleContainment();

    virtual bool load() override;
protected:
    virtual QObject *createProxyMeta() override;
private:
    DPluginMetaData targetPlugin() const;
};
