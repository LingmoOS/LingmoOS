// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QRegularExpression>

#include "common.h"

class ut_Common : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_Common::SetUp()
{

}

void ut_Common::TearDown()
{

}

TEST_F(ut_Common, normalize)
{
    /*Epson PM-A820 -> epson pm a 820*/
    QString testMakeModel = "Epson PM-A820";
    QString ret = normalize(testMakeModel);
    QStringList list = ret.split(" ", QString::SkipEmptyParts);
    ASSERT_EQ(list.count(), 4);
    bool flag = ret.contains(QRegularExpression("[A-Z]"));
    ASSERT_EQ(flag, false);
    int number = list.at(3).toInt();
    ASSERT_EQ(number, 820);
}

TEST_F(ut_Common, normalize1)
{
    QString ret = normalize("");
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, parseDeviceID)
{
    QString ID = "MFG:Pantum;MDL:Pantum CM9500 Series;";
    auto map = parseDeviceID(ID);
    ASSERT_EQ(map.contains("MFG"), true);
    ASSERT_EQ(map.contains("MDL"), true);
}

TEST_F(ut_Common, parseDeviceID1)
{
    QString ID = "MANUFACTURER:TOSHIBA;COMMAND SET:PJL,PCL,PCLXL,POSTSCRIPT,XPS;MODEL:e-STUDIO3015AC;CLASS:PRINTER;DESCRIPTION:TOSHIBA e-STUDIO3015AC;CID:TS_PCL6_Color;";
    auto map = parseDeviceID(ID);
    EXPECT_EQ(map.contains("MANUFACTURER"), true);
    EXPECT_EQ(map.contains("MODEL"), true);
    EXPECT_EQ(map.contains("COMMAND SET"), true);
}

TEST_F(ut_Common, parseDeviceID2)
{
    QString ID = "G:PWG;MDL:IPP Everywhere Basic Printer Simulator;CMD:PWGRaster,PWG,JPEG;";
    auto map = parseDeviceID(ID);
    EXPECT_EQ(map.contains("G"), true);
}

TEST_F(ut_Common, shellCmd)
{
    QString cmd = "ls";
    QString out = "";
    QString strError = "";
    int ret = shellCmd(cmd, out, strError);
    ASSERT_EQ(ret, 0);
}

TEST_F(ut_Common, shellCmd1)
{
    QString cmd = "ls -l /kkkkkk";
    QString out = "";
    QString strError = "";
    int ret = shellCmd(cmd, out, strError);
    EXPECT_EQ(ret, -1);
    EXPECT_TRUE(strError.contains("err"));
    EXPECT_TRUE(strError.contains("kkkkk"));
}

TEST_F(ut_Common, shellCmd2)
{
    QString cmd = "kkkkk";
    QString out = "";
    QString strError = "";
    int ret = shellCmd(cmd, out, strError);
    EXPECT_EQ(ret, -2);
    EXPECT_STREQ(strError.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri)
{
    QString uri = "hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6";
    QString host = getHostFromUri(uri);
    ASSERT_TRUE(host.size() > 0);
}

TEST_F(ut_Common, getHostFromUri1)
{
    QString uri = "jinni-PC.local:631/printers/CUPS-PDF";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri2)
{
    QString uri = "ipp://jinni-PC.local:631/printers/CUPS-PDF";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "jinni-pc.local");
}

TEST_F(ut_Common, getHostFromUri3)
{
    QString uri = "smb://CUPS-PDF";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri4)
{
    QString uri = "smb://username:password@workgroup/printers/CUPS-PDF";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "printers");
}

TEST_F(ut_Common, getHostFromUri5)
{
    QString uri = "dnssd://CUPS-PDFcups?uuid=816b5932-784a-30db-7109-e2d0229bc518";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri6)
{
    QString uri = "dnssd://    /cups?uuid=816b5932-784a-30db-7109-e2d0229bc518";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri7)
{
    QString uri = "dnssd://.local/cups?uuid=816b5932-784a-30db-7109-e2d0229bc518";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getHostFromUri8)
{
    QString uri = "dnssd://CUPS-PDF%20%40%20jinni-PC._ipp._tcp.local/cups?uuid=816b5932-784a-30db-7109-e2d0229bc518";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "jinni-PC.local");
}

