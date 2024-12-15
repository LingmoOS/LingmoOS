// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickprint.h"
#include "quickprint/quickprint_p.h"
#include "quickprint/printimageloader.h"
#include "gtestview.h"

#include <DPrintPreviewDialog>

#include <QtTest/QtTest>
#include <QImageReader>
#include <QTemporaryFile>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

class UT_QuickPrint : public ::testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

private:
    QMetaObject::Connection closeWindowConn;
};

void UT_QuickPrint::SetUp() {}

void UT_QuickPrint::TearDown()
{
    QObject::disconnect(closeWindowConn);
}

TEST_F(UT_QuickPrint, loadImageList_NormalSync_Pass)
{
    PrintImageLoader loader;
    QStringList imageList{QString(SVGPATH), QString(JPEGPATH), QApplication::applicationDirPath() + "/jpg.jpg"};
    bool ret = loader.loadImageList(imageList, false);

    EXPECT_TRUE(ret);
    EXPECT_FALSE(loader.loadData.isEmpty());

    auto takeData = loader.takeLoadData();
    EXPECT_EQ(takeData.size(), imageList.size());
    EXPECT_TRUE(loader.loadData.isEmpty());

    for (auto dataPtr : takeData) {
        EXPECT_TRUE(imageList.contains(dataPtr->filePath));
        EXPECT_EQ(dataPtr->state, ImageFileState::Loaded);
        EXPECT_EQ(dataPtr->frame, -1);
        EXPECT_FALSE(dataPtr->data.isNull());
    }

    EXPECT_FALSE(loader.isLoading());
}

TEST_F(UT_QuickPrint, loadImageList_NormalSync_Fault)
{
    PrintImageLoader loader;
    QSignalSpy spy(&loader, &PrintImageLoader::loadFinished);

    bool ret = loader.loadImageList({}, false);
    EXPECT_FALSE(ret);
    ASSERT_EQ(spy.count(), 0);

    ret = loader.loadImageList({"test", "test2"}, false);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(loader.loadData.isEmpty());

    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> recvArgs = spy.takeFirst();
    EXPECT_TRUE(recvArgs.at(0).toBool());
}

TEST_F(UT_QuickPrint, loadImageList_MultiSync_Pass)
{
    PrintImageLoader loader;
    QString gif1 = QApplication::applicationDirPath() + "/gif.gif";
    QString tif2 = QApplication::applicationDirPath() + "/tif.tif";

    QStringList imageList{gif1, tif2};
    bool ret = loader.loadImageList(imageList, false);
    EXPECT_TRUE(ret);

    QImageReader reader1(gif1);
    int count1 = reader1.imageCount();
    QImageReader reader2(tif2);
    int count2 = reader2.imageCount();

    ASSERT_EQ(loader.loadData.size(), count1 + count2);
    int i = 0;
    for (; i < count1; ++i) {
        EXPECT_EQ(loader.loadData.at(i)->filePath, gif1);
    }
    for (;i < count1 + count2; ++i) {
        EXPECT_EQ(loader.loadData.at(i)->filePath, tif2);
    }
}

TEST_F(UT_QuickPrint, showPrintDialog_CannotReadData_Failed)
{
    closeWindowConn = QObject::connect(qApp, &QApplication::focusWindowChanged, []() {
        for (QWindow *window : qApp->topLevelWindows()) {
            window->close();
        }
    });

    QuickPrint print;
    QSignalSpy spy(&print, &QuickPrint::printFinished);

    int ret = print.showPrintDialog({QApplication::applicationDirPath() + "/errorPic.icns"});
    EXPECT_FALSE(ret);

    qApp->processEvents();

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), QDialog::Rejected);
}

TEST_F(UT_QuickPrint, showPrintDialog_NoPermission_Failed)
{
    closeWindowConn = QObject::connect(qApp, &QApplication::focusWindowChanged, []() {
        for (QWindow *window : qApp->topLevelWindows()) {
            window->close();
        }
    });

    // No read permission
    QTemporaryFile tmpFile("testPic_XXXXXX.jpg");
    ASSERT_TRUE(tmpFile.open());
    tmpFile.setPermissions(QFileDevice::ExeOther);

    QuickPrint print;
    bool ret = print.showPrintDialog({tmpFile.fileName()});
    EXPECT_FALSE(ret);
}

TEST_F(UT_QuickPrint, showPrintDialog_NormalPicture_Pass)
{
    closeWindowConn = QObject::connect(qApp, &QApplication::focusWindowChanged, []() {
        for (QWidget *widget : qApp->topLevelWidgets()) {
            if (DPrintPreviewDialog *dialog = qobject_cast<DPrintPreviewDialog *>(widget)) {
                dialog->reject();
            } else {
                widget->close();
            }
        }
    });

    QuickPrint print;
    QSignalSpy spy(&print, &QuickPrint::printFinished);

    int ret = print.showPrintDialog({QApplication::applicationDirPath() + "/svg.svg"});
    EXPECT_TRUE(ret);

    qApp->processEvents();

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), QDialog::Rejected);
}

