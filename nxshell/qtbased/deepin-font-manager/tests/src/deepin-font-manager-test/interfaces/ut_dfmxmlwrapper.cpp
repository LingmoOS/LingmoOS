// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmxmlwrapper.h"
#include "globaldef.h"

#include "../third-party/stub/stub.h"
#include "utils.h"
#include <gtest/gtest.h>

#include <QFile>

namespace {
class TestDFMXmlWrapper: public testing::Test
{
protected:
    void SetUp()
    {
        xmlWrapper = new DFMXmlWrapper();
    }
    void TearDown()
    {
        delete xmlWrapper;
    }
    // Some expensive resource shared by all tests.
    DFMXmlWrapper *xmlWrapper;
    QString fontConfigFilePath = "71-DeepInFontManager-Reject.conf";
public:
    bool stub_exists();
};
}

// 测试 DFMXmlWrapper::createFontConfigFile
TEST_F(TestDFMXmlWrapper, checkCreateFontConfigFile)
{

    do {
        /* 测试场景：配置文件不存在，生成配置文件 */
        if (QFile::exists(fontConfigFilePath)) {
            QFile::remove(fontConfigFilePath);
        }
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fontConfigFilePath));
        EXPECT_TRUE(QFile::exists(fontConfigFilePath));
    } while (false);

    do {
        /* 测试场景：配置文件存在 */
        if (!QFile::exists(fontConfigFilePath)) {
            xmlWrapper->createFontConfigFile(fontConfigFilePath);
        }
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fontConfigFilePath));
    } while (false);
}

// 测试 DFMXmlWrapper::getNodeByName
TEST_F(TestDFMXmlWrapper, checkGetNodeByName)
{
    do {
        /* 测试场景：获取根结点 */
        if (!QFile::exists(fontConfigFilePath)) {
            xmlWrapper->createFontConfigFile(fontConfigFilePath);
        }

        QDomDocument doc;
        QFile file(fontConfigFilePath);
        if (file.open(QIODevice::ReadOnly)) {
            doc.setContent(&file);
            file.close();
        }

        QDomElement rootDomElement = doc.documentElement();
        QDomElement nodeDomElement;
        EXPECT_TRUE(xmlWrapper->getNodeByName(rootDomElement, "fontconfig", nodeDomElement));
        EXPECT_TRUE("fontconfig" == rootDomElement.tagName());
    } while (false);

    do {
        /* 测试场景：获取子结点 */
        if (!QFile::exists(fontConfigFilePath)) {
            xmlWrapper->createFontConfigFile(fontConfigFilePath);
        }

        QDomDocument doc;
        QFile file(fontConfigFilePath);
        if (file.open(QIODevice::ReadOnly)) {
            doc.setContent(&file);
            file.close();
        }

        QDomElement rootDomElement = doc.documentElement();
        QDomElement nodeDomElement;
        EXPECT_TRUE(xmlWrapper->getNodeByName(rootDomElement, "patelt", nodeDomElement));
        EXPECT_TRUE("patelt" == nodeDomElement.tagName());
    } while (false);
}

// 测试 DFMXmlWrapper::addNodesWithTextList
TEST_F(TestDFMXmlWrapper, checkAddNodesWithTextList)
{
    QFile file("./doctest.xml");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        return ;
    }

    QString strdata = "<parentNode><nodeA><nodeB property=\"value\"><nodeC>nodeText</nodeC></nodeB></nodeA></parentNode>";
    QStringList nodelist;
    nodelist << "nodeC" ;
    file.write(strdata.toStdString().data());
    file.close();
    QMap<QString, QString> nodeatt;
    nodeatt.insert("1", "A");
    nodeatt.insert("1", "A");
    nodeatt.insert("1", "A");
    QList<QSTRING_MAP> nodeAttributeList;
    nodeAttributeList.append(nodeatt);
    EXPECT_TRUE(xmlWrapper->addNodesWithText("./doctest.xml", "parentNode", nodelist, nodeAttributeList, "nodeEnd"));
    file.remove();
}


bool TestDFMXmlWrapper::stub_exists()
{
    return false;
}

