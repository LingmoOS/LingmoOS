/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "../src/config.h"
#include "../src/screen.h"

#include <QObject>

namespace KScreen
{
class Output;
class ConfigOperation;

class TestPnp : public QObject
{
    Q_OBJECT

public:
    explicit TestPnp(bool monitor, QObject *parent = nullptr);
    ~TestPnp() override;

private Q_SLOTS:
    void init();
    void configReady(KScreen::ConfigOperation *op);
    void print();

private:
    ConfigPtr m_config;
    bool m_monitor;
};

} // namespace
