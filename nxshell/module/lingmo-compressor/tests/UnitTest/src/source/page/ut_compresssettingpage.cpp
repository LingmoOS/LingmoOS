// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compresssettingpage.h"
#include "customwidget.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <DApplicationHelper>

#include <gtest/gtest.h>
#include <QTest>
#include <QAction>

bool g_checkFileNameVaild_result = false;
bool g_checkCompressOptionValid_result = false;
/*******************************函数打桩************************************/
// 对CompressSettingPage的setDefaultName进行打桩
void setDefaultName_stub(QString name)
{
    Q_UNUSED(name)
    return;
}

// 对CompressSettingPage的showWarningDialog进行打桩
void showWarningDialog_stub(const QString &msg, const QString &strToolTip)
{
    Q_UNUSED(msg)
    Q_UNUSED(strToolTip)
    return;
}

// 对CompressSettingPage的checkFileNameVaild进行打桩
bool checkFileNameVaild_stub(const QString strText)
{
    Q_UNUSED(strText)
    return g_checkFileNameVaild_result;
}

// 对CompressSettingPage的checkCompressOptionValid进行打桩
bool checkCompressOptionValid_stub()
{
    return g_checkCompressOptionValid_result;
}
/*******************************函数打桩************************************/


// 测试TypeLabel
class UT_TypeLabel : public ::testing::Test
{
public:
    UT_TypeLabel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TypeLabel;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    TypeLabel *m_tester;
};

TEST_F(UT_TypeLabel, initTest)
{

}

TEST_F(UT_TypeLabel, test_mousePressEvent)
{
    QTest::mousePress(m_tester, Qt::LeftButton);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
}


// 测试TypeLabel
class UT_CompressSettingPage : public ::testing::Test
{
public:
    UT_CompressSettingPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressSettingPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressSettingPage *m_tester;
};

TEST_F(UT_CompressSettingPage, initTest)
{

}

TEST_F(UT_CompressSettingPage, test_setFileSize_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles;
    m_tester->setFileSize(listFiles, 0);
}

TEST_F(UT_CompressSettingPage, test_setFileSize_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "path");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "path");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "path");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "path/";
    m_tester->setFileSize(listFiles, 0);
    EXPECT_EQ(m_tester->m_pSavePathEdt->text(), "path");
}

TEST_F(UT_CompressSettingPage, test_setFileSize_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "1");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1/1.txt");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "1/1.txt";
    m_tester->setFileSize(listFiles, 0);
    EXPECT_EQ(m_tester->m_pSavePathEdt->text(), "1");
}

TEST_F(UT_CompressSettingPage, test_setFileSize_004)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_path(stub, "1");
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_completeBaseName(stub, "1/1.txt");
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    stub.set(ADDR(CompressSettingPage, setDefaultName), setDefaultName_stub);

    QStringList listFiles = QStringList() << "1/1.txt" << "2/2.txt";
    m_tester->setFileSize(listFiles, 0);
    EXPECT_EQ(m_tester->m_pSavePathEdt->text(), "1");
}

TEST_F(UT_CompressSettingPage, test_refreshMenu)
{
    Stub stub;
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);

    m_tester->refreshMenu();
    EXPECT_EQ(m_tester->m_listSupportedMimeTypes.isEmpty(), false);
}

TEST_F(UT_CompressSettingPage, test_getClickLbl)
{
    EXPECT_EQ(m_tester->getClickLbl(), m_tester->m_pClickLbl);
}

TEST_F(UT_CompressSettingPage, test_getCompressBtn)
{
    EXPECT_EQ(m_tester->getCompressBtn(), m_tester->m_pCompressBtn);
}

TEST_F(UT_CompressSettingPage, test_getComment_001)
{
    m_tester->m_pCommentEdt->setPlainText("hhh");
    m_tester->m_pCommentEdt->setEnabled(true);
    EXPECT_EQ(m_tester->getComment(), "hhh");
}

TEST_F(UT_CompressSettingPage, test_getComment_002)
{
    m_tester->m_pCommentEdt->setPlainText("hhh");
    m_tester->m_pCommentEdt->setEnabled(false);
    EXPECT_EQ(m_tester->getComment().isEmpty(), true);
}

TEST_F(UT_CompressSettingPage, test_getComment_003)
{
    EXPECT_EQ(m_tester->getComment().isEmpty(), true);
}

TEST_F(UT_CompressSettingPage, test_setTypeImage)
{
    EXPECT_EQ(m_tester->checkFileNameVaild("1.zip"), true);
}

TEST_F(UT_CompressSettingPage, test_checkFileNameVaild_001)
{
    EXPECT_EQ(m_tester->checkFileNameVaild(""), false);
}

