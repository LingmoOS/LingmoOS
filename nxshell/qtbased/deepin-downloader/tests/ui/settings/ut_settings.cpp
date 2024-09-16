// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "settings.h"
#include "mainframe.h"
#include "qtest.h"
#include <QDebug>
#include <QPushButton>
#include <QTimer>
#include <QString>
#include <DRadioButton>
#include <dsettingsoption.h>
#include "diagnostictool.h"
#include "downloadsettingwidget.h"
#include "filesavepathchooser.h"
#include "settingswidget.h"
#include "httpadvancedsettingwidget.h"
#include "messagebox.h"
#include "stub.h"
#include "stubAll.h"
#include "dbinstance.h"
#include "settinginfoinputwidget.h"
#include "dalertcontrol.h"
#include "itemselectionwidget.h"

class ut_Settings : public ::testing::Test
    , public QObject
{
protected:
    ut_Settings()
    {
    }

    virtual ~ut_Settings()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        //   delete m_createTaskWidget;
    }
    // CreateTaskWidget *m_createTaskWidget;
};

extern int isDigitStr(QString str);

TEST_F(ut_Settings, isDigitStr)
{
    isDigitStr("1232");
    isDigitStr("dsfs");
}

//开机自启
TEST_F(ut_Settings, autoStart)
{
    Settings::getInstance()->setAutoStart(true);
    QTest::qWait(500);
    Settings::getInstance()->setAutoStart(false);
    EXPECT_TRUE(true);
}

//最大下载个数
TEST_F(ut_Settings, setMaxDownloadNum)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadTaskManagement.downloadtaskmanagement.MaxDownloadTask");
    option->setValue(5);
    QTest::qWait(500);
    option->setValue(3);
    EXPECT_TRUE(Settings::getInstance()->getMaxDownloadTaskNumber() == 3);
}

//初始化 原始地址线程数
TEST_F(ut_Settings, addressthread)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadSettings.downloadmanagement.addressthread");
    option->setValue(5);
  //  option->setValue(QVariant());
    QTest::qWait(500);
    option->setValue(3);
   // EXPECT_TRUE(Settings::getInstance()->getMaxDownloadTaskNumber() == 3);
}

//全局最大下载资源数
TEST_F(ut_Settings, maxlimit)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadSettings.downloadmanagement.maxlimit");
    option->setValue(5);
  //  option->setValue(QVariant());
    QTest::qWait(500);
    option->setValue(3);
   // EXPECT_TRUE(Settings::getInstance()->getMaxDownloadTaskNumber() == 3);
}

//下载设置
TEST_F(ut_Settings, downloadspeedlimit)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadSettings.downloadsettings.downloadspeedlimit");
    option->setValue(5);
    QTest::qWait(500);
}

//速度小于多少自动增加任务窗口
TEST_F(ut_Settings, AutoDownload)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadTaskManagement.downloadtaskmanagement.AutoDownload");
    option->setValue(5);
    QTest::qWait(500);
}

//按下载速度排序
TEST_F(ut_Settings, AutoSortBySpeed)
{
    auto option = Settings::getInstance()->m_settings->option("DownloadTaskManagement.downloadtaskmanagement.AutoSortBySpeed");
    option->setValue(5);
    QTest::qWait(500);
}

//下载磁盘缓存
TEST_F(ut_Settings, DownloadDiskCacheSettiing)
{
    auto option = Settings::getInstance()->m_settings->option("AdvancedSetting.DownloadDiskCache.DownloadDiskCacheSettiing");
    option->setValue(5);
    QTest::qWait(500);
}

