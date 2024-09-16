// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include "gtest/gtest.h"
#include <qtest.h>
#include <DTitlebar>
#include <DSuggestButton>
#include <DWarningButton>
#include <QTimer>
#include <QObject>

#include "../application/partedproxy/dmdbushandler.h"
#include "../application/partedproxy/dmdbusinterface.h"
#include "mainwindow.h"
#include "centerwidget.h"
#include "mainsplitter.h"
#include "devicelistwidget.h"
#include "partitionwidget.h"
#include "mountdialog.h"
#include "dmtreeview.h"
#include "titlewidget.h"
#include "unmountdialog.h"
#include "formatedialog.h"
#include "resizedialog.h"
#include "stub.h"
#include "stubAll.h"
#include "messagebox.h"
#include "partchartshowing.h"
#include "diskinfodisplaydialog.h"
#include "diskhealthdetectiondialog.h"
#include "diskbadsectorsdialog.h"
#include "createpartitiontabledialog.h"
#include "partitiontableerrorsinfodialog.h"
#include "cylinderinfowidget.h"
#include "sizeinfowidget.h"

class ut_application : public ::testing::Test
        , public QObject
{
protected:
    ut_application()
    {
    }

    virtual ~ut_application()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_application, init)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getDeviceInfo), getDeviceInfo);

    Stub stub3;
    stub3.set(ADDR(DMDbusHandler, onSetCurSelect), setCurSelect);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, startService), start);

    typedef int (*fptr)(SizeInfoWidget*);
    fptr foo = (fptr)(&DGuiApplicationHelper::themeType);
    Stub stub4;
    stub4.set(foo, currentThemeType);


    qDebug() << MainWindow::instance()->isHidden();
    qDebug() << MainWindow::instance()->children();
//    qDebug() << DMDbusHandler::instance()->getDeviceNameList();
//    qDebug() << DMDbusHandler::instance()->getCurDeviceInfo().m_path
//             << DMDbusHandler::instance()->getCurDeviceInfo().partition.count()
//             << DMDbusHandler::instance()->getCurPartititonInfo().m_path;

//    ASSERT_FALSE(DMDbusHandler::instance()->getCurDeviceInfo().m_path.isEmpty());
//    ASSERT_FALSE(DMDbusHandler::instance()->getCurPartititonInfo().m_path.isEmpty());

    MainWindow::instance()->show();

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    view->setRefreshItem(0, 0);
    QTest::qWait(1000);
    EXPECT_EQ(view->getCurrentNum(), 0);
    EXPECT_EQ(view->getCurrentTopNum(), 0);

    QRect rect = view->visualRect(view->currentIndex());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());
}

TEST_F(ut_application, hideSuccess)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, hidePartition), hide);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->m_curDiskInfoData = deviceListWidget->m_treeView->getCurItem()->data().value<DiskInfoData>();
    deviceListWidget->onHidePartitionClicked();

    DMDbusHandler::instance()->onHidePartition("1");
    ASSERT_TRUE(deviceListWidget->isHideSuccess);
}

TEST_F(ut_application, hideFailed)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, hidePartition), hide);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->m_curDiskInfoData = deviceListWidget->m_treeView->getCurItem()->data().value<DiskInfoData>();
    deviceListWidget->onHidePartitionClicked();

    DMDbusHandler::instance()->onHidePartition("0");
    ASSERT_FALSE(deviceListWidget->isHideSuccess);
}

TEST_F(ut_application, hide_mount)
{
    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    DiskInfoData oldData = deviceListWidget->m_curDiskInfoData;
    DiskInfoData diskInfoData = deviceListWidget->m_curDiskInfoData;
    qDebug() << diskInfoData.m_mountpoints;
    diskInfoData.m_mountpoints = "/";
    deviceListWidget->m_curDiskInfoData = diskInfoData;

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub5;
    stub5.set(foo, MessageboxExec);

    deviceListWidget->onHidePartitionClicked();
    ASSERT_FALSE(deviceListWidget->isHideSuccess);

    diskInfoData.m_mountpoints = "/mnt";
    deviceListWidget->m_curDiskInfoData = diskInfoData;

    deviceListWidget->onHidePartitionClicked();
    ASSERT_FALSE(deviceListWidget->isHideSuccess);

    deviceListWidget->m_curDiskInfoData = oldData;
}

