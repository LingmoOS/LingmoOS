/*
 *   SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <Module.h>

class Config : public Module
{
    Q_OBJECT

public:
    explicit Config(QObject *parent = nullptr);
    ~Config() override;

Q_SIGNALS:
    void pluginConfigChanged();
    void mainConfigChanged();

private:
    D_PTR;
};
