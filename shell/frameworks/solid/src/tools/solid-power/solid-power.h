/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_POWER_TOOL_H
#define SOLID_POWER_TOOL_H

#include <QObject>

class SolidPower : public QObject
{
    Q_OBJECT
public:
    explicit SolidPower(QObject *parent = nullptr);

    void show();
    void listen();
};

#endif // SOLID_POWER_TOOL_H
