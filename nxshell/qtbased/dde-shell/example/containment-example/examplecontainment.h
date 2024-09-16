// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "containment.h"

DS_USE_NAMESPACE

class ExampleContainment : public DContainment
{
    Q_OBJECT
public:
    explicit ExampleContainment(QObject *parent = nullptr);
    ~ExampleContainment();

    virtual bool load() override;
private:
    DPluginMetaData targetPlugin() const;
};
