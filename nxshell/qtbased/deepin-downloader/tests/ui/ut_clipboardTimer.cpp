// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "clipboardtimer.h"
#include <QClipboard>
#include <QMimeData>
#include <QObject>

class ut_clipboardTimer : public ::testing::Test
{
protected:
    ut_clipboardTimer()
    {
    }

    virtual ~ut_clipboardTimer()
    {
    }
};

TEST_F(ut_clipboardTimer, checkClipboardHasUrl)
{
    ClipboardTimer *c = new ClipboardTimer;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText("111111111");
    c->checkClipboardHasUrl();
    c->isMagnetFormat("111");
    c->isMagnetFormat("magnet:?xt=urn:btih:111");
    c->isHttpFormat("111");
    c->isHttpFormat("http://111.txt");
    c->isHttpFormat("http://111.tx");
    c->isBtFormat("111");
    c->isBtFormat("http://111.txt");
    c->isBtFormat("http://111.torrent");
    c->isMlFormat("111");
    c->isMlFormat("http://111.txt");
    c->isMlFormat("http://111.torrent");
    c->getTypeList();
    c->getWebList();
    c->isWebFormat("http://11.txt");
    QStringList list;
    list << "111"<< "222";
    c->midWebList(list);



//    QMimeData mimeData;
//    mimeData.setText("Text");
//    mimeData.setData("TIMESTAMP", "111");
//    clipboard->setMimeData(&mimeData);
//    c->m_timeStamp = "111";
//    c->checkClipboardHasUrl();

//    mimeData.setData("FROM_DEEPIN_CLIPBOARD_MANAGER", "1");
//    mimeData.setData("TIMESTAMP", "1");
//    clipboard->setMimeData(&mimeData);
//    c->checkClipboardHasUrl();

//    mimeData.setData("FROM_DEEPIN_CLIPBOARD_MANAGER", "0");
//    QClipboard *clipboard1 = QApplication::clipboard();
//    clipboard1->setText("magnet:?xt=urn:btih:0FC4D73CCC9E6AC29A1B10DDCC3696E81D6CACAF");
//    c->checkClipboardHasUrl();
//    delete c;
//    c = nullptr;
    //  c->~ClipboardTimer();
    if(c != nullptr){
        delete c;
        c = nullptr;
    }

}

TEST_F(ut_clipboardTimer, getDataChanged)
{
//    ClipboardTimer c;
//    c.getDataChanged();
}