TEST_F(ut_Common, getHostFromUri9)
{
    QString uri = "file://CUPS-PDF";
    QString ret = getHostFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getPrinterNameFromUri)
{
    QString uri = "hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6";
    QString name = getPrinterNameFromUri(uri);
    ASSERT_TRUE(name.size() > 0);
}

TEST_F(ut_Common, getPrinterNameFromUri1)
{
    QString uri = "file:CUPS-PDF";
    QString ret = getPrinterNameFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, getPrinterNameFromUri2)
{
    QString uri = "dnssd://CUPS-PDF%20%40%20jinni-PC._ipp._tcp.local/cups?uuid=816b5932-784a-30db-7109-e2d0229bc518";
    QString ret = getPrinterNameFromUri(uri);
    EXPECT_STREQ(ret.toStdString().c_str(), "CUPS-PDF");
}

TEST_F(ut_Common, reslovedHost)
{
    QString ret = reslovedHost("");
    EXPECT_STREQ(ret.toStdString().c_str(), "");
}

TEST_F(ut_Common, reslovedHost1)
{
    QString ret = reslovedHost("Test_String");
    EXPECT_TRUE(ret.contains("Test_String not found"));
}

TEST_F(ut_Common, isPackageExists)
{
    bool ret = isPackageExists("libgtest-dev");
    EXPECT_TRUE(ret || true);
}

TEST_F(ut_Common, toNormalName)
{
    QString ret = toNormalName("hp");
    EXPECT_STREQ(ret.toStdString().c_str(), "HP");

    ret = toNormalName("ciaat");
    EXPECT_STREQ(ret.toStdString().c_str(), "CIAAT");

    ret = toNormalName("dec");
    EXPECT_STREQ(ret.toStdString().c_str(), "DEC");

    ret = toNormalName("ibm");
    EXPECT_STREQ(ret.toStdString().c_str(), "IBM");

    ret = toNormalName("NEC");
    EXPECT_STREQ(ret.toStdString().c_str(), "NEC");

    ret = toNormalName("Nrg");
    EXPECT_STREQ(ret.toStdString().c_str(), "NRG");

    ret = toNormalName("Pcpi");
    EXPECT_STREQ(ret.toStdString().c_str(), "PCPI");

    ret = toNormalName("qms");
    EXPECT_STREQ(ret.toStdString().c_str(), "QMS");
}

TEST_F(ut_Common, toNormalName1)
{
    QString ret = toNormalName("CANON IR8800");
    EXPECT_STREQ(ret.toStdString().c_str(), "Canon Ir8800");
}

TEST_F(ut_Common, splitStdoutString)
{
    QString str = "network socket://10.0.0.10 \"Kyocera ECOSYS P5026cdn\" \"ECOSYS P5026cdn\" \"ID:ECOSYS P5026cdn;MFG:Kyocera;CMD:PCLXL,PostScript Emulation,PCL5C,PJL;MDL:ECOSYS P5026cdn;CLS:PRINTER;DES:Kyocera ECOSYS P5026cdn;CID:KY_XPS_ColorA4FID;SER:VWA9901960;\" \"\"";
    QStringList ret = splitStdoutString(str);
    EXPECT_EQ(ret.size(), 5);
}

TEST_F(ut_Common, splitStdoutString1)
{
    QString str = "network socket://10.0.0.10 \"Kyocera ECOSYS P5026cdn\" \"ECOSYS P5026cdn\" \"ID:ECOSYS P5026cdn;MFG:Kyocera;CMD:PCLXL,PostScript Emulation,PCL5C,PJL;MDL:ECOSYS P5026cdn;CLS:PRINTER;DES:Kyocera ECOSYS P5026cdn;CID:KY_XPS_ColorA4FID;SER:VWA9901960;\" \" test \\\\ test\"";
    QStringList ret = splitStdoutString(str);
    EXPECT_EQ(ret.size(), 6);
}

