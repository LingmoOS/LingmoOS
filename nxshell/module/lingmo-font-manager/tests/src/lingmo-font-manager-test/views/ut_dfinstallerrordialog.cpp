// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/dfinstallerrordialog.h"
#include "views/dfinstallerrorlistview.h"
#include "views/dfinstallnormalwindow.h"
#include "dstyleoption.h"
#include "dfontinfomanager.h"
#include "utils.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QTest>
#include <QSignalSpy>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStylePainter>

#include <DApplication>
#include <DStyleHelper>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DStyleOptionButton>
#include <DVerticalLine>
#include <DCheckBox>
#include <DLog>

namespace {
class TestDFInstallErrorDialog: public testing::Test
{

protected:
    void SetUp()
    {
        installNormalWindow = new DFInstallNormalWindow();
        DFErrDialog = new DFInstallErrorDialog(installNormalWindow);

    }
    void TearDown()
    {
        delete installNormalWindow;
    }
    DFInstallErrorDialog *DFErrDialog;
    DFInstallNormalWindow *installNormalWindow;

};

DFontInfo  stub_getFontInfoError(const QString &filePath)
{
//    qDebug() << filePath << endl;
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isError = true;
    f.fullname = "a";
    f.filePath = "b";

    return f;
}

DFontInfo  stub_getFontInfoInstalled(const QString &filePath)
{
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isInstalled = true;
    f.fullname = "a";
    f.filePath = "b";

    return f;
}

DFontInfo  stub_getFontInfo(const QString &filePath)
{
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isInstalled = false;
    f.isError = false;
    f.fullname = "a";
    f.filePath = "b";

    return f;
}

bool stub_isSystemFont(DFontInfo &f)
{
    Q_UNUSED(f)

    return true;

}

bool stub_isSystemFontFalse(DFontInfo &f)
{
    Q_UNUSED(f)

    return false;

}
}

TEST_F(TestDFInstallErrorDialog, checkGetErrorFontCheckedCount)
{
    DFErrDialog->m_installErrorListView->m_errorListSourceModel = new QStandardItemModel(2, 1);
    int count = DFErrDialog->getErrorFontCheckedCount();
    EXPECT_TRUE(count == 0);
}

TEST_F(TestDFInstallErrorDialog, checkInitDataError)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoError);
    DFErrDialog->m_errorInstallFiles.append("first");
    DFErrDialog->initData();
    EXPECT_TRUE(DFErrDialog->m_installErrorFontModelList.size() == 1);
}

TEST_F(TestDFInstallErrorDialog, checkInitDataInstalled)
{

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoInstalled);
    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFontFalse);
    DFErrDialog->m_errorInstallFiles.append("first");
    DFErrDialog->initData();
    EXPECT_TRUE(DFErrDialog->m_installErrorFontModelList.size() == 1);
}

TEST_F(TestDFInstallErrorDialog, checkInitDataSystem)
{

    Stub s;
    s.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFont);
    Stub s2;
    s2.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfo);
    DFErrDialog->m_errorInstallFiles.append("first");
    DFErrDialog->initData();
    EXPECT_TRUE(DFErrDialog->m_installErrorFontModelList.size() == 1);
}

TEST_F(TestDFInstallErrorDialog, checkAddDataError)
{
    QStringList list;
    QStringList list2 = QStringList();
    list << "a";
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoError);

    DFErrDialog->addData(list, list2, list2, list2);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_errorFontlist.size() == 1);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_installErrorFontModelList.size() == 1);
}

TEST_F(TestDFInstallErrorDialog, checkAddDataInstalled)
{
    QStringList list;
    QStringList list2 = QStringList();
    list << "a";
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoInstalled);

    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFontFalse);

    DFErrDialog->addData(list, list2, list2, list2);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_errorFontlist.size() == 1);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_installErrorFontModelList.size() == 1);
}


TEST_F(TestDFInstallErrorDialog, checkAddDataSystem)
{
    QStringList list;
    QStringList list2 = QStringList();
    list << "a";

    Stub s;
    s.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFont);

    Stub s2;
    s2.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfo);

    DFErrDialog->addData(list, list2, list2, list2);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_errorFontlist.size() == 1);
    EXPECT_TRUE(DFErrDialog->m_installErrorListView->m_installErrorFontModelList.size() == 1);
}

//onListItemsClicked
TEST_F(TestDFInstallErrorDialog, checkOnListItemClicked)
{
    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    DFErrDialog->m_installErrorListView->m_installErrorFontModelList.append(model1);
    DFErrDialog->m_installErrorListView->initErrorListData();

    QModelIndex index = DFErrDialog->m_installErrorListView->getErrorListSourceModel()->index(0, 0);

    DFErrDialog->onListItemClicked(index);
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(DFErrDialog->m_installErrorListView->getErrorListSourceModel()->data(index));
    EXPECT_TRUE(itemModel.bChecked);
}

//onListItemsClicked
TEST_F(TestDFInstallErrorDialog, checkOnListItemsClicked)
{
    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    DFErrDialog->m_installErrorListView->m_installErrorFontModelList.append(model1);
    DFErrDialog->m_installErrorListView->initErrorListData();

    QModelIndex index = DFErrDialog->m_installErrorListView->getErrorListSourceModel()->index(0, 0);
    QModelIndexList list;
    list << index;

    DFErrDialog->onListItemsClicked(list);
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(DFErrDialog->m_installErrorListView->getErrorListSourceModel()->data(index));
    EXPECT_TRUE(itemModel.bChecked);

}

TEST_F(TestDFInstallErrorDialog, checkOnControlButtonClicked_firstbutton)
{
    QSignalSpy spy(DFErrDialog, SIGNAL(onCancelInstall()));
    QSignalSpy spy2(DFErrDialog->m_signalManager, SIGNAL(setSpliteWidgetScrollEnable(bool)));

    DFErrDialog->onControlButtonClicked(0);

    //oncancelinstall信号多发送了一次 导致错误
    EXPECT_TRUE(spy.count() == 1);
    EXPECT_TRUE(spy2.count() == 1);

}

TEST_F(TestDFInstallErrorDialog, checkOnControlButtonClicked_secondbutton)
{
    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    DFErrDialog->m_installErrorListView->m_installErrorFontModelList.append(model1);
    DFErrDialog->m_installErrorListView->initErrorListData();

    QSignalSpy spy(DFErrDialog, SIGNAL(onContinueInstall(QStringList)));
    DFErrDialog->onControlButtonClicked(1);

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFInstallErrorDialog, checkKeyPressEvent)
{
    QSignalSpy spy(DFErrDialog, SIGNAL(onCancelInstall()));
    QTest::keyPress(DFErrDialog, Qt::Key_Escape);
    EXPECT_TRUE(spy.count() == 1);

    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    DFErrDialog->m_installErrorListView->m_installErrorFontModelList.append(model1);

    model1.bChecked = true;
    DFErrDialog->m_installErrorListView->m_installErrorFontModelList.append(model1);
    DFErrDialog->m_installErrorListView->initErrorListData();

    QTest::keyPress(DFErrDialog, Qt::Key_Return);
    DFErrDialog->m_installErrorListView->setFocus();
    QTest::keyPress(DFErrDialog, Qt::Key_Return);

    QTest::keyPress(DFErrDialog, Qt::Key_Home);
    QTest::keyPress(DFErrDialog, Qt::Key_End);
    QTest::keyPress(DFErrDialog, Qt::Key_PageUp);
    QTest::keyPress(DFErrDialog, Qt::Key_PageDown);

}