TEST_F(ut_application, showSuccess)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, unhidePartition), show);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onShowPartitionClicked();

    DMDbusHandler::instance()->onShowPartition("1");
    ASSERT_TRUE(deviceListWidget->isShowSuccess);
}

TEST_F(ut_application, showFailed)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, unhidePartition), show);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onShowPartitionClicked();

    DMDbusHandler::instance()->onShowPartition("0");
    ASSERT_FALSE(deviceListWidget->isShowSuccess);
}

TEST_F(ut_application, deleteMountPartition)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, deletePartition), deletePartition);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    DiskInfoData oldData = deviceListWidget->m_curDiskInfoData;
    DiskInfoData diskInfoData = deviceListWidget->m_curDiskInfoData;
    qDebug() << diskInfoData.m_mountpoints;
    diskInfoData.m_mountpoints = "/";
    deviceListWidget->m_curDiskInfoData = diskInfoData;

    deviceListWidget->onDeletePartitionClicked();
    ASSERT_FALSE(deviceListWidget->isDeleteSuccess);

    deviceListWidget->m_curDiskInfoData = oldData;
}

TEST_F(ut_application, deletePartitionSuccess)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, deletePartition), deletePartition);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onDeletePartitionClicked();

    DMDbusHandler::instance()->onDeletePartition("1:0");
    ASSERT_TRUE(deviceListWidget->isDeleteSuccess);
}

TEST_F(ut_application, deletePartitionFailed)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, deletePartition), deletePartition);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onDeletePartitionClicked();

    DMDbusHandler::instance()->onDeletePartition("0:0");
    ASSERT_FALSE(deviceListWidget->isDeleteSuccess);

    DMDbusHandler::instance()->onDeletePartition("");
}

TEST_F(ut_application, unmountPartition)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, unmount), unmountPartition);

    typedef int (*fptr)(UnmountDialog*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);
    
    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    PartitionInfo oldInfo = DMDbusHandler::instance()->m_curPartitionInfo;
    PartitionInfo info = DMDbusHandler::instance()->m_curPartitionInfo;
    QVector<QString> vector;
    vector.append("/");
    info.m_mountPoints = vector;
    DMDbusHandler::instance()->m_curPartitionInfo = info;

    UnmountDialog *unmount = new UnmountDialog;
    unmount->onButtonClicked(1, "");
    
    DMDbusHandler::instance()->onUnmountPartition("0");
    ASSERT_FALSE(deviceListWidget->isUnmountSuccess);

    info.m_mountPoints = QVector<QString>();
    DMDbusHandler::instance()->m_curPartitionInfo = info;
    unmount->onButtonClicked(1, "");

    DMDbusHandler::instance()->onUnmountPartition("1");
    ASSERT_TRUE(deviceListWidget->isUnmountSuccess);
    
    DMDbusHandler::instance()->m_curPartitionInfo = oldInfo;
}

TEST_F(ut_application, mountPartition)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, mount), mountPartition);

    typedef int (*fptr)(UnmountDialog*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    MountDialog *mount = new MountDialog;
    mount->m_fileChooserEdit->setText("/");
    mount->onButtonClicked(1, "");

    mount->m_fileChooserEdit->setText("/mnt");
    mount->onButtonClicked(1, "");

    mount->onButtonClicked(0, "");
}

TEST_F(ut_application, formatPartition)
{
//    Stub stub;
//    stub.set(ADDR(DMDbusHandler, format), formatPartition);

//    Stub stub2;
//    stub2.set(ADDR(DMDbusHandler, getAllSupportFileSystem), getAllSupportFileSystem);

//    FormateDialog *formatDialog = new FormateDialog;

//    formatDialog->m_formatComboBox->setCurrentText("fat32");
//    formatDialog->m_fileNameEdit->setText("一二三四五六七");
//    formatDialog->m_fileNameEdit->setText("一二三");

//    formatDialog->m_formatComboBox->setCurrentText("ext4");
//    formatDialog->m_fileNameEdit->setText("一二三四五六七");
//    formatDialog->m_fileNameEdit->setText("一二三四五");

//    formatDialog->onWipeButtonClicked();
}

