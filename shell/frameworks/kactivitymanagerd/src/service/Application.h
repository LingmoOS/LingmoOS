/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Qt
#include <QApplication>

// Utils
#include <utils/d_ptr.h>

class Resources;
class Activities;
class Features;

/**
 * Main application object
 * This can *not* be a QGuiApplication because
 * we need KWindowSystem widgets.
 */
class Application : public QApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Application")

public:
    Application(int &argc, char **argv);
    ~Application() override;

    virtual int newInstance();

    // static Application * self();
    // static void quit();

    Resources &resources() const;
    Activities &activities() const;
    Features &features() const;

public Q_SLOTS:
    void quit();
    QString serviceVersion() const;
    bool loadPlugin(const QString &plugin);
    QStringList loadedPlugins() const;

private Q_SLOTS:
    void init();
    void loadPlugins();

private:
    D_PTR;

    friend int main(int, char **);
};
