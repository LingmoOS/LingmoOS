// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QtConcurrent/QtConcurrent>
#include <functional>
#include <QMetaType>

#include "../testItems/publicApi.h"

#include "application.h"
#include "mainwindow.h"
#include "shortcut.h"

#include "QTestEventList"

#define protected public
#define private public

#include "ccutdialog.h"
#include "cexportimagedialog.h"
#include "cexportimagedialog_p.h"
#include "cprintmanager.h"
#include "drawdialog.h"

#if TEST_DIALOG

//对话框自动打桩机
//输入：对话框对象、对话框运行时要执行的操作
template<typename T, typename U>
void stubDialog(T &&activeFun, U &&processFun)
{
    activeFun();//启动函数

    QEventLoop loop;
    QtConcurrent::run([ =, &loop]() {
        (void)QTest::qWaitFor([ =, &loop]() {
            return (loop.isRunning());
        });
        (void)QTest::qWaitFor([ = ]() {
            return (qApp->activeModalWidget() != nullptr && qApp->activeModalWidget() != getMainWindow());
        });
        if (qApp->activeModalWidget() != nullptr) {
            QThread::msleep(200);
            processFun(); //要执行的操作在这里
            QThread::msleep(200);
            QMetaObject::invokeMethod(&loop, "quit");
        } else {
            QMetaObject::invokeMethod(&loop, "quit");
        }
    });
    loop.exec();
}

TEST(dialog, cutdialog)
{
    CCutDialog cutDialog;
    ASSERT_EQ(cutDialog.getCutStatus(), CCutDialog::Discard);

    //key event
    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&cutDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        DTestEventList e;
        e.addKeyPress(Qt::Key::Key_Escape);//这个会让它退出去，不需要执行done
        e.simulate(dialog);
    });

    //button
    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&cutDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "buttonClicked", Qt::QueuedConnection, Q_ARG(int, 0), Q_ARG(const QString &, "123"));
    });
    ASSERT_EQ(cutDialog.getCutStatus(), CCutDialog::Discard);

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&cutDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "buttonClicked", Qt::QueuedConnection, Q_ARG(int, 1), Q_ARG(const QString &, "123"));
    });
    ASSERT_EQ(cutDialog.getCutStatus(), CCutDialog::Save);
}

TEST(dialog, exportimagedialog)
{
    CExportImageDialog exportDialog;

    //exec
    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&exportDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        CExportImageDialog *dialog = qobject_cast<CExportImageDialog *>(qApp->activeModalWidget());

        DTestEventList e;
        e.addMouseClick(Qt::LeftButton, Qt::NoModifier, QPoint(1, 1), 100);
        //e.simulate(dialog->_pPrivate->_radioPiexlBtn);

        e.clear();
        e.addMouseClick(Qt::LeftButton, Qt::NoModifier);
        e.addKeyClick(Qt::Key_Backspace, Qt::NoModifier, 100);
        e.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 100);
        e.addDelay(1000);
//        e.simulate(dialog->_pPrivate->_widthEditor);
//        dialog->_pPrivate->_widthEditor->editingFinished();
//        e.simulate(dialog->_pPrivate->_heightEditor);
//        dialog->_pPrivate->_heightEditor->editingFinished();

        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(qApp, [ & ]() {
            exportDialog.execFileIsExists("123.png");
        });
    },
    [ = ]() {
        QDialog *dialog = qobject_cast<QDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });

    //exportDialog._pPrivate->_radioPiexlBtn->setChecked(true);

    ASSERT_EQ(exportDialog.getImageType(), CExportImageDialog::JPG);

    //slots
    //slotOnSavePathChange
    exportDialog.slotOnSavePathChange(CExportImageDialog::Documents);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Downloads);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Desktop);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Music);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Other);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Videos);
    exportDialog.slotOnSavePathChange(CExportImageDialog::Pictures);
    exportDialog.slotOnSavePathChange(99999);

    //slotOnFormatChange
    exportDialog.slotOnFormatChange(CExportImageDialog::PDF);

    ASSERT_EQ(exportDialog.getImageFormate(), QObject::tr("pdf"));

    //slotOnQualityChanged
    exportDialog.slotOnQualityChanged(1);
    ASSERT_EQ(exportDialog.getQuality(), 1);

    //get result
    exportDialog.resultFile();
    exportDialog.desImageSize();

    //other functions
//    exportDialog._pPrivate->resetImageSettingSizeTo({1920, 1080});
//    exportDialog._pPrivate->showTip(CExportImageDialog::CExportImageDialog_private::ETooBig);
//    QTest::qWait(500);
//    exportDialog._pPrivate->showTip(CExportImageDialog::CExportImageDialog_private::ETooSmall);
//    QTest::qWait(500);
//    exportDialog._pPrivate->showTip(CExportImageDialog::CExportImageDialog_private::ENoAlert);
//    QTest::qWait(500);

//    CExportImageDialog::CExportImageDialog_private::EAlertReason aler;
//    exportDialog._pPrivate->autoKeepWHRadio(CExportImageDialog::CExportImageDialog_private::EFreeSetting,
//                                            1.0, QSize(192, 108), QSize(1920, 1080), QSize(3840, 2160), aler);

//    ASSERT_EQ(exportDialog._pPrivate->isFocusInEditor(), false);

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(qApp, [ & ]() {
            exportDialog.execCheckFile(QApplication::applicationDirPath() + "/test.png");
        });
    },
    [ = ]() {
        QDialog *dialog = qobject_cast<QDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });
}

TEST(dialog, CPrintManager)
{
    //由于QMetaObject::invokeMethod机制，需要采用一点特殊方法来使其生效
    CPrintManager printManager;

    //showPrintDialog
    qRegisterMetaType<DWidget *>("DWidget *");
    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(qApp, [ & ]() {
            printManager.showPrintDialog(QImage(), nullptr, "");
        });
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(qApp, [ & ]() {
            printManager.showPrintDialog(QImage(500, 500, QImage::Format_RGB32), nullptr, "");
        });
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });
}

TEST(dialog, ProgressDialog)
{
    //ProgressDialog
    QWidget widget;
    ProgressDialog progressDialog("321", &widget);

    progressDialog.setText("123");
    progressDialog.setProcess(1, 100);

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&progressDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });

    //CAbstractProcessDialog
    CAbstractProcessDialog abstractProcessDialog;
    abstractProcessDialog.setTitle("123");
    abstractProcessDialog.setProcess(1);

    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&abstractProcessDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        QDialog *dialog = qobject_cast<QDialog *>(qApp->activeModalWidget());
        QMetaObject::invokeMethod(dialog, "done", Q_ARG(int, 1));
    });
}

TEST(dialog, DrawDialog)
{
    DrawDialog drawDialog;

    //key event
    stubDialog(
    [ & ]() {
        QMetaObject::invokeMethod(&drawDialog, "exec", Qt::QueuedConnection);
    },
    [ = ]() {
        DDialog *dialog = qobject_cast<DDialog *>(qApp->activeModalWidget());
        DTestEventList e;
        e.addKeyPress(Qt::Key::Key_Escape);//这个会让它退出去，不需要执行done
        e.simulate(dialog);
    });
}

#endif
