#include "src/mainwindow.h"
#include "src/imageitem.h"
#include "src/switchcamerabtn.h"

#include <QComboBox>
#include <QAction>
#include <QProcess>
#include <QShortcut>
#include <QMap>
#include <QVariantMap>
#include <QStandardPaths>

#include <DWindowMaxButton>
#include <DWindowMinButton>
#include <DWindowCloseButton>
#include <DPrintPreviewDialog>

#include "MainwindowTest.h"
#include "DButtonBox"
#include "datamanager.h"
#include "ac-deepin-camera-define.h"
#include "stub_function.h"
#include "stub.h"
#include "addr_pri.h"
#include "src/photorecordbtn.h"
#include "src/rollingbox.h"
#include "src/videowidget.h"

using namespace Dtk::Core;

ACCESS_PRIVATE_FUN(CMainWindow, void(const QString &serviceName, QVariantMap key2value, QStringList), onTimeoutLock);
ACCESS_PRIVATE_FUN(CMainWindow, void(), setSelBtnShow);
ACCESS_PRIVATE_FUN(CMainWindow, void(), onLocalTimeChanged);
ACCESS_PRIVATE_FUN(CMainWindow, void(bool bTrue), stopCancelContinuousRecording);
ACCESS_PRIVATE_FUN(CMainWindow, void(const QString &), onDirectoryChanged);
ACCESS_PRIVATE_FUN(CApplication, void(), QuitAction);
ACCESS_PRIVATE_FIELD(MajorImageProcessingThread, QAtomicInt, m_stopped);

ACCESS_PRIVATE_FIELD(CMainWindow, int, m_photoState);

ACCESS_PRIVATE_FIELD(CMainWindow, videowidget *, m_videoPre);

MainwindowTest::MainwindowTest()
{
    if (!mainwindow)
        mainwindow = CamApp->getMainWindow();

    if (!mainwindow->isVisible()) {
        mainwindow->show();
        mainwindow->activateWindow();
    }

    QTest::qWait(2000);
}

MainwindowTest::~MainwindowTest()
{
}

/**
 *  @brief Tab键切换
 */