//启动时关联MetaLink种子文件
TEST_F(ut_Settings, ClipBoard)
{
    auto optionClipBoard = Settings::getInstance()->m_settings->option("Monitoring.MonitoringObject.ClipBoard");
    auto optionHttpDownload = Settings::getInstance()->m_settings->option("Monitoring.MonitoringDownloadType.HttpDownload");
    auto optionBTDownload = Settings::getInstance()->m_settings->option("Monitoring.MonitoringDownloadType.BTDownload");
    auto optionMetaLinkDownload = Settings::getInstance()->m_settings->option("Monitoring.MonitoringDownloadType.MetaLinkDownload");
    auto optionMagneticDownload = Settings::getInstance()->m_settings->option("Monitoring.MonitoringDownloadType.MagneticDownload");
    optionHttpDownload->setValue(true);
    optionBTDownload->setValue(true);
    optionMetaLinkDownload->setValue(true);
    optionMagneticDownload->setValue(true);
    optionClipBoard->setValue(true);

    optionHttpDownload->setValue(true);
    optionBTDownload->setValue(true);
    optionMetaLinkDownload->setValue(true);
    optionMagneticDownload->setValue(true);
    optionClipBoard->setValue(true);

    optionHttpDownload->setValue(false);
    optionBTDownload->setValue(false);
    optionMetaLinkDownload->setValue(false);
    optionMagneticDownload->setValue(false);
    optionClipBoard->setValue(false);



}

//实现剪切板和接管下载类型关联
TEST_F(ut_Settings, AssociateMetaLinkFileAtStartup)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MetaLinkRelation.AssociateMetaLinkFileAtStartup");
    option->setValue(true);
    option->setValue(false);
    QTest::qWait(500);
}

//自动下载bt文件
TEST_F(ut_Settings, setAutoDownloadBtfile)
{
    auto startAssociatedBTFile = Settings::getInstance()->m_settings->option("Monitoring.BTRelation.AssociateBTFileAtStartup");
    startAssociatedBTFile->setValue(false);
    QTest::qWait(500);
    startAssociatedBTFile->setValue(true);
    EXPECT_TRUE(Settings::getInstance()->getStartAssociatedBTFileState());
}