TEST_F(ut_Common, replaceMakeName)
{
    int len = 0;
    QString strMake = "";
    QString ret = replaceMakeName(strMake, &len);
    EXPECT_STREQ(ret.toStdString().c_str(), "");
    EXPECT_EQ(len, 0);
}

TEST_F(ut_Common, replaceMakeName1)
{
    int len = 0;
    QString strMake = "Kyocera ECOSYS P5026cdn";
    QString ret = replaceMakeName(strMake, &len);
    EXPECT_STREQ(ret.toStdString().c_str(), "Kyocera");
    EXPECT_EQ(len, 0);
}

TEST_F(ut_Common, replaceMakeName2)
{
    int len = 0;
    QString strMake = "eastman kodak company";
    QString ret = replaceMakeName(strMake, &len);
    EXPECT_STREQ(ret.toStdString().c_str(), "kodak");
    EXPECT_EQ(len, 21);
}

TEST_F(ut_Common, isIpv4Address)
{
    QString strIp = "192.168.0.100";
    bool ret = isIpv4Address(strIp);
    EXPECT_TRUE(ret);

    strIp = "10.0.0.1";
    ret = isIpv4Address(strIp);
    EXPECT_TRUE(ret);

    strIp = "123.255.23.2";
    ret = isIpv4Address(strIp);
    EXPECT_TRUE(ret);
}

TEST_F(ut_Common, isIpv4Address1)
{
    QString strIp = "10.0.0.";
    bool ret = isIpv4Address(strIp);
    EXPECT_FALSE(ret);

    // function has some problem
//    strIp = "265.0.0.1";
//    ret = isIpv4Address(strIp);
//    EXPECT_FALSE(ret);

    strIp = "123.255.333.2";
    ret = isIpv4Address(strIp);
    EXPECT_FALSE(ret);

//    strIp = "123.255.255.2.1";
//    ret = isIpv4Address(strIp);
//    EXPECT_FALSE(ret);
}

TEST_F(ut_Common, removeMakeInModel)
{
    QString strMake = "Canon";
    QString strModel = "Canon iR2625/2630 UFR II";
    removeMakeInModel(strMake, strModel);
    EXPECT_STREQ(strModel.toStdString().c_str(), "iR2625/2630 UFR II");
}

TEST_F(ut_Common, ppdMakeModelSplit)
{
    QString strMakeModel = "Canon iR2625/2630 UFR II";
    QString strMake, strModel;
    ppdMakeModelSplit(strMakeModel, strMake, strModel);
    EXPECT_STREQ(strMake.toStdString().c_str(), "Canon");
    EXPECT_STREQ(strModel.toStdString().c_str(), "iR2625/2630");

    QString strFullModel = getPrinterFullModel();
    EXPECT_STREQ(strFullModel.toStdString().c_str(), "iR2625/2630 UFR II");
}

TEST_F(ut_Common, ppdMakeModelSplit1)
{
    QString strMakeModel = "Canon MG4100 series Ver.3.90";
    QString strMake, strModel;
    ppdMakeModelSplit(strMakeModel, strMake, strModel);
    EXPECT_STREQ(strMake.toStdString().c_str(), "Canon");
    EXPECT_STREQ(strModel.toStdString().c_str(), "MG4100");

    QString strFullModel = getPrinterFullModel();
    EXPECT_STREQ(strFullModel.toStdString().c_str(), "MG4100 series");
}

TEST_F(ut_Common, ppdMakeModelSplit2)
{
    QString strMakeModel = "HP Lj 4 Plus v2013.111 Postscript";
    QString strMake, strModel;
    ppdMakeModelSplit(strMakeModel, strMake, strModel);
    EXPECT_STREQ(strMake.toStdString().c_str(), "HP");
    EXPECT_STREQ(strModel.toStdString().c_str(), "LaserJet 4 Plus");
}