TEST_F(ut_application, newPartition)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, onSetCurSelect), setCurSelectOcated);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, create), createPartition);

    Stub stub3;
    stub3.set(ADDR(DMDbusHandler, getAllSupportFileSystem), getAllSupportFileSystem);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    view->setRefreshItem(0, 2);
    QTest::qWait(1000);
    EXPECT_EQ(view->getCurrentNum(), 2);
    EXPECT_EQ(view->getCurrentTopNum(), 0);

    QRect rect = view->visualRect(view->currentIndex());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();
    QPushButton *partition = titleWidget->findChild<QPushButton *>("partition");

    ASSERT_TRUE(partition->isEnabled());

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        QWidgetList l = QApplication::topLevelWidgets();
        foreach (QWidget *w, l) {
            if (w->objectName() == "partitionDialog") {
                QTest::keyClick(w, Qt::Key_Enter);
            }
            if (w->objectName() == "partitionWidget") {
                timer->stop();

                DLineEdit *partName = w->findChild<DLineEdit *>("partName");
                DLineEdit *partSize = w->findChild<DLineEdit *>("partSize");
                DIconButton *addButton = w->findChild<DIconButton *>("addButton");
                DIconButton *removeButton = w->findChild<DIconButton *>("removeButton");
                DPushButton *confirmButton = w->findChild<DPushButton *>("confirm");
                DPushButton *revertButton = w->findChild<DPushButton *>("revert");
                PartChartShowing *partChartShowing = w->findChild<PartChartShowing *>();
                PartitionWidget *widget = qobject_cast<PartitionWidget*>(w);

                QTest::mouseMove(partChartShowing, QPoint(20, 20));
                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(20, 20));
                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(-1, -1));

                widget->m_partFormateCombox->setCurrentText("fat32");
                partName->setText("一二三四五六七");
                partName->setText("一二三");
                widget->m_partFormateCombox->setCurrentText("ext4");

                partName->setText("一二三四五六七");
                ASSERT_FALSE(addButton->isEnabled());
                QTest::qWait(1000);

                partName->setText("1111");
                ASSERT_TRUE(addButton->isEnabled());
                QTest::qWait(1000);

                partSize->setText("1");
                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);
                ASSERT_EQ(widget->partCount(), 1);

                QTest::qWait(1000);
                partSize->setText("2");
                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);
                ASSERT_EQ(widget->partCount(), 2);

                QTest::qWait(1000);
                QTest::mouseClick(removeButton, Qt::LeftButton);
                QTest::qWait(1000);
                ASSERT_EQ(widget->partCount(), 1);

                widget->m_partComboBox->setCurrentText("MiB");
                widget->m_partComboBox->setCurrentText("GiB");

                widget->m_slider->setValue(0);
                widget->m_slider->setValue(20);
                widget->m_slider->setValue(50);
                widget->m_slider->setValue(100);

                QTest::qWait(1000);
                QTest::mouseClick(revertButton, Qt::LeftButton);
                QTest::qWait(1000);
                ASSERT_EQ(widget->partCount(), 0);

                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);

                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(400, 20));

                QTest::qWait(1000);
                QTest::mouseClick(revertButton, Qt::LeftButton);
                QTest::qWait(1000);

                partSize->setText("1");
                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);

                partSize->setText("1");
                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);

                QTest::mouseMove(partChartShowing, QPoint(150, 28));
                QTest::qWait(1000);
                QTest::mouseMove(partChartShowing, QPoint(350, 28));
                QTest::qWait(1000);
                QTest::mouseMove(partChartShowing, QPoint(90, 75));

                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(24, 40));
                QTest::qWait(1000);
                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(15, 40));
                QTest::qWait(1000);
                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(400, 20));

                QTest::mouseClick(addButton, Qt::LeftButton);
                QTest::qWait(1000);

                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(20, 20));
                QTest::mouseClick(partChartShowing, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(400, 20));

                QTest::mouseMove(partChartShowing, QPoint(30, 20));

                QTest::mouseClick(confirmButton, Qt::LeftButton);
                QTest::qWait(1000);

                break;
            }
        }
    });
    timer->start(1000);

    QTest::mouseClick(partition, Qt::LeftButton);
}