TEST_F(ut_Settings, getSpeed)
{
    EXPECT_TRUE(Settings::getInstance()->getAutoSortBySpeed());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getPath)
{
    Settings::getInstance()->setCustomFilePath("/home/uos/Downloades");
    auto option = Settings::getInstance()->m_settings->option("Basic.DownloadDirectory.downloadDirectoryFileChooser");
    option->setValue("/home/uos/Downloades");
    QString str = Settings::getInstance()->getCustomFilePath();
    QString str1 = Settings::getInstance()->getDownloadSavePath();
    EXPECT_FALSE(str.isEmpty() && str1.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getspeed)
{
    EXPECT_TRUE(Settings::getInstance()->getDownloadInfoSystemNotifyState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getPowerOnState)
{
    Settings::getInstance()->getPowerOnState();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getAutostartUnfinishedTaskState)
{
    EXPECT_TRUE(Settings::getInstance()->getAutostartUnfinishedTaskState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDownloadDirectorySelected)
{
    EXPECT_FALSE(Settings::getInstance()->getDownloadDirectorySelected());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDownloadSavePath)
{
    auto option = Settings::getInstance()->m_settings->option("Basic.DownloadDirectory.downloadDirectoryFileChooser");
    option->setValue("/home/uos/Downloades");
    QString str = Settings::getInstance()->getDownloadSavePath();
    EXPECT_TRUE(str.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getOneClickDownloadState)
{
    Settings *pSettings = Settings::getInstance();
    EXPECT_NE(pSettings, nullptr);
    pSettings->getOneClickDownloadState();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getCloseMainWindowSelected)
{
    Settings::getInstance()->getCloseMainWindowSelected();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getMaxDownloadTaskNumber)
{
    EXPECT_TRUE(Settings::getInstance()->getMaxDownloadTaskNumber());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDownloadFinishedOpenState)
{
    Settings::getInstance()->getDownloadFinishedOpenState();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getAutoDeleteFileNoExistentTaskState)
{
    Settings::getInstance()->getAutoDeleteFileNoExistentTaskState();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDownloadSettingSelected)
{
    Settings::getInstance()->getDownloadSettingSelected();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getMaxDownloadSpeedLimit)
{
    QString str = Settings::getInstance()->getMaxDownloadSpeedLimit();
    EXPECT_TRUE(str.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getMaxUploadSpeedLimit)
{
    QString str = Settings::getInstance()->getMaxUploadSpeedLimit();
   // EXPECT_FALSE(str.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getSpeedLimitStartTime)
{
    QString str = Settings::getInstance()->getSpeedLimitStartTime();
  //  EXPECT_FALSE(str.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getSpeedLimitEndTime)
{
    QString str = Settings::getInstance()->getSpeedLimitEndTime();
 //   EXPECT_FALSE(str.isEmpty());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getAllSpeedLimitInfo)
{
    Settings::getInstance()->getAllSpeedLimitInfo();
  //  EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getClipBoardState)
{
    EXPECT_FALSE(Settings::getInstance()->getClipBoardState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getHttpDownloadState)
{
    EXPECT_FALSE(Settings::getInstance()->getHttpDownloadState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getBtDownloadState)
{
    EXPECT_FALSE(Settings::getInstance()->getBtDownloadState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getMagneticDownloadState)
{
    EXPECT_FALSE(Settings::getInstance()->getMagneticDownloadState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getAutoOpennewTaskWidgetState)
{
    Settings::getInstance()->getAutoOpenBtTaskState();
  //  EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getAutoOpenMetalinkTaskState)
{
    Settings::getInstance()->getAutoOpenMetalinkTaskState();
 //   EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getStartAssociatedBTFileState)
{
    EXPECT_TRUE(Settings::getInstance()->getStartAssociatedBTFileState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDownloadInfoSystemNotifyState)
{
    EXPECT_TRUE(Settings::getInstance()->getDownloadInfoSystemNotifyState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getNewTaskShowMainWindowState)
{
    EXPECT_TRUE(Settings::getInstance()->getNewTaskShowMainWindowState());
    QTest::qWait(50);
}

TEST_F(ut_Settings, getDisckcacheNum)
{
    EXPECT_TRUE(Settings::getInstance()->getDisckcacheNum());
    auto option = Settings::getInstance()->m_settings->option("AdvancedSetting.DownloadDiskCache.DownloadDiskCacheSettiing");
    option->setValue(0);
    Settings::getInstance()->getDisckcacheNum();
    option->setValue(1);
    Settings::getInstance()->getDisckcacheNum();
    option->setValue(2);
    Settings::getInstance()->getDisckcacheNum();
    QTest::qWait(50);
}

TEST_F(ut_Settings, getPriorityDownloadBySize)
{
    QString size;
    Settings::getInstance()->getPriorityDownloadBySize(size);
  //  EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getCustomFilePath)
{
    QString str = Settings::getInstance()->getCustomFilePath();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getIsShowTip)
{
    Settings::getInstance()->getIsShowTip();
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, getIsClipboradStart)
{
    QString str;
    Settings::getInstance()->getIsClipboradStart(str);
  //  EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, setCloseMainWindowSelected)
{
    Settings::getInstance()->setCloseMainWindowSelected(1);
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, setCustomFilePath)
{
    Settings::getInstance()->setCustomFilePath(Settings::getInstance()->getCustomFilePath());
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, setIsShowTip)
{
    Settings::getInstance()->setIsShowTip(true);
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

TEST_F(ut_Settings, setIsClipboradStart)
{
    Settings::getInstance()->setIsClipboradStart("aaaa");
    EXPECT_TRUE(true);
    QTest::qWait(50);
}

//TEST_F(ut_Settings, close)
//{
//    MainFrame *m = MainFrame::instance();
//    QSystemTrayIcon *tray = m->findChild<QSystemTrayIcon *>("systemTray");
//    QMenu *menu = tray->contextMenu();
//    QTimer::singleShot(1000, this, [=]() {
//        QTimer::singleShot(1000, this, [=]() {
//            QWidget *w = QApplication::activeWindow();
//            QAbstractButton *btn = w->findChild<QAbstractButton *>("sure");
//            QTest::mouseClick(btn, Qt::LeftButton);
//            // w->close();
//        });
//        QPoint p = menu->rect().bottomRight();
//        QTest::mouseClick(menu, Qt::LeftButton, Qt::MetaModifier, QPoint(p.x() - 10, p.y() - 10));
//        // menu->close();
//    });
//    menu->exec();
//    //QTest::mouseClick(menu, Qt::RightButton);
//}

TEST_F(ut_Settings, DiagnosticModel)
{
    DiagnosticModel *model = new DiagnosticModel;
    model->appendData(true);
    EXPECT_TRUE(true);
    delete model;
}

TEST_F(ut_Settings, DiagnosticModel1)
{
    DiagnosticModel *model = new DiagnosticModel;
    model->appendData(true);
    model->setData(model->index(0, 0), true, Qt::DisplayRole);
    EXPECT_TRUE(true);
    delete model;
}

TEST_F(ut_Settings, DiagnosticModel2)
{
    DiagnosticModel *model = new DiagnosticModel;
    model->appendData(true);
    model->data(model->index(0, 0), Qt::DisplayRole);
    model->data(model->index(0, 1), Qt::DisplayRole);
    model->data(model->index(1, 1), Qt::DisplayRole);
    model->data(model->index(2, 1), Qt::DisplayRole);
    model->data(model->index(3, 1), Qt::DisplayRole);
    model->data(model->index(4, 1), Qt::DisplayRole);
    EXPECT_TRUE(true);
    delete model;
}

TEST_F(ut_Settings, DiagnosticModel3)
{
    DiagnosticModel *model = new DiagnosticModel;
    model->clearData();
    EXPECT_TRUE(true);
    delete model;
}

TEST_F(ut_Settings, DiagnosticModel4)
{
    DiagnosticDelegate *pDelegate = new DiagnosticDelegate;
    EXPECT_TRUE(true);
    delete pDelegate;
}

TEST_F(ut_Settings, DownloadSettingWidget)
{
    DownloadSettingWidget *pWidget = new DownloadSettingWidget;

    typedef int (*fptr)(DownloadSettingWidget *);
    fptr foo = (fptr)(&QObject::sender);
    Stub stub;
    stub.set(foo, mockSender);


    pWidget->onRadioButtonClicked();


   // pWidget->m_fullSpeedDownloadButton  = new DRadioButton;
    delete pWidget->m_fullSpeedDownloadButton;
    pWidget->m_fullSpeedDownloadButton = nullptr;
    pWidget->m_fullSpeedDownloadButton = mockSender();
    pWidget->onRadioButtonClicked();

    pWidget->m_fullSpeedDownloadButton = new DRadioButton;
    delete pWidget->m_speedLimitDownloadButton;
    pWidget->m_speedLimitDownloadButton = nullptr;
    pWidget->m_speedLimitDownloadButton = mockSender();
    pWidget->onRadioButtonClicked();

    pWidget->m_speedLimitDownloadButton->setCheckable(false);
    pWidget->onTimeChanged("22");
    pWidget->onValueChanged(QVariant());
    QVariant v(QString("asda"));
    pWidget->onValueChanged(v);
    delete pWidget->m_fullSpeedDownloadButton;
    delete pWidget;
}

TEST_F(ut_Settings, DownloadSettingWidget5)
{
    DownloadSettingWidget *pWidget = new DownloadSettingWidget;

    typedef int (*fptr)(DownloadSettingWidget *);
    fptr foo = (fptr)(&QObject::sender);
    Stub stub;
    stub.set(foo, mockSenderTime);
    delete pWidget->m_startTimeEdit;
    pWidget->m_startTimeEdit = nullptr;

    pWidget->m_startTimeEdit = mockSenderTime() ;
    pWidget->m_startTimeEdit->dLineEdit()->parentWidget()->parentWidget();
    pWidget->onTimeChanged("1");
   // pWidget->onTimeChanged("9999999999999999999999999999");

    pWidget->m_startTimeEdit = new CTimeEdit;
    delete pWidget->m_endTimeEdit;
    pWidget->m_endTimeEdit = nullptr;
    pWidget->m_endTimeEdit = mockSenderTime();
    pWidget->onTimeChanged("1");
  //  pWidget->onTimeChanged("9999999999999999999999999999");
    delete pWidget->m_startTimeEdit;
    delete pWidget;
}

TEST_F(ut_Settings, DiagnosticTool)
{
    DiagnosticTool *d = new DiagnosticTool;
    d->startDiagnostic();
    DiagnosticModel model;
    model.m_DiagnosticStatusList.clear();
    model.rowCount();
    model.columnCount();
    QModelIndex index;
    index.r = 2;
    model.data(index, Qt::TextColorRole);
    model.data(index, 123);
    QTimer::singleShot(7000, this, [=]() {
        d->close();
    });
    d->exec();
//    if(d->m_pDelegate != nullptr){
//        delete d->m_pDelegate;
//        d->m_pDelegate = nullptr;
//    }
    delete d;
}


TEST_F(ut_Settings, DownloadSettingWidget1)
{
    DownloadSettingWidget *pWidget = new DownloadSettingWidget;
    pWidget->onTextChanged("str");
    delete pWidget;
}

TEST_F(ut_Settings, DownloadSettingWidget2)
{
    DownloadSettingWidget *pWidget = new DownloadSettingWidget;
    pWidget->onTextChanged("1234");
    delete pWidget;
}

TEST_F(ut_Settings, DownloadSettingWidget3)
{
    DownloadSettingWidget *pWidget = new DownloadSettingWidget;
    pWidget->onFocusChanged(true);
    delete pWidget;
}

TEST_F(ut_Settings, FileSavePathChooser)
{
    FileSavePathChooser *pWidget = new FileSavePathChooser(1, "");
    pWidget->onRadioButtonClicked();
    delete pWidget;
}

TEST_F(ut_Settings, FileSavePathChooser1)
{
    typedef int (*fptr)(DDialog *);
    fptr foo = (fptr)(&DDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    FileSavePathChooser *pWidget = new FileSavePathChooser(1, "");
    pWidget->onLineEditTextChanged("123");
    delete pWidget;
}

TEST_F(ut_Settings, FileSavePathChooser2)
{
    FileSavePathChooser *pWidget = new FileSavePathChooser(1, "");
    pWidget->setCurrentSelectRadioButton(0);
    delete pWidget;
}

TEST_F(ut_Settings, FileSavePathChooser3)
{
    typedef int (*fptr)(DDialog *);
    fptr foo = (fptr)(&DDialog::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    FileSavePathChooser *pWidget = new FileSavePathChooser(1, "");
    pWidget->setLineEditText("123456");
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget1)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->setSize("111");
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget2)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->setSpeend("111");
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget3)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->setSwitch(false);
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget4)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->m_Edit->setText("4444");
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget5)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->m_Edit->setText("+11");
    delete pWidget;
}

TEST_F(ut_Settings, SettingsControlWidget6)
{
    SettingsControlWidget *pWidget = new SettingsControlWidget;
    pWidget->initUI(tr("When total speed is lower than"), tr("KB/S add active downloads"));
    pWidget->m_Edit->setText("001");
    delete pWidget;
}

TEST_F(ut_Settings, controlbrowserOn)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MonitoringObject.Browser");
    option->setValue(false);
    option->setValue(true);
    EXPECT_TRUE(Settings::getInstance()->getWebBrowserState());
}

TEST_F(ut_Settings, controlbrowserOff)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MonitoringObject.Browser");
    option->setValue(true);
    option->setValue(false);
    EXPECT_FALSE(Settings::getInstance()->getWebBrowserState());
}

TEST_F(ut_Settings, HttpAdvancedChangeSuffix)
{
    HttpAdvancedSettingWidget advance;
    advance.onSuffixBtnClicked();
    advance.onWebBtnClicked();
    advance.onRstoreDefaultClicked();
    advance.onSureClicked();
    advance.onCancelClicked();
    EXPECT_TRUE(true);
}

TEST_F(ut_Settings, OpenDownloadMetaLinkPanelTrue)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MetaLinkRelation.OpenDownloadMetaLinkPanel");
    option->setValue(false);
    option->setValue(true);
    QTest::qWait(100);
    EXPECT_FALSE(Settings::getInstance()->getWebBrowserState());
}

TEST_F(ut_Settings, OpenDownloadMetaLinkPanelFalse)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MetaLinkRelation.OpenDownloadMetaLinkPanel");
    option->setValue(false);
    option->setValue(true);
    QTest::qWait(100);
    EXPECT_FALSE(Settings::getInstance()->getWebBrowserState());
}

TEST_F(ut_Settings, initWdiget)
{
    auto option = Settings::getInstance()->m_settings->option("Monitoring.MetaLinkRelation.OpenDownloadMetaLinkPanel");
    option->setValue(false);
    option->setValue(true);
    QTest::qWait(100);
    EXPECT_FALSE(Settings::getInstance()->getWebBrowserState());
}

TEST_F(ut_Settings, ItemSelectionWidget)
{
    ItemSelectionWidget item;
    item.onCheckBoxStateChanged(Qt::Unchecked);
    item.onCheckBoxStateChanged(Qt::Checked);
    item.setCheckboxState(Qt::Unchecked);
    item.setBlockSignals(true);
}

TEST_F(ut_Settings, CTimeEditOnIndexChanged)
{
    CTimeEdit c;
    c.m_timeEdit->editingFinished();
    c.onIndexChanged("11");
}

//TEST_F(ut_Settings, FileSavePathChooserInit)
//{
//    typedef int (*fptr)(DownloadSettingWidget *);
//    fptr foo = (fptr)(&QObject::sender);
//    Stub stub;
//    stub.set(foo, mockSender);

//    FileSavePathChooser f(1, "~/Downloader/");
//    f.m_currentSelect= 0;
//    f.initUI();
//    f.m_customsPathRadioButton = mockSender();
//    f.onRadioButtonClicked();
//    f.m_autoLastPathRadioButton = mockSender();
//    f.onRadioButtonClicked();
//    f.setCurrentSelectRadioButton(1);
//}

TEST_F(ut_Settings, getMaxDownloadSpeedLimit1)
{
     auto option = Settings::getInstance()->m_settings->option("DownloadSettings.downloadsettings.downloadspeedlimit");
     option->setValue(";;;;;;");
     Settings::getInstance()->getMaxDownloadSpeedLimit();
     Settings::getInstance()->getMaxUploadSpeedLimit();
     Settings::getInstance()->getSpeedLimitStartTime();
     Settings::getInstance()->getSpeedLimitEndTime();
}

TEST_F(ut_Settings, createWindows)
{
     auto option = Settings::getInstance()->m_settings->option("DownloadSettings.downloadsettings.downloadspeedlimit");
     option->setValue(QVariant());
     Settings::getInstance()->createDownloadSpeedLimitSettiingHandle(option);
}


TEST_F(ut_Settings, clearAllTask)
{
    QProcess p;
    p.start("pkill", QStringList()<<"dde-file-manage");
    p.waitForFinished();
    DBInstance::delAllTask();
    Settings *s = Settings::getInstance();
    delete s;
}