TEST_F(UT_CompressSettingPage, test_checkFileNameVaild_002)
{
    EXPECT_EQ(m_tester->checkFileNameVaild("hdfjshfjksdhfkjshaflkashdfkjshfdksjdhfjhsadfrsahfsjahfdhsakjdfhsadifhasdhfasjkdhfjksadfhkjsadfhjksadhfkjsadhfjksadhfosdhfkjsadhfkjsadhfjklashdfkjsadhfkjasdhfkjasdhfkjlsahdfjksadhofusadkjfhhdfjshfjksdhfkjshaflkashdfkjshfdksjdhfjhsadfrsahfsjahfdhsakjdfhsadifhasdhfa.zip"), false);
}

TEST_F(UT_CompressSettingPage, test_checkFileNameVaild_h003)
{
    EXPECT_EQ(m_tester->checkFileNameVaild("1/1.zip"), false);
}

TEST_F(UT_CompressSettingPage, test_setEncryptedEnabled)
{
    m_tester->setEncryptedEnabled(false);
    EXPECT_EQ(m_tester->m_pPasswordEdt->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_setListEncryptionEnabled)
{
    m_tester->setListEncryptionEnabled(false);
    EXPECT_EQ(m_tester->m_pListEncryptionLbl->isEnabled(), false);
    EXPECT_EQ(m_tester->m_pListEncryptionBtn->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_setSplitEnabled_001)
{
    m_tester->setSplitEnabled(false);
    EXPECT_EQ(m_tester->m_pSplitValueEdt->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_setSplitEnabled_002)
{
    m_tester->m_pSplitCkb->setCheckState(Qt::Checked);
    m_tester->setSplitEnabled(true);
    EXPECT_EQ(m_tester->m_pSplitValueEdt->isEnabled(), true);
}

TEST_F(UT_CompressSettingPage, test_refreshCompressLevel_001)
{
    m_tester->refreshCompressLevel("tar");
    EXPECT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_refreshCompressLevel_002)
{
    m_tester->refreshCompressLevel("tar.Z");
    EXPECT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_refreshCompressLevel_003)
{
    m_tester->refreshCompressLevel("zip");
    EXPECT_EQ(m_tester->m_pCompressLevelLbl->isEnabled(), true);
}

TEST_F(UT_CompressSettingPage, test_setCommentEnabled)
{
    m_tester->m_pCommentEdt->setPlainText("aaa");
    m_tester->setCommentEnabled(false);
    EXPECT_EQ(m_tester->m_pCommentEdt->toPlainText().isEmpty(), true);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_001)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    m_tester->m_pSplitCkb->setChecked(false);
    EXPECT_EQ(m_tester->checkCompressOptionValid(), true);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_002)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pFileNameEdt->setText("");
    EXPECT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_003)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText(" ");
    EXPECT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_004)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    EXPECT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_005)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    EXPECT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(UT_CompressSettingPage, test_checkCompressOptionValid_006)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);

    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("1/");
    m_tester->m_pSplitCkb->setChecked(true);
    m_tester->m_pSplitValueEdt->setValue(0.000000000000001);
    m_tester->m_strMimeType = "7z";
    EXPECT_EQ(m_tester->checkCompressOptionValid(), false);
}

TEST_F(UT_CompressSettingPage, test_checkFile_001)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    EXPECT_EQ(m_tester->checkFile(""), true);
}

TEST_F(UT_CompressSettingPage, test_checkFile_002)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, true);

    EXPECT_EQ(m_tester->checkFile(""), false);
}

TEST_F(UT_CompressSettingPage, test_checkFile_003)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, false);

    EXPECT_EQ(m_tester->checkFile(""), false);
}

TEST_F(UT_CompressSettingPage, test_checkFile_004)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);

    EXPECT_EQ(m_tester->checkFile(""), false);
}

TEST_F(UT_CompressSettingPage, test_checkFile_005)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);

    EXPECT_EQ(m_tester->checkFile(""), true);
}

TEST_F(UT_CompressSettingPage, test_showWarningDialog)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    EXPECT_EQ(m_tester->showWarningDialog("", ""), 0);
}

TEST_F(UT_CompressSettingPage, test_setDefaultName)
{
    m_tester->setDefaultName("/home");
    EXPECT_EQ(m_tester->m_pFileNameEdt->lineEdit()->text(), "/home");
}

TEST_F(UT_CompressSettingPage, test_slotTypeChanged_001)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("tar.7z");
    m_tester->slotTypeChanged(pAction);
    EXPECT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_slotTypeChanged_002)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("7z");
    m_tester->slotTypeChanged(pAction);
    EXPECT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_slotTypeChanged_003)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("zip");
    m_tester->slotTypeChanged(pAction);
    EXPECT_EQ(m_tester->m_pSplitCkb->isEnabled(), true);
}

TEST_F(UT_CompressSettingPage, test_slotTypeChanged_004)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText("tar");
    m_tester->slotTypeChanged(pAction);
    EXPECT_EQ(m_tester->m_pCommentLbl->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_slotTypeChanged_005)
{
    m_tester->m_strMimeType = "123";
    m_tester->slotTypeChanged(nullptr);
    EXPECT_EQ(m_tester->m_strMimeType, "123");
}