TEST_F(UT_QuickPrint, showPrintDialogAsync_ShowSpinner)
{
    QuickPrint print;
    QStringList hugeImageList;
    for (int i = 0; i < 1000; ++i) {
        hugeImageList.append(QApplication::applicationDirPath() + "/png.png");
    }

    bool ret = print.showPrintDialogAsync(hugeImageList);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(print.isLoading());
    EXPECT_TRUE(print.dd_ptr->procSpinnerTimer.isActive());

    print.dd_ptr->startSpinner();
    ASSERT_TRUE(print.dd_ptr->spinner);
    EXPECT_TRUE(QTest::qWaitForWindowExposed(print.dd_ptr->spinner.data()));
    EXPECT_TRUE(print.dd_ptr->spinner->isVisible());

    print.cancel();
    EXPECT_FALSE(print.dd_ptr->procSpinnerTimer.isActive());
    EXPECT_FALSE(print.dd_ptr->spinner->isVisible());
}

TEST_F(UT_QuickPrint, showPrintDialogAsync_Cancel)
{
    QuickPrint print;
    QStringList hugeImageList;
    for (int i = 0; i < 1000; ++i) {
        hugeImageList.append(QApplication::applicationDirPath() + "/png.png");
    }

    QSignalSpy spy(&print, &QuickPrint::printFinished);

    bool ret = print.showPrintDialogAsync(hugeImageList);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(print.isLoading());
    EXPECT_TRUE(print.dd_ptr->procSpinnerTimer.isActive());

    print.cancel();
    EXPECT_FALSE(print.isLoading());
    EXPECT_FALSE(print.dd_ptr->procSpinnerTimer.isActive());

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_QuickPrint, showPrintDialogAsync_RemoveFile_Failed)
{
    QuickPrint print;
    bool warnDialog = false;
    QEventLoop loop;

    QObject::connect(&print, &QuickPrint::printFinished, &loop, &QEventLoop::quit);
    closeWindowConn = QObject::connect(qApp, &QApplication::focusWindowChanged, [&]() {
        for (QWidget *widget : qApp->topLevelWidgets()) {
            if ("QuickPrint_WarnDialog" == widget->objectName()) {
                warnDialog = true;
                widget->close();
                loop.quit();
            } else {
                widget->close();
            }
        }
    });

    QImage image(QApplication::applicationDirPath() + "/png.png");
    QTemporaryFile tmpFile("test_remove_XXXXXX.png");
    ASSERT_TRUE(tmpFile.open());
    image.save(tmpFile.fileName());

    QStringList hugeImageList;
    for (int i = 0; i < 1000; ++i) {
        hugeImageList.append(tmpFile.fileName());
    }

    QSignalSpy spy(&print, &QuickPrint::printFinished);

    print.showPrintDialogAsync(hugeImageList);
    tmpFile.remove();

    loop.exec();
    // Wait queued signal
    qApp->processEvents();

    EXPECT_TRUE(warnDialog);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), QDialog::Rejected);
}

TEST_F(UT_QuickPrint, showPrintDialogAsync_RenameFile_Failed)
{
    QuickPrint print;
    bool warnDialog = false;
    QEventLoop loop;

    QObject::connect(&print, &QuickPrint::printFinished, &loop, &QEventLoop::quit);
    closeWindowConn = QObject::connect(qApp, &QApplication::focusWindowChanged, [&]() {
        for (QWidget *widget : qApp->topLevelWidgets()) {
            if ("QuickPrint_WarnDialog" == widget->objectName()) {
                warnDialog = true;
                widget->close();
                loop.quit();
            } else {
                widget->close();
            }
        }
    });

    QImage image(QApplication::applicationDirPath() + "/png.png");
    QTemporaryFile tmpFile("test_remove_XXXXXX.png");
    ASSERT_TRUE(tmpFile.open());
    image.save(tmpFile.fileName());

    QStringList hugeImageList;
    for (int i = 0; i < 1000; ++i) {
        hugeImageList.append(tmpFile.fileName());
    }

    QSignalSpy spy(&print, &QuickPrint::printFinished);

    print.showPrintDialogAsync(hugeImageList);
    tmpFile.rename(tmpFile.fileName() + ".png");

    loop.exec();
    // Wait queued signal
    qApp->processEvents();

    EXPECT_TRUE(warnDialog);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), QDialog::Rejected);
}
