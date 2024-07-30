/*
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIATEST_H
#define MEDIATEST_H

#include "adapter.h"
#include "mediaendpoint.h"

class TestEndpoint : public BluezQt::MediaEndpoint
{
    Q_OBJECT

public:
    using BluezQt::MediaEndpoint::MediaEndpoint;
    void release() override;

    // release
    bool m_releaseCalled = false;
};

class MediaTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void setConfigurationTest();
    void selectConfigurationTest();
    void clearConfigurationTest();
    void releaseTest();

private:
    TestEndpoint *m_endpoint;
    BluezQt::AdapterPtr m_adapter;
};

#endif // MEDIATEST_H