TEST_F(ut_application, resize)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, onSetCurSelect), setCurSelectSecond);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, resize), resizePartition);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    view->setRefreshItem(0, 1);
    QTest::qWait(1000);
    EXPECT_EQ(view->getCurrentNum(), 1);
    EXPECT_EQ(view->getCurrentTopNum(), 0);

    QRect rect = view->visualRect(view->currentIndex());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());

    ResizeDialog *resizeDialog = new ResizeDialog;
    resizeDialog->m_lineEdit->setText("1024");
    resizeDialog->m_comboBox->setCurrentIndex(1);
    resizeDialog->m_comboBox->setCurrentIndex(0);
    resizeDialog->onButtonClicked(1, "");

    resizeDialog->m_comboBox->setCurrentIndex(1);
    resizeDialog->m_lineEdit->setText("1");
    resizeDialog->onButtonClicked(1, "");

    resizeDialog->m_comboBox->setCurrentIndex(1);
    resizeDialog->m_lineEdit->setText("10000");
    resizeDialog->onButtonClicked(1, "");

    resizeDialog->m_lineEdit->setText("1");
    resizeDialog->onButtonClicked(0, "");
}

TEST_F(ut_application, diskInfo)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getHardDiskInfo), getDiskInfo);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    DiskInfoDisplayDialog *diskInfoDisplayDialog = new DiskInfoDisplayDialog(deviceListWidget->m_curDiskInfoData.m_diskPath);

    typedef int (*fptr)(DiskInfoDisplayDialog*);
    fptr foo = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub2;
    stub2.set(foo, emptyFilePath);

    diskInfoDisplayDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskInfoDisplayDialog*);
    fptr foo2 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub3;
    stub3.set(foo2, errorFilePath);

    diskInfoDisplayDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskInfoDisplayDialog*);
    fptr foo3 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub4;
    stub4.set(foo3, noPermissionFilePath);

    diskInfoDisplayDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskInfoDisplayDialog*);
    fptr foo4 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub5;
    stub5.set(foo4, filePath);

    diskInfoDisplayDialog->onExportButtonClicked();

    QFile file("/mnt/DiskInfo.txt");
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(ut_application, diskHealth)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getHardDiskInfo), getDiskInfo);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, getDeviceHardStatus), passedDeviceStatus);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

