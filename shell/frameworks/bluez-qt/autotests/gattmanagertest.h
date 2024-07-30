/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTMANAGERTEST_H
#define GATTMANAGERTEST_H

#include "adapter.h"
#include "gattapplication.h"
#include "gattcharacteristic.h"

class GattManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void readCharcTest();
    void writeCharcTest();

private:
    BluezQt::GattApplication *m_application;
    BluezQt::GattCharacteristic *m_characteristic;
    BluezQt::AdapterPtr m_adapter;
};

#endif
