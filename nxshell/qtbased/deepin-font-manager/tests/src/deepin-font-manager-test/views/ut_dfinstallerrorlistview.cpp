// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/dfinstallerrorlistview.h"

#include "globaldef.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"
#include "commonheaderfile.h"

#include <QTest>
#include <QSignalSpy>
#include <QPainter>
#include <QMouseEvent>
#include <QStandardItemModel>

#include <DLog>
#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DCheckBox>
#include <DFontSizeManager>


namespace {
class TestDFInstallErrorListView : public testing::Test
{

protected:
    void SetUp()
    {
        iEListview = new DFInstallErrorListView(QList<DFInstallErrorItemModel>());
    }
    void TearDown()
    {
        SAFE_DELETE_ELE(iEListview);
        SAFE_DELETE_ELE(w);
        SAFE_DELETE_ELE(p);
    }
    // Some expensive resource shared by all tests.
    DFInstallErrorListView *iEListview;
    QWidget *w = new QWidget;
    QPainter *p = new QPainter;
    QStyleOptionViewItem option;
};


}

DGuiApplicationHelper::ColorType stub_themeType1()
{
    return DGuiApplicationHelper::DarkType;
}

TEST_F(TestDFInstallErrorListView, checkPaint)
{
    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    iEListview->m_installErrorFontModelList.append(model1);

    model1.bChecked = true;
    iEListview->m_installErrorFontModelList.append(model1);

    model1.bSelectable = true;
    iEListview->m_installErrorFontModelList.append(model1);

    iEListview->initErrorListData();

    QModelIndex index ;
    index = iEListview->getErrorListSourceModel()->index(0, 0);
    iEListview->m_errorListItemDelegate->paint(p, option, index);

    index = iEListview->getErrorListSourceModel()->index(1, 0);
    iEListview->m_errorListItemDelegate->paint(p, option, index);

    index = iEListview->getErrorListSourceModel()->index(2, 0);
    iEListview->m_IsTabFocus = true;
    option.state.setFlag(QStyle::State_Selected);
    iEListview->m_errorListItemDelegate->paint(p, option, index);
    iEListview->m_IsTabFocus = false;
    iEListview->m_errorListItemDelegate->paint(p, option, index);

//    iEListview->selectAll();
    index = iEListview->getErrorListSourceModel()->index(0, 0);
    option.state = QStyle::State_Selected;
    iEListview->m_IsTabFocus = true;
    iEListview->m_errorListItemDelegate->paint(p, option, index);

    iEListview->m_IsTabFocus = false;
    iEListview->m_errorListItemDelegate->paint(p, option, index);

    Stub st;
    st.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType1);
    iEListview->m_errorListItemDelegate->paint(p, option, index);

}

TEST_F(TestDFInstallErrorListView, checkMousePressEvent)
{

    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    iEListview->m_installErrorFontModelList.append(model1);

    model1.bChecked = true;
    iEListview->m_installErrorFontModelList.append(model1);
    iEListview->initErrorListData();

    QTest::mousePress(iEListview->viewport(), Qt::LeftButton);

    QTest::mousePress(iEListview->viewport(), Qt::RightButton);

}

TEST_F(TestDFInstallErrorListView, checkSetSelection)
{
    QSignalSpy spy(iEListview, SIGNAL(onClickErrorListItem(QModelIndex)));

    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    iEListview->m_installErrorFontModelList.append(model1);
    iEListview->initErrorListData();

    QRect t;
    QTest::mousePress(iEListview->viewport(), Qt::LeftButton);
    iEListview->setSelection(t, QItemSelectionModel::Select);
}

TEST_F(TestDFInstallErrorListView, checkKeyPressEvent)
{
    DFInstallErrorItemModel model1;
    model1.bIsNormalUserFont = true;
    model1.bChecked = false;
    iEListview->m_installErrorFontModelList.append(model1);
    iEListview->initErrorListData();

    QTest::keyPress(iEListview->viewport(), Qt::Key_Home);
    QTest::keyPress(iEListview->viewport(), Qt::Key_End);
    QTest::keyPress(iEListview->viewport(), Qt::Key_PageUp);
    QTest::keyPress(iEListview->viewport(), Qt::Key_PageDown);

    QTest::keyPress(iEListview->viewport(), Qt::Key_Up, Qt::ControlModifier);
    QTest::keyPress(iEListview->viewport(), Qt::Key_Up, Qt::ShiftModifier);

    QTest::keyPress(iEListview->viewport(), Qt::Key_Up);
    QTest::keyPress(iEListview->viewport(), Qt::Key_Down);
    QTest::keyRelease(iEListview->viewport(), Qt::Key_Down);

}

TEST_F(TestDFInstallErrorListView, checkLengthAutoFeed)
{
    QPainter *p = new QPainter;

    QString str;
    str.fill('a', 200);

    str = iEListview->m_errorListItemDelegate->lengthAutoFeed(p, str, 50);
    EXPECT_TRUE(str.contains(QLatin1String("...")));

    SAFE_DELETE_ELE(p)
}

TEST_F(TestDFInstallErrorListView, checkEventFilter)
{
    QFocusEvent *focusOutEvent = new QFocusEvent(QEvent::FocusOut);

    iEListview->eventFilter(iEListview, focusOutEvent);
    EXPECT_FALSE(iEListview->m_IsTabFocus);

    QFocusEvent *focusInEvent = new QFocusEvent(QEvent::FocusIn);

    iEListview->eventFilter(iEListview, focusInEvent);
    EXPECT_TRUE(iEListview->m_IsTabFocus);

    SAFE_DELETE_ELE(focusOutEvent)
    SAFE_DELETE_ELE(focusInEvent)
}