// 测试 DFMXmlWrapper::addPatternNodesWithTextList
TEST_F(TestDFMXmlWrapper, checkAddPatternNodesWithTextList)
{
    do {
        /* 测试场景：添加空节点 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::addPatternNodesWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_TRUE(result);
        QFile::remove(fileName);
    } while (false);

    do {
        /* 测试场景：添加不存在的节点 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QString parentNodeName = "accpetfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::addPatternNodesWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_FALSE(result);
        QFile::remove(fileName);
    } while (false);

    do {
        /* 测试场景：文件名为空 */
        QString fileName = "";

        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::addPatternNodesWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_FALSE(result);
    } while (false);

    do {
        /* 测试场景：添加有效节点数据 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));



        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;
        lastNodeTextList << "test_font.ttf";

        bool result = false;
        result = DFMXmlWrapper::addPatternNodesWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_TRUE(result);
        QFile::remove(fileName);
    } while (false);



    QString confilename = "tmp.config";
    QFile file(confilename);
    file.open(QIODevice::WriteOnly);
    file.write(confilename.toStdString().data());
    file.close();

    QFile::setPermissions(confilename, QFileDevice::ReadOwner);
    Stub st;
    st.set((bool (QFile::*)() const)ADDR(QFile, exists), ADDR(TestDFMXmlWrapper, stub_exists));
    xmlWrapper->createFontConfigFile(confilename);

    QFile::setPermissions(confilename, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QFile::remove(confilename);
}


// 测试 DFMXmlWrapper::deleteNodeWithTextList
TEST_F(TestDFMXmlWrapper, checkDeleteNodeWithTextList)
{
    do {
        /* 测试场景：删除空节点 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::deleteNodeWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_TRUE(result);
        QFile::remove(fileName);
    } while (false);

    do {
        /* 测试场景：删除不存在的节点 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QString parentNodeName = "accpetfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::deleteNodeWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_FALSE(result);
        QFile::remove(fileName);
    } while (false);

    do {
        /* 测试场景：文件名为空 */
        QString fileName = "";

        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;

        bool result = false;
        result = DFMXmlWrapper::deleteNodeWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_FALSE(result);
    } while (false);

    do {
        /* 测试场景：添加有效节点数据 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QString parentNodeName = "rejectfont";
        QStringList lastNodeTextList;
        lastNodeTextList << "test_font.ttf";

        bool result = false;
        result = DFMXmlWrapper::deleteNodeWithTextList(fileName, parentNodeName, lastNodeTextList);
        EXPECT_TRUE(result);
        QFile::remove(fileName);
    } while (false);
}

// 测试 DFMXmlWrapper::queryAllChildNodes_Text
TEST_F(TestDFMXmlWrapper, checkQueryAllChildNodesText)
{
    do {
        /* 测试场景：文件名为空 */
        QString fileName = "";
        QStringList strDisableFontPathList;
        bool result = false;
        result = DFMXmlWrapper::queryAllChildNodes_Text(fileName, "rejectfont", strDisableFontPathList);
        EXPECT_FALSE(result);
    } while (false);

    do {
        /* 测试场景：Node节点名非法 */
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fontConfigFilePath));
        QStringList strDisableFontPathList;
        bool result = false;
        result = DFMXmlWrapper::queryAllChildNodes_Text(fontConfigFilePath, "acceptfont", strDisableFontPathList);
        EXPECT_FALSE(result);
    } while (false);

    do {
        /* 测试场景：参数正确，正常场景 */
        QString fileName = "tmp.config";
        EXPECT_TRUE(xmlWrapper->createFontConfigFile(fileName));

        QStringList lastNodeTextList;
        lastNodeTextList << "test1.ttf" << "test2.ttf";
        DFMXmlWrapper::addPatternNodesWithTextList(fileName, "rejectfont", lastNodeTextList);

        QStringList strDisableFontPathList;
        bool result = false;
        result = DFMXmlWrapper::queryAllChildNodes_Text(fileName, "rejectfont", strDisableFontPathList);
        EXPECT_TRUE(result);
        QFile::remove(fileName);
    } while (false);
}

// 测试 DFMXmlWrapper::getFontConfigDisableFontPathList
TEST_F(TestDFMXmlWrapper, checkGetFontConfigDisableFontPathList)
{
    QStringList disableFontList;
    disableFontList = DFMXmlWrapper::getFontConfigDisableFontPathList();
    EXPECT_GE(disableFontList.size(), 0);
}