TEST_F(MainwindowTest, TabChange)
{
    //tab键切换次数
    const int TabNumber = 10;
    //预览界面循环切换tab键
    for (int i = 0 ; i < TabNumber; i++) {
        QTest::keyPress(mainwindow, Qt::Key_Tab, Qt::NoModifier, 100);
    }
    QTest::mouseClick(mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 等比例拉伸
 */
#ifndef __mips__
TEST_F(MainwindowTest, equalScale)
{
    MajorImageProcessingThread *processThread = mainwindow->findChild<MajorImageProcessingThread *>("MajorThread");
    if(processThread) {
        mainwindow->resize(1280, 600);
        processThread->sigRenderYuv(true);
        QTest::qWait(1000);

        mainwindow->resize(800, 1080);
        processThread->sigRenderYuv(true);
        QTest::qWait(1000);

        mainwindow->resize(800, 600);
        processThread->sigRenderYuv(true);
        QTest::qWait(500);

        Dtk::Widget::moveToCenter(mainwindow);
    }
}
#endif

/**
 *  @brief 关闭拍照，录像
 */
TEST_F(MainwindowTest, stopCancelContinuousRecording)
{
    call_private_fun::CMainWindowstopCancelContinuousRecording(*mainwindow, false);
    call_private_fun::CMainWindowstopCancelContinuousRecording(*mainwindow, true);
}

/**
 *  @brief onTimeoutLock
 */
TEST_F(MainwindowTest, onTimeoutLock)
{
    QVariantMap key2value1;
    QStringList strList;
    call_private_fun::CMainWindowonTimeoutLock(*mainwindow, "", key2value1, strList);
    QVariantMap key2value2;
    key2value2.insert("Locked", true);
    mainwindow->setWayland(true);
    call_private_fun::CMainWindowonTimeoutLock(*mainwindow, "", key2value2, strList);
    QVariantMap key2value3;
    key2value3.insert("Locked", false);
    call_private_fun::CMainWindowonTimeoutLock(*mainwindow, "", key2value3, strList);
    mainwindow->setWayland(false);
}

/**
 *  @brief onLocalTimeChanged
 */
TEST_F(MainwindowTest, onLocalTimeChanged)
{
    call_private_fun::CMainWindowonLocalTimeChanged(*mainwindow);
}

/**
 *  @brief 上下左右键
 */
TEST_F(MainwindowTest, UpDownLeftRight)
{
    QTest::keyPress(mainwindow, Qt::Key_Tab, Qt::NoModifier, 100);
    QTest::keyPress(mainwindow, Qt::Key_Right, Qt::NoModifier, 100);
    QTest::keyPress(mainwindow, Qt::Key_Left, Qt::NoModifier, 100);
    QTest::keyPress(mainwindow, Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyPress(mainwindow, Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 空格键测试
 */
TEST_F(MainwindowTest, KeySpace)
{
    QTest::keyPress(mainwindow, Qt::Key_Space, Qt::NoModifier, 100);
}

/**
 *  @brief 快捷键
 */
TEST_F(MainwindowTest, RightMenushortCut)
{
    QShortcut *shortcutView = mainwindow->findChild<QShortcut *>(SHORTCUT_VIEW);
//    QShortcut *shortcutCopy = mainwindow->findChild<QShortcut *>(SHORTCUT_COPY);
//    QShortcut *shortcutDel = mainwindow->findChild<QShortcut *>(SHORTCUT_DELETE);
//    QShortcut *shortcutMenu = mainwindow->findChild<QShortcut *>(SHORTCUT_CALLMENU);
//    QShortcut *shortcutOpenFolder = mainwindow->findChild<QShortcut *>(SHORTCUT_OPENFOLDER);
//    QShortcut *shortcutPrint = mainwindow->findChild<QShortcut *>(SHORTCUT_PRINT);
    emit shortcutView->activated();
//    emit shortcutCopy->activated();
//    emit shortcutDel->activated();
//    emit shortcutMenu->activated();
//    emit shortcutOpenFolder->activated();
//    emit shortcutPrint->activated();
    QTest::mouseClick(mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 重置设置页面
 */
TEST_F(MainwindowTest, SettingDialogReset)
{
    mainwindow->settingDialog();
    DSettingsDialog *dialog = mainwindow->findChild<DSettingsDialog * >(SETTING_DIALOG);
    if (dialog == nullptr)
        return ;
    dialog->show();
    auto reset = dialog->findChild<QPushButton *>("SettingsContentReset");
    if (reset) {
        QTest::mouseMove(reset, QPoint(0, 0), 500);
        QTest::mousePress(reset, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
        QTest::mouseRelease(reset, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    }
    QTest::qWait(1000);
    dialog->hide();
    mainwindow->settingDialogDel();
}

/**
 *  @brief 设置页面
 */
TEST_F(MainwindowTest, SettingDialogShow)
{
    dc::Settings::get().settings()->setOption("outsetting.resolutionsetting.resolution", "1920x1080");
    mainwindow->settingDialog();
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 1);
    dc::Settings::get().settings()->sync();
    DSettingsDialog *dialog = mainwindow->findChild<DSettingsDialog * >(SETTING_DIALOG);
    if (dialog == nullptr)
        return ;
    dialog->show();
    //拍照路径
    DPushButton *openpicfilebtn = dialog->findChild<DPushButton *>(BUTTON_OPTION_PIC_LINE_EDIT);
    DDialog *invaliddlg = dialog->findChild<DDialog *>(OPTION_INVALID_DIALOG);
    DLineEdit *piclineedit = dialog->findChild<DLineEdit *>(OPTION_PIC_SELECTABLE_LINE_EDIT);
    if (piclineedit) {
        piclineedit->clear();
        piclineedit->setText(QDir::homePath() + "/Musics/");
        emit piclineedit->editingFinished();
        emit piclineedit->focusChanged(true);
    }
    if (openpicfilebtn) {
        emit openpicfilebtn->clicked();
        QTest::mouseMove(openpicfilebtn, QPoint(0, 0), 500);
        QTest::mousePress(openpicfilebtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
        QTest::mouseRelease(openpicfilebtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    }
    if (invaliddlg)
        emit invaliddlg->close();
    QTest::qWait(500);
    //视频路径
    DPushButton *openvideofilebtn = dialog->findChild<DPushButton *>(BUTTON_OPTION_VIDEO_LINE_EDIT);
    DLineEdit *videolineedit = dialog->findChild<DLineEdit *>(OPTION_VIDEO_SELECTABLE_LINE_EDIT);
    if (videolineedit) {
        videolineedit->clear();
        videolineedit->setText(QDir::homePath() + "/Musics/");
        emit videolineedit->editingFinished();
        emit videolineedit->focusChanged(true);
    }
    if (openvideofilebtn) {
        emit openvideofilebtn->clicked();
        QTest::mouseMove(openvideofilebtn, QPoint(0, 0), 500);
        QTest::mousePress(openvideofilebtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
        QTest::mouseRelease(openvideofilebtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    }
    if (invaliddlg)
        emit invaliddlg->close();
    QTest::qWait(500);
    dialog->hide();
    mainwindow->settingDialogDel();
}
/**
 *  @brief 切换分辨率
 */
TEST_F(MainwindowTest, ChangeResolution)
{
//    mainwindow->settingDialog();
//    DSettingsDialog *dialog = mainwindow->findChild<DSettingsDialog * >(SETTING_DIALOG);
//    dialog->show();
//    QComboBox *cmbox = dialog->findChild<QComboBox *>("OptionLineEdit");
//    mainwindow->update();
//    if (cmbox->count() > 2) {
//        cmbox->setCurrentIndex(1);
//        dc::Settings::get().settings()->sync();
//    }
//    mainwindow->settingDialogDel();
    dc::Settings::get().settings()->setOption(QString("outsetting.resolutionsetting.resolution"), 1);
    dc::Settings::get().settings()->setOption(QString("outsetting.resolutionsetting.resolution"), 0);
}

/**
 *  @brief 切换网格线类型
 */
TEST_F(MainwindowTest, ChangeGridType)
{
    dc::Settings::get().settings()->setOption(QString("base.photogrid.photogrids"), 0);
    dc::Settings::get().settings()->setOption(QString("base.photogrid.photogrids"), 1);
    QTest::qWait(1000);
    dc::Settings::get().settings()->setOption(QString("base.photogrid.photogrids"), 2);
    QTest::qWait(1000);
    dc::Settings::get().settings()->setOption(QString("base.photogrid.photogrids"), 0);
}

/**
 *  @brief 打开摄像头设置页面
 */
TEST_F(MainwindowTest, SettingDialogOpen)
{
    mainwindow->settingDialog();
    DSettingsDialog *dialog = mainwindow->findChild<DSettingsDialog * >(SETTING_DIALOG);
    dialog->show();
    QTest::qWait(1000);
    dialog->hide();
    mainwindow->settingDialogDel();
}



/**
 *  @brief 主题切换
 */
TEST_F(MainwindowTest, Themechange)
{
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    QTest::qWait(100);
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    QTest::qWait(100);
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    QTest::qWait(100);
}

/**
 *  @brief 测试设置界面
 */
TEST_F(MainwindowTest, settingDlgTest)
{
    QAction *actionSettings = mainwindow->findChild<QAction *>("SettingAction");
    emit actionSettings->triggered();
}

/**
 *  @brief 测试拍照
 */
TEST_F(MainwindowTest, TakePicture)
{
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(1000);
}

/**
 *  @brief 闪光灯测试拍照
 */
TEST_F(MainwindowTest, FlashLightTakePic)
{
    dc::Settings::get().setOption(QString("photosetting.photosdelay.photodelays"), 0);
    dc::Settings::get().setOption(QString("photosetting.Flashlight.Flashlight"), true);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(1000);
    dc::Settings::get().setOption(QString("photosetting.Flashlight.Flashlight"), false);
}

/**
 *  @brief 测试三连拍照
 */
TEST_F(MainwindowTest, ThreeContinuousShooting)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 1);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(3000);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
}

/**
 *  @brief 测试十连拍照
 */
TEST_F(MainwindowTest, TenContinuousShooting)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 2);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(10000);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
}

/**
 *  @brief 测试十连拍照取消
 */
TEST_F(MainwindowTest, TenContinuousShootingCancel)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 2);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(2000);
    emit pBtn->clicked();
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
}

/**
 *  @brief 测试十连拍照最小化
 */
TEST_F(MainwindowTest, TenContinuousShootingMinWnd)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 2);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(3000);
    //最小化
    DWindowMinButton *windowMinBtn = mainwindow->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    QTest::mouseClick(windowMinBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 200);
    QTest::qWait(100);
    //check snap end
    EXPECT_EQ(CMainWindow::photoNormal, access_private_field::CMainWindowm_photoState(*mainwindow));
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
    //恢复显示
    if (mainwindow->isMinimized()) {
        mainwindow->showNormal();
        mainwindow->show();
        mainwindow->activateWindow();
    }
}

/**
 *  @brief 测试延迟拍照
 */
TEST_F(MainwindowTest, DelayShooting)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 1);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(4000);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
}

/**
 *  @brief 测试延迟拍照取消
 */
TEST_F(MainwindowTest, DelayShootingCancel)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 2);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(1000);
    emit pBtn->clicked();
    dc::Settings::get().settings()->setOption(QString("photosetting.photosnumber.takephotos"), 0);
}

