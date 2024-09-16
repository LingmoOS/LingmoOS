#ifndef MAINWINDOWTEST_H
#define MAINWINDOWTEST_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QtTest/QTest>
#include <QSharedMemory>

#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DWindowOptionButton>
#include <DMenu>
#include <DLog>
#include <DApplicationSettings>

#include "src/mainwindow.h"
#include "src/capplication.h"
#include "src/Settings.h"
#include "src/videowidget.h"
#include "src/imageitem.h"
#include "src/Settings.h"
#include "v4l2_devices.h"
#include "src/LPF_V4L2.h"
#include "src/majorimageprocessingthread.h"

DWIDGET_USE_NAMESPACE

//最后执行用例
class MainwindowTest : public ::testing::Test
{
public:
    MainwindowTest();
    ~MainwindowTest();
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }

protected:
    CMainWindow *mainwindow = nullptr;
    DIconButton *selectBtn = nullptr;//切换按钮
//    DButtonBoxButton *iconpixBtn = nullptr;//标题栏拍照按钮
//    DButtonBoxButton *iconVdBtn = nullptr;//标题栏录像按钮
    DSettingsDialog *settingdlg = nullptr;//设置界面
    DPushButton *PixVdBtn = nullptr;//缩略图按钮
    DPushButton *EndBtn = nullptr;//录制结束按钮
};

#endif // MAINWINDOWTEST_H