//    DiskHealthDetectionDialog diskHealthDetectionDialog(deviceListWidget->m_curDiskInfoData.m_diskPath, deviceCheckHealthInfo());

    DiskHealthDetectionDialog *diskHealthDetectionDialog = new DiskHealthDetectionDialog(deviceListWidget->m_curDiskInfoData.m_diskPath, deviceCheckHealthInfo());

    typedef int (*fptr)(DiskHealthDetectionDialog*);
    fptr foo = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub3;
    stub3.set(foo, emptyFilePath);

    diskHealthDetectionDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskHealthDetectionDialog*);
    fptr foo2 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub4;
    stub4.set(foo2, errorFilePath);

    diskHealthDetectionDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskHealthDetectionDialog*);
    fptr foo3 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub5;
    stub5.set(foo3, noPermissionFilePath);

    diskHealthDetectionDialog->onExportButtonClicked();

    typedef int (*fptr)(DiskHealthDetectionDialog*);
    fptr foo4 = (fptr)(&QFileDialog::getSaveFileName);
    Stub stub6;
    stub6.set(foo4, filePath);

    diskHealthDetectionDialog->onExportButtonClicked();

    QFile file("/mnt/DiskInfo.txt");
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(ut_application, diskBadSectorsCheck)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, checkBadSectors), badSectorsCheck);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, repairBadBlocks), badSectorsRepair);

    typedef int (*fptr)(DiskBadSectorsDialog*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub3;
    stub3.set(foo, MessageboxExec);

    DiskBadSectorsDialog *diskBadSectorsDialog = new DiskBadSectorsDialog;

    diskBadSectorsDialog->m_startLineEdit->setText("10");
    diskBadSectorsDialog->m_endLineEdit->setText("5");
    ASSERT_FALSE(diskBadSectorsDialog->inputValueIsEffective());

    diskBadSectorsDialog->m_startLineEdit->setText("");
    diskBadSectorsDialog->m_endLineEdit->setText("");
    diskBadSectorsDialog->m_checkTimesEdit->setText("");
    ASSERT_FALSE(diskBadSectorsDialog->inputValueIsEffective());

    diskBadSectorsDialog->m_startLineEdit->setText("100000");
    diskBadSectorsDialog->m_endLineEdit->setText("100000");
    ASSERT_FALSE(diskBadSectorsDialog->inputValueIsEffective());

    diskBadSectorsDialog->m_methodComboBox->setCurrentIndex(1);
    diskBadSectorsDialog->m_timeoutEdit->setText("");
    ASSERT_FALSE(diskBadSectorsDialog->inputValueIsEffective());

    diskBadSectorsDialog->m_timeoutEdit->setText("4000");
    ASSERT_FALSE(diskBadSectorsDialog->inputValueIsEffective());

    diskBadSectorsDialog->m_verifyComboBox->setCurrentIndex(1);
    diskBadSectorsDialog->m_timeoutEdit->setText("300");
    ASSERT_TRUE(diskBadSectorsDialog->inputValueIsEffective());
    diskBadSectorsDialog->onStartVerifyButtonClicked();
    diskBadSectorsDialog->onCheckComplete();
    diskBadSectorsDialog->onAgainVerifyButtonClicked(); // 重新检测
    diskBadSectorsDialog->onExitButtonClicked();

    diskBadSectorsDialog->m_verifyComboBox->setCurrentIndex(2);
    diskBadSectorsDialog->onStartVerifyButtonClicked();
    diskBadSectorsDialog->onCheckComplete();
    diskBadSectorsDialog->onResetButtonClicked(); // 复位

    diskBadSectorsDialog->m_methodComboBox->setCurrentIndex(0);
    diskBadSectorsDialog->m_verifyComboBox->setCurrentIndex(0);
    diskBadSectorsDialog->m_startLineEdit->setText("10");
    diskBadSectorsDialog->m_endLineEdit->setText("5");
    diskBadSectorsDialog->onStartVerifyButtonClicked();

    diskBadSectorsDialog->m_startLineEdit->setText("10");
    diskBadSectorsDialog->m_endLineEdit->setText("50");
    diskBadSectorsDialog->onStartVerifyButtonClicked();

    diskBadSectorsDialog->onCheckBadBlocksInfo("10", "62", "good", "");
    diskBadSectorsDialog->onCheckBadBlocksInfo("11", "389", "bad", "IO Read Error");

    diskBadSectorsDialog->onStopButtonClicked(); // 停止检测
    diskBadSectorsDialog->onContinueButtonClicked(); // 继续检测
    diskBadSectorsDialog->onCheckBadBlocksInfo("12", "75", "good", "");
    diskBadSectorsDialog->onCheckBadBlocksInfo("13", "456", "bad", "IO Read Error");

    diskBadSectorsDialog->onCheckComplete(); // 检测完成

    diskBadSectorsDialog->onExitButtonClicked();
}