/**
 *  @brief 切换摄像头
 */
TEST_F(MainwindowTest, ChangeCamera)
{
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
}

/**
 *  @brief 切换摄像头
 */
TEST_F(MainwindowTest, InitFormatErrorChangeCamera)
{
    Stub_Function::resetSub(::camInit, ADDR(Stub_Function, camInitFormatError));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
    Stub_Function::resetSub(::camInit, ADDR(Stub_Function, camInit));
}

/**
 *  @brief 切换摄像头
 */
TEST_F(MainwindowTest, initNodeviceErrorChangeCamera)
{
    Stub_Function::resetSub(::camInit, ADDR(Stub_Function, camInitNoDevice));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);
    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
    Stub_Function::resetSub(::camInit, ADDR(Stub_Function, camInit));
}

/**
 *  @brief 没有相机时切换摄像头
 */
TEST_F(MainwindowTest, NoCameraChangeCamera)
{
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_0));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_3));
}

/**
 *  @brief dark下没有相机时切换摄像头
 */
TEST_F(MainwindowTest, DarkNoCameraChangeCamera)
{
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_0));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_3));
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
}

/**
 *  @brief 一个相机时切换摄像头
 */
TEST_F(MainwindowTest, OneCameraChangeCamera)
{
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_1));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
        dc::Settings::get().setBackOption("device", "/dev/video0");
    }
    QTest::qWait(1000);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_3));
}

