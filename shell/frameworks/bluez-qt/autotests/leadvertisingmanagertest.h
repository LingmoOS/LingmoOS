/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LEADVERTISINGMANAGERTEST_H
#define LEADVERTISINGMANAGERTEST_H

#include "adapter.h"
#include "leadvertisement.h"

class TestAdvertisement : public BluezQt::LEAdvertisement
{
    Q_OBJECT

public:
    using BluezQt::LEAdvertisement::LEAdvertisement;
    void release() override;

    // release
    bool m_releaseCalled = false;
};

class LEAdvertisingManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void releaseTest();

private:
    TestAdvertisement *m_advertisement;
    BluezQt::AdapterPtr m_adapter;
};

#endif
