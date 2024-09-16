// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlefontapplication.h"
#include "views/dfontmgrmainwindow.h"
#include "views/dfquickinstallwindow.h"
#include "globaldef.h"
#include "utils.h"
#include "performancemonitor.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>

#include <sys/types.h>
#include <unistd.h>

DCORE_USE_NAMESPACE

/*************************************************************************
 <Function>      SingleFontApplication
 <Description>   构造函数-返回一个应用的单例对象
 <Author>        null
 <Input>
    <param1>     argc                  Description:标准函数
    <param2>     argv                  Description:标准函数
 <Return>        SingleFontApplication Description:返回一个应用的单例对象
 <Note>          null
*************************************************************************/
SingleFontApplication::SingleFontApplication(int &argc, char **argv)
    : DApplication(argc, argv)
    , m_qspMainWnd(nullptr)
    , m_qspQuickWnd(nullptr)
{
    connect(SignalManager::instance(), &SignalManager::finishFontInstall, this,
            &SingleFontApplication::onFontInstallFinished);
}

/*************************************************************************
 <Function>      ~SingleFontApplication
 <Description>   析构函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
SingleFontApplication::~SingleFontApplication()
{
}

/*************************************************************************
 <Function>      parseCmdLine
 <Description>   初始化命令行
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
bool SingleFontApplication::parseCmdLine(bool bAppExist)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Font Manager.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(*this);

    if (bAppExist) {
        QList<QVariant> fontInstallPathList;
        fontInstallPathList << parser.positionalArguments();
        QDBusInterface notification("com.deepin.FontManager", "/com/deepin/FontManager", "com.deepin.FontManager", QDBusConnection::sessionBus());
        QDBusMessage msg = notification.callWithArgumentList(QDBus::AutoDetect, "installFonts", fontInstallPathList);
    } else {
        //Clear old parameter
        if (!m_selectedFiles.isEmpty()) {
            m_selectedFiles.clear();
        }

        QStringList paraList = parser.positionalArguments();
        for (auto &it : paraList) {
            if (Utils::isFontMimeType(it)) {
                m_selectedFiles.append(it);
            }
        }

        if (paraList.size() > 0 && m_selectedFiles.size() == 0) {
            qDebug() << __FUNCTION__ << "invalid :" << paraList;
            return false;
        }

        qDebug() << __FUNCTION__ << m_selectedFiles;
    }

    return true;
}

/*************************************************************************
 <Function>      activateWindow
 <Description>   激活应用窗口
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void SingleFontApplication::activateWindow()
{
    //If quick install mode
    //不符合逻辑的代码
//    if (m_selectedFiles.size() < 0) {
//        qDebug() << "Active quick install window to install file:" << m_selectedFiles;

//        //Hide normal window in quick mode
//        if (nullptr != m_qspMainWnd.get()) {
//            //Force quit installtion
//            reinterpret_cast<DFontMgrMainWindow *>(
//                m_qspMainWnd.get())->forceNoramlInstalltionQuitIfNeeded();

//            m_qspMainWnd->hide();
//        }

//        //Init quick window at first time
//        if (nullptr == m_qspQuickWnd.get()) {
//            m_qspQuickWnd.reset(new DFQuickInstallWindow());
//            Dtk::Widget::moveToCenter(m_qspQuickWnd.get());

//            DFQuickInstallWindow *qw = qobject_cast<DFQuickInstallWindow *>(m_qspQuickWnd.get());

//            connect(qw, &DFQuickInstallWindow::requestShowMainWindow, this, [ = ](const QStringList & fileList) {
//                qDebug() << "requestShowMainWindow " << fileList;
//                if (nullptr == m_qspMainWnd.get()) {
//                    m_qspMainWnd.reset(new
//                                       DFontMgrMainWindow());
//                    int windowWidth = reinterpret_cast<DFontMgrMainWindow *>(
//                                          m_qspMainWnd.get())->getWinWidth();
//                    int windowHeight = reinterpret_cast<DFontMgrMainWindow *>(
//                                           m_qspMainWnd.get())->getWinHeight();

//                    m_qspMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
//                    if (DEFAULT_WINDOWS_WIDTH <= windowWidth && DEFAULT_WINDOWS_HEIGHT <= windowHeight) {
//                        m_qspMainWnd->resize(windowWidth, windowHeight);
//                    }

//                    Dtk::Widget::moveToCenter(m_qspMainWnd.get());

//                    m_qspMainWnd->show();
//                    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_qspMainWnd.get());
//                    Q_UNUSED(mw);
////                    mw->setFileList(fileList);
//                } else {
//                    m_qspMainWnd->setWindowState(Qt::WindowActive);
//                    m_qspMainWnd->activateWindow(); // Reactive main window
//                    //m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
//                }
//            });

//            m_qspQuickWnd->show();
//        } else {
//            m_qspQuickWnd->setWindowState(Qt::WindowActive);
//            m_qspQuickWnd->activateWindow(); // Reactive main window
//        }

//        QMetaObject::invokeMethod(m_qspQuickWnd.get(), "fileSelectedInSys", Qt::QueuedConnection,
//                                  Q_ARG(QStringList, m_selectedFiles));

//    } else {
    qDebug() << "++Active quick install window to install file:" << m_selectedFiles;
    //Hide quick window in normal mode
    if (nullptr != m_qspQuickWnd.get()) {
        m_qspQuickWnd->hide();
    }

    //Init Normal window at first time
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new DFontMgrMainWindow());
        int windowWidth = reinterpret_cast<DFontMgrMainWindow *>(
                              m_qspMainWnd.get())->getWinWidth();
        int windowHeight = reinterpret_cast<DFontMgrMainWindow *>(
                               m_qspMainWnd.get())->getWinHeight();
        //.toInt(&hWinDataStatus);
        m_qspMainWnd->setMinimumSize(MIN_WINDOWS_WIDTH, MIN_WINDOWS_HEIGHT);
        if(0 == windowWidth || 0 == windowHeight){
            m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
        }
        else if (MIN_WINDOWS_WIDTH <= windowWidth && MIN_WINDOWS_HEIGHT <= windowHeight) {
            m_qspMainWnd->resize(windowWidth, windowHeight);
        }

        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();

    } else {
        bool IsWindowMax = reinterpret_cast<DFontMgrMainWindow *>(
                               m_qspMainWnd.get())->getIsMaximized();
        if (IsWindowMax == true) {
            m_qspMainWnd->setWindowState(Qt::WindowMaximized);
        } else {
            m_qspMainWnd->setWindowState(Qt::WindowActive);
        }
        m_qspMainWnd->activateWindow();
        //m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    }
    //For: Drag files on task bar app icon
    //need start installtion flow
    if (m_selectedFiles.size() > 0) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "fileSelectedInSys", Qt::QueuedConnection,
                                  Q_ARG(QStringList, m_selectedFiles));
    }
//    }
    PerformanceMonitor::initializeAppFinish();
}

/*************************************************************************
 <Function>      slotBatchInstallFonts
 <Description>   批量安装字体响应
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void SingleFontApplication::slotBatchInstallFonts()
{
    qDebug() << "batch install fonts";
    m_selectedFiles << waitForInstallSet.toList();
    activateWindow();
    m_selectedFiles.clear();
    waitForInstallSet.clear();
}

/*************************************************************************
 <Function>      onFontInstallFinished
 <Description>   安装完成清空列表
 <Author>
 <Input>
    <param1>     fileList        Description:UnUsed
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void SingleFontApplication::onFontInstallFinished(const QStringList &fileList)
{
    Q_UNUSED(fileList);
    m_selectedFiles.clear();
}

/*************************************************************************
 <Function>      installFonts
 <Description>   批量安装字体响应
 <Author>
 <Input>
    <param1>     fontPathList    Description:待安装字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void SingleFontApplication::installFonts(const QStringList &fontPathList)
{
    PerformanceMonitor::installFontStart();
//    qDebug() << __FUNCTION__ << fontPathList;
    for (QString fontPath : fontPathList) {
        if (Utils::isFontMimeType(fontPath)) {
            /* bug#19081 UT00591 */
            waitForInstallSet.insert(fontPath);
        }
    }
    slotBatchInstallFonts();
}

