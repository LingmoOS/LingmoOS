// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printerapplication.h"
#include "reviselogger.h"

#include <DApplication>
#include <DLog>

#include <DApplicationSettings>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    MLogger loggerConf;
#endif

    int iRet = 0;
    /*需要在构造app之前设置这个属性,自适应屏幕缩放*/
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    DApplication a(argc, argv);

    if (0 != g_printerApplication->create()) {
        qCritical() << "Create printer application failed";
        return -1;
    }

    if (0 != g_printerApplication->launchWithMode(a.arguments())) {
        qCritical() << "Init printer application failed";
        return -2;
    }
    /*自动保存主题设置,需要在main里面设置*/
    DApplicationSettings saveTheme;

    iRet = a.exec();
    g_printerApplication->stop();

    return iRet;
}
