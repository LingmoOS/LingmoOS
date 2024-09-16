// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "panel.h"

DS_USE_NAMESPACE

class ExamplePanel : public DPanel
{
    Q_OBJECT
public:
    explicit ExamplePanel(QObject *parent = nullptr);

    virtual bool load() override;

    virtual bool init() override;
};
