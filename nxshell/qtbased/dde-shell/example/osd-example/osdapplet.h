// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

DS_USE_NAMESPACE

class OsdApplet : public DApplet
{
    Q_OBJECT
public:
    explicit OsdApplet(QObject *parent = nullptr);

    Q_INVOKABLE void sendOsd(const QString &type);
};
