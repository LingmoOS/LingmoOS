// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "clipboardloader.h"

#include <QApplication>
#include <QMimeData>
#include <QStyle>
#include <QSignalSpy>
#include <QTest>

class TstClipboardLoader : public testing::Test
{
public:
    void SetUp() override
    {
        loader = new ClipboardLoader();
    }

    void TearDown() override
    {
        delete loader;
        loader = nullptr;
    }

public:
    ClipboardLoader *loader = nullptr;
};

TEST_F(TstClipboardLoader, coverageTest)
{
    QPixmap srcPix;
    ItemInfo info;

    loader->cachePixmap(srcPix, info);
}

TEST_F(TstClipboardLoader, setImageData)
{
    QStyle *style = QApplication::style();
    const QPixmap &srcPix = style->standardPixmap(QStyle::SP_DialogYesButton);

    QSignalSpy spy(loader, &ClipboardLoader::dataComing);
    qApp->clipboard()->setPixmap(srcPix);

    QVERIFY(spy.count() == 1);

    QList<QVariant> arguments = spy.takeFirst();
    QByteArray pixBuf = arguments.at(0).toByteArray();

    // 重设一张图片的数据，防止连续两张同样的图片被写入
    const QPixmap &applyBtnPix = style->standardPixmap(QStyle::SP_DialogApplyButton);
    qApp->clipboard()->setPixmap(applyBtnPix);

    QTest::qWait(10);
    loader->dataReborned(pixBuf);

    qDebug() << spy.count();

    ItemInfo info;

    const QMimeData *mime = qApp->clipboard()->mimeData();
    QMimeData *newData = new QMimeData;

    foreach (auto key, mime->formats()) {
        newData->setData(key, mime->data(key));
        newData->setData("x-dfm-copied/file-icons", "");
        newData->setData("FROM_DEEPIN_CLIPBOARD_MANAGER", "1"); // 还有此数据不会触发剪贴板记录
    }

    // 复制文件测试
    QMimeData *fileMime = new QMimeData;
    fileMime->setData("text/uri-list", "1, 2");
    qApp->clipboard()->setMimeData(fileMime);

    QSignalSpy systemDataCheckSpy(loader, &ClipboardLoader::dataComing);
    loader->setImageData(info, newData);
    QTest::qWait(10);
    QVERIFY(systemDataCheckSpy.count() == 0);

    // 设置文本测试
    qApp->clipboard()->setText("test");

    delete newData;
    newData = nullptr;
}

