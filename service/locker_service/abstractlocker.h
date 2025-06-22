/*
    SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
    SPDX-FileCopyrightText: 2002 Luboš Luňák <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2003 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2008 Chani Armitage <chanika@gmail.com>
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Bhushan Shah <bhush94@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QRasterWindow>

class GlobalAccel;

namespace ScreenLocker
{
class AbstractLocker;

class BackgroundWindow : public QRasterWindow
{
    Q_OBJECT
public:
    explicit BackgroundWindow(AbstractLocker *lock);
    ~BackgroundWindow() override;

    void emergencyShow();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    AbstractLocker *m_lock;
    bool m_greeterFailure = false;
};

class AbstractLocker : public QObject
{
    Q_OBJECT
public:
    explicit AbstractLocker(QObject *parent);
    ~AbstractLocker() override;

    virtual void showLockWindow() = 0;
    virtual void hideLockWindow() = 0;

    virtual void addAllowedWindow(quint32 window);

    void setGlobalAccel(GlobalAccel *ga)
    {
        m_globalAccel = ga;
    }

    void emergencyShow();

Q_SIGNALS:
    void userActivity();
    void lockWindowShown();

protected:
    virtual void stayOnTop() = 0;

    GlobalAccel *globalAccel()
    {
        return m_globalAccel;
    }
    QScopedPointer<BackgroundWindow> m_background;

private:
    GlobalAccel *m_globalAccel = nullptr;

    friend class BackgroundWindow;
};

}
