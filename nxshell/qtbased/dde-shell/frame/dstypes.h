// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include <QObject>

DS_BEGIN_NAMESPACE

class Types : public QObject
{
    Q_OBJECT
public:
    explicit Types(QObject *parent = nullptr);
};

DS_END_NAMESPACE
