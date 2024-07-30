/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QGuiApplication>
#include <QVariantList>

namespace PowerDevil
{
class Core;
}

using InhibitionInfo = QPair<QString, QString>;

class PowerDevilApp : public QGuiApplication
{
    Q_OBJECT
    Q_DISABLE_COPY(PowerDevilApp)

public:
    explicit PowerDevilApp(int &argc, char **argv);
    ~PowerDevilApp() override;

    void init();

private Q_SLOTS:
    void onCoreReady();

private:
    PowerDevil::Core *m_core;
    void showOsd();
};
