/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef INPUTTEST_H
#define INPUTTEST_H

#include <QObject>

#include "bluezinput1_tst.h"

#include "device.h"
#include "input.h"
#include "manager.h"

class InputTest : public QObject
{
    Q_OBJECT

public:
    explicit InputTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void getPropertiesTest();

private:
    QString reconnectModeString(const BluezQt::InputPtr &input) const;

    struct InputUnit {
        BluezQt::DevicePtr device;
        org::bluez::Input1 *dbusInput;
    };

    BluezQt::Manager *m_manager;
    QList<InputUnit> m_units;
};

#endif // INPUTTEST_H