TEST_F(ut_application, diskBadSectorsRepair)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, checkBadSectors), badSectorsCheck);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, repairBadBlocks), badSectorsRepair);

    typedef int (*fptr)(DiskBadSectorsDialog*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub3;
    stub3.set(foo, MessageboxExec);

    typedef int (*fptr)(DiskBadSectorsDialog*);
    fptr foo2 = (fptr)(&DDialog::exec);
    Stub stub4;
    stub4.set(foo2, MessageboxExec);

    DeviceInfoMap oldInfo = DMDbusHandler::instance()->m_deviceMap;
    DMDbusHandler::instance()->m_deviceMap = deviceInfo_mountPartition();

    DiskBadSectorsDialog *diskBadSectorsDialog = new DiskBadSectorsDialog;
    diskBadSectorsDialog->onRepairButtonClicked();

    DMDbusHandler::instance()->m_deviceMap = oldInfo;
    diskBadSectorsDialog->onRepairButtonClicked();
    diskBadSectorsDialog->onExitButtonClicked();

    diskBadSectorsDialog->m_methodComboBox->setCurrentIndex(0);
    diskBadSectorsDialog->m_verifyComboBox->setCurrentIndex(0);

    diskBadSectorsDialog->m_startLineEdit->setText("10");
    diskBadSectorsDialog->m_endLineEdit->setText("15");
    diskBadSectorsDialog->onStartVerifyButtonClicked();

    diskBadSectorsDialog->onCheckBadBlocksInfo("10", "388", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckBadBlocksInfo("11", "385", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckBadBlocksInfo("12", "386", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckBadBlocksInfo("13", "387", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckBadBlocksInfo("14", "389", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckBadBlocksInfo("15", "380", "bad", "IO Read Error");
    diskBadSectorsDialog->onCheckComplete();

    diskBadSectorsDialog->onRepairButtonClicked();
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairBadBlocksInfo("10", "good", "334");
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairBadBlocksInfo("11", "good", "338");
    QTest::qWait(1000);

    diskBadSectorsDialog->onStopButtonClicked(); // 停止修复
    diskBadSectorsDialog->onContinueButtonClicked(); // 继续修复

    diskBadSectorsDialog->onRepairBadBlocksInfo("12", "good", "339");
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairBadBlocksInfo("13", "good", "357");
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairBadBlocksInfo("14", "good", "358");
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairBadBlocksInfo("15", "good", "355");
    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairComplete(); // 修复完成

    diskBadSectorsDialog->onDoneButtonClicked();
}

TEST_F(ut_application, diskCheckRepair)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, checkBadSectors), badSectorsCheck);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, repairBadBlocks), badSectorsRepair);

    typedef int (*fptr)(DiskBadSectorsDialog*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub3;
    stub3.set(foo, MessageboxExec);

    typedef int (*fptr)(DiskBadSectorsDialog*);
    fptr foo2 = (fptr)(&DDialog::exec);
    Stub stub4;
    stub4.set(foo2, MessageboxExec);

    DeviceInfoMap oldInfo = DMDbusHandler::instance()->m_deviceMap;

    DiskBadSectorsDialog *diskBadSectorsDialog = new DiskBadSectorsDialog;
    diskBadSectorsDialog->show();
    diskBadSectorsDialog->m_startLineEdit->setText("0");
    diskBadSectorsDialog->m_endLineEdit->setText("560");

    DPushButton *startButton = diskBadSectorsDialog->findChild<DPushButton *>("start");
    QTest::mouseClick(startButton, Qt::LeftButton);
    QTest::qWait(1000);
//    diskBadSectorsDialog->onStartVerifyButtonClicked();

    QStringList lstCylinder;
    for (int i = 0; i < 561; i++) {
        QString cylinderNumber = QString::number(i);
        QString cylinderTimeConsuming = "254";
        QString cylinderStatus = "good";
        QString cylinderErrorInfo = "";

        if (i % 30 == 0) {
            cylinderStatus = "bad";
            cylinderErrorInfo = "IO Read Error";

            lstCylinder << cylinderNumber;
        }

        diskBadSectorsDialog->onCheckBadBlocksInfo(cylinderNumber, cylinderTimeConsuming, cylinderStatus, cylinderErrorInfo);

        QTest::qWait(200);
    }
    diskBadSectorsDialog->onCheckComplete();

    QTest::qWait(1000);
    DPushButton *repairButton = diskBadSectorsDialog->findChild<DPushButton *>("repair");
    QTest::mouseClick(repairButton, Qt::LeftButton);
    QTest::qWait(1000);
//    diskBadSectorsDialog->onRepairButtonClicked();

    for (int i = 0; i < lstCylinder.count(); i++) {
        diskBadSectorsDialog->onRepairBadBlocksInfo(lstCylinder.at(i), "good", "2000");

        QTest::qWait(2000);
    }

    QTest::qWait(1000);
    diskBadSectorsDialog->onRepairComplete(); // 修复完成

    diskBadSectorsDialog->onDoneButtonClicked();
}

TEST_F(ut_application, cylinderInfoTest)
{
    CylinderInfoWidget *cylinderInfoWidget = new CylinderInfoWidget(291);

    cylinderInfoWidget->againVerify(291);
    cylinderInfoWidget->againVerify(750);
    cylinderInfoWidget->reset(278);

    cylinderInfoWidget->updateCylinderInfo(0, QString::number(0 * 16065), "0", "200", "good", "", "0");
    cylinderInfoWidget->updateCylinderInfo(1, QString::number(1 * 16065), "0", "200", "bad", "IO Read Error", "0");
    cylinderInfoWidget->updateCylinderInfo(2, QString::number(2 * 16065), "0", "200", "", "", "0");
    cylinderInfoWidget->updateCylinderInfo(3, "", "0", "200", "", "", "0");
    cylinderInfoWidget->updateCylinderInfo(361, "", "0", "200", "", "", "0");
}

TEST_F(ut_application, cylinderInfoTest2)
{
    Stub stub;
    stub.set(ADDR(CylinderInfoWidget, updateCylinderInfo), updateCylinderInfo);

    CylinderInfoWidget *cylinderInfoWidget = new CylinderInfoWidget(291);

    cylinderInfoWidget->setCylinderNumber(300);
    cylinderInfoWidget->setCylinderNumber(560);
    cylinderInfoWidget->m_startCylinder = 0;
    cylinderInfoWidget->m_endCylinder = 560;

    for (int i = 0; i < 561; i++) {
        QString strLBANumber = QString::number(i * 16065);
        QString cylinderNumber = QString::number(i);
        QString cylinderTimeConsuming = "254";
        QString cylinderStatus = "good";
        QString cylinderErrorInfo = "";

        if (i % 30 == 0) {
            cylinderStatus = "bad";
            cylinderErrorInfo = "IO Read Error";
        }

        cylinderInfoWidget->setCurCheckBadBlocksInfo(strLBANumber, cylinderNumber, cylinderTimeConsuming, cylinderStatus, cylinderErrorInfo);
    }
}

TEST_F(ut_application, createPartitionTable_replace)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, createPartitionTable), createPartitionTable);

    CreatePartitionTableDialog *createPartitionTableDialog = new CreatePartitionTableDialog();
    createPartitionTableDialog->onButtonClicked(1, createPartitionTableDialog->m_curType);

    DMDbusHandler::instance()->onCreatePartitionTable(true);

    DMDbusHandler::instance()->onCreatePartitionTable(false);
}