/**
 *  @brief dark下一个相机时切换摄像头
 */
TEST_F(MainwindowTest, DarkOneCameraChangeCamera)
{
    DataManager::instance()->setdevStatus(CAM_CANNOT_USE);
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_1));
    QWidget *switchBtn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);

    if (switchBtn->isVisible()) {
        QTest::mouseClick(switchBtn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
    }
    QTest::qWait(1000);
    dc::Settings::get().setBackOption("device", "/dev/video0");
    Stub_Function::resetSub(::get_device_list, ADDR(Stub_Function, get_device_list_3));
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
}

/**
 *  @brief 录像，结束录像
 */
TEST_F(MainwindowTest, TakeVideo1)
{
    RollingBox *rollBox = mainwindow->findChild<RollingBox *>(MODE_SWITCH_BOX);
    //切换到录像
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
    emit rollBox->currentValueChanged(ActType::ActTakeVideo);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(10000);
    emit pBtn->clicked();
    //切换回拍照模式
    emit rollBox->currentValueChanged(ActType::ActTakePic);
    QTest::qWait(500);
}

/**
 *  @brief 延迟录像
 */
TEST_F(MainwindowTest, TakeVideoDelay)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 1);
    RollingBox *rollBox = mainwindow->findChild<RollingBox *>(MODE_SWITCH_BOX);
    //切换到录像
    emit rollBox->currentValueChanged(ActType::ActTakeVideo);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(5000);
    emit pBtn->clicked();
    emit rollBox->currentValueChanged(ActType::ActTakePic);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
}