TEST_F(UT_CompressSettingPage, test_slotFileNameChanged_001)
{
    m_tester->m_pFileNameEdt->setText("");
    m_tester->slotRefreshFileNameEdit();
    EXPECT_EQ(m_tester->m_pFileNameEdt->text(), "");
    DPalette plt = DApplicationHelper::instance()->palette(m_tester->m_pFileNameEdt);
    EXPECT_EQ(plt.brush(DPalette::Text).color(), plt.color(DPalette::WindowText));
}

TEST_F(UT_CompressSettingPage, test_slotFileNameChanged_002)
{
    g_checkFileNameVaild_result = false;
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkFileNameVaild), checkFileNameVaild_stub);
    m_tester->m_pFileNameEdt->setText("hh");
    m_tester->slotRefreshFileNameEdit();
    DPalette plt = DApplicationHelper::instance()->palette(m_tester->m_pFileNameEdt);
    EXPECT_EQ(plt.brush(DPalette::Text).color(), plt.color(DPalette::TextWarning));
}

TEST_F(UT_CompressSettingPage, test_slotFileNameChanged_003)
{
    g_checkFileNameVaild_result = true;
    Stub stub;
    stub.set(ADDR(CompressSettingPage, checkFileNameVaild), checkFileNameVaild_stub);
    m_tester->m_pFileNameEdt->setText("hh");
    m_tester->slotRefreshFileNameEdit();
    DPalette plt = DApplicationHelper::instance()->palette(m_tester->m_pFileNameEdt);
    EXPECT_EQ(plt.brush(DPalette::Text).color(), plt.color(DPalette::WindowText));
}

TEST_F(UT_CompressSettingPage, test_slotAdvancedEnabled_001)
{
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotAdvancedEnabled(true);
    EXPECT_EQ(m_tester->m_pSplitValueEdt->value(), 1.0);
}

TEST_F(UT_CompressSettingPage, test_slotAdvancedEnabled_002)
{
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotAdvancedEnabled(false);
    EXPECT_EQ(m_tester->m_pSplitValueEdt->value(), 0.0);
}

TEST_F(UT_CompressSettingPage, test_slotSplitEdtEnabled_003)
{
    m_tester->m_pSplitCkb->setChecked(false);
    m_tester->m_pSplitValueEdt->setValue(1.0);
    m_tester->slotSplitEdtEnabled();
    EXPECT_EQ(m_tester->m_pSplitValueEdt->value(), 0.0);
}

TEST_F(UT_CompressSettingPage, test_slotSplitEdtEnabled_004)
{
    m_tester->m_pSplitCkb->setChecked(true);
    m_tester->m_strMimeType = "zip";
    m_tester->slotSplitEdtEnabled();
    EXPECT_EQ(m_tester->m_pCommentEdt->isEnabled(), false);
}

TEST_F(UT_CompressSettingPage, test_slotCompressClicked_001)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);

    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->m_pFileNameEdt->setText("1");
    m_tester->m_pSavePathEdt->setText("/home");
    m_tester->m_listFiles << "/home/1.zip";
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, test_slotCompressClicked_002)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = false;
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, testslotCompressClicked_003)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("tar");
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, testslotCompressClicked_004)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, testslotCompressClicked_005)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    QFileStub::stub_QFile_remove(stub, true);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, testslotCompressClicked_006)
{
    Stub stub;
    stub.set(ADDR(CompressSettingPage, showWarningDialog), showWarningDialog_stub);
    stub.set(ADDR(CompressSettingPage, checkCompressOptionValid), checkCompressOptionValid_stub);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    QFileStub::stub_QFile_remove(stub, false);
    g_checkCompressOptionValid_result = true;
    m_tester->m_pCompressTypeLbl->setText("zip");
    m_tester->slotCompressClicked();
}

TEST_F(UT_CompressSettingPage, test_slotCommentTextChanged)
{
    QString str;
    QString strTemp;
    for (int i = 0; i < 10001; ++i) {
        str += "1";
        if (i < 10000)
            strTemp += "1";
    }
    m_tester->m_pCommentEdt->setPlainText(str);
    m_tester->slotCommentTextChanged();
    EXPECT_EQ(m_tester->m_pCommentEdt->toPlainText(), strTemp);
}

TEST_F(UT_CompressSettingPage, test_slotPasswordChanged)
{
    m_tester->m_pPasswordEdt->setText("123《》");
    m_tester->slotPasswordChanged();
    EXPECT_EQ(m_tester->m_pPasswordEdt->lineEdit()->text(), "123");
}

TEST_F(UT_CompressSettingPage, test_slotEchoModeChanged)
{
    m_tester->slotEchoModeChanged(false);
    EXPECT_EQ(m_tester->m_pPasswordEdt->lineEdit()->testAttribute(Qt::WA_InputMethodEnabled), false);
}