TEST_F(ut_application, createPartitionTable_create)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, createPartitionTable), createPartitionTable);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    DeviceInfoMap oldInfo = DMDbusHandler::instance()->m_deviceMap;
    DMDbusHandler::instance()->m_deviceMap = deviceInfo_noPartitionTable();

    CreatePartitionTableDialog *createPartitionTableDialog = new CreatePartitionTableDialog();
    createPartitionTableDialog->onButtonClicked(1, createPartitionTableDialog->m_curType);

    DMDbusHandler::instance()->onCreatePartitionTable(true);

    DMDbusHandler::instance()->onCreatePartitionTable(false);

    DMDbusHandler::instance()->m_deviceMap = oldInfo;
}

TEST_F(ut_application, createPartitionTable)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, createPartitionTable), createPartitionTable);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo2 = (fptr)(&CreatePartitionTableDialog::exec);
    Stub stub3;
    stub3.set(foo2, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onCreatePartitionTableClicked();
}

TEST_F(ut_application, createPartitionTable_mount)
{
    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    DeviceInfoMap oldInfo = DMDbusHandler::instance()->m_deviceMap;
    DMDbusHandler::instance()->m_deviceMap = deviceInfo_mountPartition();

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onCreatePartitionTableClicked();

    DMDbusHandler::instance()->m_deviceMap = oldInfo;
}

TEST_F(ut_application, partitionTableCheck_error)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, detectionPartitionTableError), partitionTableErrorCheck_error);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&PartitionTableErrorsInfoDialog::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onPartitionErrorCheckClicked();
}

TEST_F(ut_application, partitionTableCheck_normal)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, detectionPartitionTableError), partitionTableErrorCheck_normal);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onPartitionErrorCheckClicked();
}

TEST_F(ut_application, partitionTableErrorCheck) // 模拟右键点击分区表错误检测，需要页面显示出来
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, detectionPartitionTableError), partitionTableErrorCheck_error);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "Health management") {
                timer->stop();
                QList<QAction*> lstAction = w.at(i)->actions();
                for (int j = 0; j < lstAction.count(); j++) {
                    QAction* action = lstAction.at(j);
                    if (action->objectName() == "Check partition table error") {
                        action->triggered(true);
                        break;
                    }
                }
                break;
            }
        }
     });
    timer->start(1000);

    QTimer::singleShot(2000, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "partitionErrorCheck") {
                w.at(i)->close();
            }
        }
    });

    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(100, 50));
    view->customContextMenuRequested(QPoint(100, 50));
}

TEST_F(ut_application, diskInformationTest)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getHardDiskInfo), getDiskInfo);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&DiskInfoDisplayDialog::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onDiskInfoClicked();
}

TEST_F(ut_application, diskBadSectorsTest)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, checkBadSectors), badSectorsCheck);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, repairBadBlocks), badSectorsRepair);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&DiskBadSectorsDialog::exec);
    Stub stub3;
    stub3.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onDiskBadSectorsClicked();
}

