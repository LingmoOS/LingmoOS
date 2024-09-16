// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QRegularExpression>

#include "vendor/zdrivermanager.h"
#include "vendor/printerservice.h"

class ut_zdrivermanager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DriverManager *m_driverManager;
    DriverSearcher *m_pdriverSearcher;
};

void ut_zdrivermanager::SetUp()
{
}

void ut_zdrivermanager::TearDown()
{
}
#if 0
TEST_F(ut_zdrivermanager, startSearch)
{
    /* uri: socket://10.20.41.241,
     * class:network,
     * info:Canon iR2625 04.01,
     * name:Canon iR2625/2630 UFR II,
     * makemodel:Canon iR2625/2630 UFR II,
     * id:MFG:Canon;MDL:iR2625/2630 UFR II;CLS:PRINTER;DES:Canon iR2625/2630;CID:CA_XPS_OIP;CMD:LIPSLX,CPCA;
     */

    DriverManager *m_driverManager = DriverManager::getInstance();
    TDeviceInfo device;
    device.uriList << "socket://10.0.0.1";
    device.strClass = "network";
    device.strInfo = "Canon OO9999";
    device.strName = "Canon OO9999 UFR II";
    device.strMakeAndModel = "Canon OO9999 UFR II";
    device.strDeviceId = "MFG:Canon;MDL:OO9999 UFR II;CLS:PRINTER;DES:Canon OO9999;CID:CA_XPS_OIP;CMD:LIPSLX,CPCA;";

    DriverSearcher *m_pdriverSearcher = m_driverManager->createSearcher(device);
    m_pdriverSearcher->startSearch();
    qInfo() << " 000000 : " << m_pdriverSearcher->getDrivers();
    EXPECT_EQ((m_pdriverSearcher->getDrivers()).size(), 0);
}

TEST_F(ut_zdrivermanager, startSearch1)
{
    /* uri: socket://10.20.41.241,
     * class:network,
     * info:Canon iR2625 04.01,
     * name:Canon iR2625/2630 UFR II,
     * makemodel:Canon iR2625/2630 UFR II,
     * id:MFG:Canon;MDL:iR2625/2630 UFR II;CLS:PRINTER;DES:Canon iR2625/2630;CID:CA_XPS_OIP;CMD:LIPSLX,CPCA;
     */

    DriverManager *m_driverManager = DriverManager::getInstance();
    TDeviceInfo device;
    device.uriList << "dnssd://Canon iR2625 uos-PC._ipp._tcp.local/cups?uuid=3efd5067-e31c-357d-7886-dc20cf22c729";
    device.strClass = "network";
    device.strInfo = "Canon iR2625 04.01";
    device.strName = "Canon iR2625/2630 UFR II";
    device.strMakeAndModel = "Canon iR2625/2630 UFR II";
    device.strDeviceId = "MFG:Canon;MDL:iR2625/2630 UFR II;CLS:PRINTER;DES:Canon iR2625/2630;CID:CA_XPS_OIP;CMD:LIPSLX,CPCA;";

    DriverSearcher *m_pdriverSearcher = m_driverManager->createSearcher(device);
    m_pdriverSearcher->startSearch();
    EXPECT_EQ((m_pdriverSearcher->getDrivers()).size(), 1);
}

TEST_F(ut_zdrivermanager, startSearch2)
{
    /* uri: cups-pdf:/,
     * class:file,
     * info:CUPS-PDF,
     * name:CUPS-PDF,
     * makemodel:Virtual PDF Printer,
     * id:MFG:Generic;MDL:CUPS-PDF Printer;DES:Generic CUPS-PDF Printer;CLS:PRINTER;CMD:POSTSCRIPT;
     */

    DriverManager *m_driverManager = DriverManager::getInstance();
    TDeviceInfo device;
    device.uriList << "uri: cups-pdf:/";
    device.strClass = "file";
    device.strInfo = "CUPS-PDF";
    device.strName = "CUPS-PDF";
    device.strMakeAndModel = "Virtual PDF Printer";
    device.strDeviceId = "MFG:Generic;MDL:CUPS-PDF Printer;DES:Generic CUPS-PDF Printer;CLS:PRINTER;CMD:POSTSCRIPT";

    DriverSearcher *m_pdriverSearcher = m_driverManager->createSearcher(device);
    m_pdriverSearcher->startSearch();
//    EXPECT_TRUE((m_pdriverSearcher->getDrivers()).size() > 0);
}

/*
TEST_F(ut_zdrivermanager, feedbackResult)
{
    QString reason = "ut test";
    QString feedback = "please ignore ut";
    auto server = g_printerServer->feedbackResult(111111, true, reason, feedback, 111111);

//    EXPECT_TRUE(server != nullptr);
}

TEST_F(ut_zdrivermanager, searchDriver)
{
    auto server = g_printerServer->searchDriver(11111111);
//    EXPECT_TRUE(server != nullptr);
}
*/
#endif

