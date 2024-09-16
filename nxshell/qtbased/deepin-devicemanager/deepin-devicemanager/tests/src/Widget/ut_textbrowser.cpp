// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TextBrowser.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "stub.h"
#include "ut_Head.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QMap>
#include <QDomDocument>
#include <QMenu>
#include <QClipboard>
#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_TextBrowser : public UT_HEAD
{
public:
    void SetUp()
    {
        tBrowser = new TextBrowser;
    }
    void TearDown()
    {
        delete tBrowser;
    }
    TextBrowser *tBrowser;
};

TEST_F(UT_TextBrowser, UT_TextBrowser_showDeviceInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "mouse";
    device->m_Interface = "USB";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    tBrowser->showDeviceInfo(bInfo.at(0));
    EXPECT_STREQ("(Unavailable)mouse ", tBrowser->toPlainText().toStdString().c_str());
    delete device;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "keyboard";
    device->m_Interface = "USB";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    tBrowser->mp_Info = bInfo.at(0);
    tBrowser->m_ShowOtherInfo = true;
    tBrowser->updateInfo();
    EXPECT_STREQ("(Unavailable)keyboard ", tBrowser->toPlainText().toStdString().c_str());
    delete device;
}

QString ut_selectedText()
{
    return "/";
}

TEST_F(UT_TextBrowser, UT_TextBrowser_setDeviceEnabled)
{
    tBrowser->updateShowOtherInfo();
    Stub stub;
    stub.set(ADDR(QTextCursor, selectedText), ut_selectedText);
    tBrowser->fillClipboard();
    tBrowser->mp_Info = nullptr;
    EXPECT_EQ(tBrowser->setDeviceEnabled(true), EDS_Cancle);
}

TEST_F(UT_TextBrowser, UT_TextBrowser_PaintEvent)
{
    QPaintEvent paint(QRect(tBrowser->rect()));
    tBrowser->paintEvent(&paint);
    EXPECT_FALSE(tBrowser->grab().isNull());
}

TEST_F(UT_TextBrowser, UT_TextBrowser_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_C, Qt::ShiftModifier);
    EXPECT_TRUE(tBrowser->event(&keyPressEvent));
}

TEST_F(UT_TextBrowser, UT_TextBrowser_slotActionRefresh)
{
    QSignalSpy spy1(tBrowser, SIGNAL(refreshInfo()));
    ASSERT_EQ(0, spy1.count());
    tBrowser->slotActionRefresh();
    ASSERT_EQ(1, spy1.count());

    QSignalSpy spy2(tBrowser, SIGNAL(exportInfo()));
    ASSERT_EQ(0, spy2.count());
    tBrowser->slotActionExport();
    ASSERT_EQ(1, spy2.count());
}

TEST_F(UT_TextBrowser, UT_TextBrowser_slotActionCopy)
{
    Stub stub;
    stub.set(ADDR(QTextCursor, selectedText), ut_selectedText);
    tBrowser->slotActionCopy();
    QClipboard *clipboard = DApplication::clipboard();
    EXPECT_STREQ("/", clipboard->text().toStdString().c_str());
}

TEST_F(UT_TextBrowser, UT_TextBrowser_domTitleInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "keyboard";
    device->m_Interface = "USB";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QDomDocument doc("h3");
    tBrowser->domTitleInfo(doc, bInfo.at(0));
    EXPECT_FALSE(doc.isNull());
    delete device;
}

bool ut_rect_contains()
{
    return false;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_focusInEvent)
{
    tBrowser->m_ShowOtherInfo = true;
    QFocusEvent focus(QFocusEvent::FocusIn);
    QCoreApplication::sendEvent(tBrowser, &focus);

    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contains);
    DeviceInput *device = new DeviceInput;
    device->m_Name = "keyboard";
    device->m_Interface = "USB";

    tBrowser->mp_Info = dynamic_cast<DeviceBaseInfo *>(device);
    QFocusEvent focusd(QFocusEvent::FocusOut);
    EXPECT_TRUE(tBrowser->event(&focusd));
    delete device;
}

void ut_text_exec()
{
    return;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_slotShowMenu)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_text_exec);

    tBrowser->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(tBrowser->mp_Menu->actions().size(), 3);
}
