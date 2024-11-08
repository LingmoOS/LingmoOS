/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#define SHADOW 6//阴影宽度
#define BUTTONRADIUS 0//按钮圆角
#define SHADOWALPHA 0.16//阴影透明度
#include "include/qtsingleapplication.h"
#include <QTranslator>
#include <QDebug>
#include <QGuiApplication>
#include <QApplication>
#include <QStringList>
#include <QStandardPaths>
#include <lingmo-log4qt.h>
#include <fcntl.h>
#include <QLibraryInfo>
#include "include/xatom-helper.h"
#include "mainwindow.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#if (QT_VERSION <= QT_VERSION_CHECK(5,15,0))
#include "dbusadaptor.h"
#endif
#include "include/qtsingleapplication.h"

// 拉起最小化窗口
void activeMainwindow()
{
    if(QGSettings::isSchemaInstalled(APPDATA))
    {
        QGSettings *p = new QGSettings(APPDATA);
        qDebug()<<"main.cpp activeMainwindow value:"<<p->get("mode").toString();
        p->set("mode","stat4");
        qDebug()<<"main.cpp after set value:"<<p->get("mode").toString();
    }
}
int main(int argc, char *argv[])
{
     bool isWayland=false;

//    init log module
    initLingmoUILog4qt("usb-boot-maker");
    QString parm = "--";
    if(argc == 2){
        parm = argv[1];
    }
//    高清屏幕自适应
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        isWayland = true;
    }

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QtSingleApplication a(argc,argv);
    a.setApplicationName(QObject::tr("lingmo-usb-creator"));
    qApp->setProperty("noChangeSystemFontSize",true);
    qApp->setWindowIcon(QIcon::fromTheme("lingmo-usb-creator"));
    //    TODO: 整合qt的标准翻译和自行定制的qm翻译
    //    标准对话框汉化(QT标准翻译
#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator();
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        a.installTranslator(translator);
    else
        qDebug() << "Failed to load Chinese translation file.";
#endif
    // 应用内翻译
    QTranslator app_trans;
    QString locale = QLocale::system().name();
    qInfo()<<__func__<<__LINE__<<locale;
    QString trans_path;

    if(QDir("/usr/share/lingmo-usb-creator/src/translations").exists()){
        trans_path = "/usr/share/lingmo-usb-creator/translations";
    }else{
        trans_path = qApp->applicationDirPath() + ":/src/translations";
    }
    if(locale == "zh_CN"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_zh_CN.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_zh_CN.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "bo_CN"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_bo_CN.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_bo_CN.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale =="mn_MN"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_mn.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_mn.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale =="zh_HK"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_zh_HK.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_zh_HK.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "ug_CN"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_ug.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_ug.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "ky_KG"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_kk.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_kk.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "kk_KZ"){
        if(!app_trans.load(":/src/translations/lingmo-usb-creator_ky.qm"))
        {
            qDebug()<<"Load translation file lingmo-usb-creator_ky.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    
    //sdk翻译
    QTranslator *transGui = new QTranslator();
    if (locale == "zh_CN")
    {
        if(transGui->load(":/translations/gui_zh_CN.qm"))
        {
            a.installTranslator(transGui);
        }
    } else if (locale == "bo_CN")
    {
        if(transGui->load(":/translations/gui_bo_CN.qm"))
        {
            a.installTranslator(transGui);
        }
    } else if(locale == "mn_MN")
    {
        if(transGui->load(":/translations/gui_mn.qm"))
        {
            a.installTranslator(transGui);
        }
    }
    else if(locale == "zh_HK")
    {
        if(transGui->load(":/translations/gui_zh_HK.qm"))
        {
            a.installTranslator(transGui);
        }
    }
    else if(locale == "ug_CN"){
        if(transGui->load(":/translations/gui_ug_CN.qm"))
        {
            a.installTranslator(transGui);
        }
    }
    else if(locale == "ky_KG"){
        if(transGui->load(":/translations/gui_ky_KG.qm"))
        {
            a.installTranslator(transGui);
        }
    }
    else if(locale == "kk_KZ"){
        if(transGui->load(":/translations/gui_kk_KZ.qm"))
        {
            a.installTranslator(transGui);
        }
    }
    
    
    if(a.isRunning()){
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1):"");
        qWarning()<<"lingmo-usb-creator is already running";
        return EXIT_SUCCESS;
    }else {
        MainWindow w(NULL,parm);
        w.handleIconClickedSub("");
        if(isWayland) {
            // 去除窗管标题栏，传入参数为QWidget *
            kdk::LingmoUIStyleHelper::self()->removeHeader(&w);
        }
        w.show();
        if(isWayland){
        // center window
        int sw=QGuiApplication::primaryScreen()->availableGeometry().width();
        int sh=QGuiApplication::primaryScreen()->availableGeometry().height();
        kdk::WindowManager::setGeometry(w.windowHandle(),QRect((sw-w.width())/2,(sh-w.height())/2,w.width(),w.height()));
        }
//        QObject::connect(&a,SIGNAL(messageReceived(const QString&)),&w,SLOT(handleIconClickedSub("")));
//        return a.exec();

        QObject::connect(&a,SIGNAL(messageReceived(const QString&)),&w,SLOT(slotActiveWindow(const QString&)));
        return a.exec();
    }
    return a.exec();
}