/**
 *  @brief 延迟录像取消
 */
TEST_F(MainwindowTest, TakeVideoDelayCancel)
{
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 1);
    RollingBox *rollBox = mainwindow->findChild<RollingBox *>(MODE_SWITCH_BOX);
    //切换到录像
    emit rollBox->currentValueChanged(ActType::ActTakeVideo);
    photoRecordBtn *pBtn = mainwindow->findChild<photoRecordBtn *>(BUTTON_PICTURE_VIDEO);
    emit pBtn->clicked();
    QTest::qWait(1000);
    emit pBtn->clicked();
    emit rollBox->currentValueChanged(ActType::ActTakePic);
    dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), 0);
}

/**
 *  @brief 最大化
 */
TEST_F(MainwindowTest, MaxWindow)
{
    DWindowMaxButton *WindowMaxBtnOpt = mainwindow->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    QTest::mouseClick(WindowMaxBtnOpt, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 200);
    QTest::qWait(100);
    QTest::mouseClick(WindowMaxBtnOpt, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 200);
    QTest::qWait(100);
}

/**
 *  @brief 切换摄像机按钮测试
 */
TEST_F(MainwindowTest, SwitchCameraBtnEventTest)
{
    QWidget *btn = mainwindow->findChild<QWidget *>(CAMERA_SWITCH_BTN);
    QTest::mouseMove(btn, QPoint(5, 5), 500);
    QTest::qWait(100);
    QTest::mousePress(btn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    QTest::qWait(100);
    QTest::mouseRelease(btn, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    QTest::qWait(1000);
    //切换相机之后，恢复video0
    dc::Settings::get().setBackOption("device", "/dev/video0");
}

/**
 *  @brief 快捷方式测试
 */
TEST_F(MainwindowTest, ShortCurtTest)
{
    QTest::keyClick(mainwindow, Qt::Key_Return);
    QTest::qWait(100);
    QTest::keySequence(mainwindow, QKeySequence("Ctrl+Shift+/"));
    QTest::qWait(100);
    QTest::keySequence(mainwindow, QKeySequence(QKeySequence::Cancel));
    QTest::qWait(100);
    QTest::mouseClick(mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 200);
    QTest::qWait(100);
    QTest::keyClick(mainwindow, Qt::Key_Space);
    QTest::qWait(1000);

}

/**
 *  @brief 快捷方式测试
 */
TEST_F(MainwindowTest, HorizontalMirrorFalse)
{
    dc::Settings::get().setOption(QString("photosetting.mirrorMode.mirrorMode"), false);
    QTest::qWait(5000);
    dc::Settings::get().setOption(QString("photosetting.mirrorMode.mirrorMode"), true);
}

/**
 *  @brief 应用退出
 */
TEST_F(MainwindowTest, QuitAction)
{
    call_private_fun::CApplicationQuitAction(*CamApp);
    QTest::qWait(1000);
}

///**
// *  @brief 主窗口退出(这个case一定得放在最后)
// */
//TEST(ZTest, deleteMainWindow)
//{
//    CMainWindow* mainwindow = CamApp->getMainWindow();
//    delete mainwindow;
//    mainwindow = nullptr;
//}

