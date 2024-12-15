// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrapplication.h"
#include "service/ocrinterface.h"
#include "service/dbusocr_adaptor.h"

#include <DWidget>
#include <DLog>
#include <DWindowManagerHelper>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>


DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{

    if (argc < 2) {
        qDebug() << "Cant open a null file";
        return 0;
    }

    DGuiApplicationHelper::setUseInactiveColorGroup(false);
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
#endif
    app->setOrganizationName("lingmo");
    app->setApplicationName("lingmo-ocr");
    app->setProductName(QObject::tr("OCR Tool"));
    app->setApplicationVersion("1.0");

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("lingmo-Ocr");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(dbusOption);
    cmdParser.process(*app);

    app->loadTranslator();


    OcrApplication instance;
    QDBusConnection dbus = QDBusConnection::sessionBus();

    if (dbus.registerService("com.lingmo.Ocr")) {
        // 第一次启动
        // 注册Dbus服务和对象
        dbus.registerObject("/com/lingmo/Ocr", &instance);
        // 初始化适配器
        new DbusOcrAdaptor(&instance);

        if (cmdParser.isSet(dbusOption)) {
            // 第一调用已 --dbus参数启动
            qDebug() << "dbus register waiting!";
            return app->exec();
        }

        instance.openFile(QString(argv[1]));
    } else {
        // 第二次运行此应用，
        // 调用DBus接口，处理交给第一次调用的进程
        // 本进程退退出
        OcrInterface *pOcr = new OcrInterface("com.lingmo.Ocr", "/com/lingmo/Ocr", QDBusConnection::sessionBus(), &instance);
        qDebug() << __FUNCTION__ << __LINE__;
        pOcr->openFile(QString(argv[1]));
        delete pOcr;
        return 0;
    }

    return app->exec();
}


