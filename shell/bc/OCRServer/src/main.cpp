/*
* Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
*
* Author: He MingYang Hao<hemingyang@uniontech.com>
*
* Maintainer: He MingYang <hemingyang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "ocrapplication.h"
#include "service/ocrinterface.h"
#include "service/dbusocr_adaptor.h"

#include <QWidget>
//#include <QLog>
//#include <DWindowManagerHelper>
//#include <DWidgetUtil>
//#include <DGuiApplicationHelper>
#include <QApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDesktopWidget>

//判断是否是wayland
bool CheckWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive))
        return true;
    else {
        return false;
    }
}

int main(int argc, char *argv[])
{


    //判断是否是wayland
    if (CheckWayland()) {
        //默认走xdgv6,该库没有维护了，因此需要添加该代码
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

//    DGuiApplicationHelper::setUseInactiveColorGroup(false);
//#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
//    DApplication::loadDXcbPlugin();
//    QScopedPointer<DApplication> app(new DApplication(argc, argv));
//#else
//    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
//#endif
    QApplication app(argc, argv);
    app.setOrganizationName("lingmoos");
    app.setApplicationName("lingmo-ocr");
//    app.setProductName(QObject::tr("OCR Tool"));
    app.setApplicationVersion("1.9.9");

//    Dtk::Core::DLogManager::registerConsoleAppender();
//    Dtk::Core::DLogManager::registerFileAppender();

    QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("lingmo-Ocr");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(dbusOption);
    cmdParser.process(app);

//    app->loadTranslator();


    OcrApplication* instance = OcrApplication::instance();
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.lingmo.Ocr")) {
        // 第一次启动
        // 注册Dbus服务和对象
        dbus.registerObject("/com/lingmo/Ocr", instance);
        // 初始化适配器
        new DbusOcrAdaptor(instance);

        if(QString(argv[1]) != "")
        {
            instance->openFile(QString(argv[1]));
        }

    } else {
            // 第二次运行此应用，
            // 调用DBus接口，处理交给第一次调用的进程
            // 本进程退退出
            OcrInterface *pOcr = new OcrInterface("com.lingmo.Ocr", "/com/lingmo/Ocr", QDBusConnection::sessionBus(), instance);
            qDebug() << __FUNCTION__ << __LINE__;
            pOcr->openFile(QString(argv[1]));
            delete pOcr;
            return 0;
        }



    return app.exec();
}