TEST_F(ut_application, diskCheckInformationEmpty)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getDeviceHardStatusInfo), emptyDeviceCheckHealthInfo);

    typedef int (*fptr)(DeviceListWidget*);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();

    deviceListWidget->onDiskCheckHealthClicked();
}

TEST_F(ut_application, diskHealthCheck)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, getDeviceHardStatusInfo), deviceCheckHealthInfo);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, getHardDiskInfo), getDiskInfo);

    Stub stub3;
    stub3.set(ADDR(DMDbusHandler, getDeviceHardStatus), failureDeviceStatus);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "Health management") {
                timer->stop();
                QList<QAction*> lstAction = w.at(i)->actions();
                for (int j = 0; j < lstAction.count(); j++) {
                    QAction* action = lstAction.at(j);
                    if (action->objectName() == "Check health") {
                        action->triggered(true);
                        break;
                    }
                }
                break;
            }
        }
    });
    timer->start(1000);

    QTimer::singleShot(2000, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        qDebug() << w;
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "diskHealthDetectionDialog") {
                w.at(i)->close();
            }
        }
    });

    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(100, 50));
    view->customContextMenuRequested(QPoint(100, 50));
}

TEST_F(ut_application, treeNodeRightClicked)
{
    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "treeMenu") {
                timer->stop();
                w.at(i)->close();
                break;
            }
        }
     });
    timer->start(1000);

    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(100, 50));
    view->customContextMenuRequested(QPoint(100, 50));
}

TEST_F(ut_application, treePartitionNodeRightClicked)
{
    Stub stub;
    stub.set(ADDR(DMDbusHandler, onSetCurSelect), setCurSelect);

    CenterWidget *centerWidget = MainWindow::instance()->findChild<CenterWidget *>();
    MainSplitter *mainSplitter = centerWidget->findChild<MainSplitter *>();
    DeviceListWidget *deviceListWidget = mainSplitter->findChild<DeviceListWidget *>();
    DmTreeview *view = deviceListWidget->findChild<DmTreeview *>();
    QTest::qWait(1000);

    view->setRefreshItem(0, 0);
    QTest::qWait(1000);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        QWidgetList w = QApplication::topLevelWidgets();
        for (int i = 0; i < w.count(); i++) {
            if (w.at(i)->objectName() == "treeMenu") {
                timer->stop();
                w.at(i)->close();
                break;
            }
        }
     });
    timer->start(1000);

    QRect rect = view->visualRect(view->currentIndex());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());
    view->customContextMenuRequested(rect.center());
}

TEST_F(ut_application, titleWidget_noPartitionTable)
{
    typedef int (*fptr)(TitleWidget*);
    fptr foo = (fptr)(&PartitionDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();

    PartitionInfo oldInfo = DMDbusHandler::instance()->m_curPartitionInfo;
    DMDbusHandler::instance()->m_curPartitionInfo = deviceInfo_noPartitionTable().first().m_partition.at(0);

    titleWidget->showPartInfoWidget();

    DMDbusHandler::instance()->m_curPartitionInfo = oldInfo;
}

TEST_F(ut_application, titleWidget_formate)
{
    typedef int (*fptr)(TitleWidget*);
    fptr foo = (fptr)(&FormateDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);

    Stub stub2;
    stub2.set(ADDR(DMDbusHandler, getAllSupportFileSystem), getAllSupportFileSystem);

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();

    titleWidget->showFormateInfoWidget();
}

TEST_F(ut_application, titleWidget_mount)
{
    typedef int (*fptr)(TitleWidget*);
    fptr foo = (fptr)(&MountDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();

    titleWidget->showMountInfoWidget();
}

TEST_F(ut_application, titleWidget_unmount)
{
    typedef int (*fptr)(TitleWidget*);
    fptr foo = (fptr)(&UnmountDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();

    titleWidget->showUnmountInfoWidget();
}

TEST_F(ut_application, titleWidget_resize)
{
    typedef int (*fptr)(TitleWidget*);
    fptr foo = (fptr)(&ResizeDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);

    DTitlebar *titlebar = MainWindow::instance()->findChild<DTitlebar *>();
    QWidget *widget = titlebar->findChild<QWidget *>();
    TitleWidget *titleWidget = widget->findChild<TitleWidget *>();

    titleWidget->showResizeInfoWidget();
}



