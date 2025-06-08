/*
    SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>

#include <kscreen/output.h>

namespace KScreen
{
class Config;
}
class Console : public QObject
{
    Q_OBJECT
public:
    explicit Console(const KScreen::ConfigPtr &config);
    ~Console() override;

public Q_SLOTS:
    void printConfig();
    void printJSONConfig();
    void printSerializations();
    void monitor();
    void monitorAndPrint();

private:
    KScreen::ConfigPtr m_config;
};

#endif
