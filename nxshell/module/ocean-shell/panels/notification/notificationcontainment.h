// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "containment.h"

DS_BEGIN_NAMESPACE

class NotificationContainment : public DContainment
{
    Q_OBJECT
public:
    explicit NotificationContainment(QObject *parent = nullptr);
    ~NotificationContainment();

    virtual bool load() override;
};

DS_END_NAMESPACE
