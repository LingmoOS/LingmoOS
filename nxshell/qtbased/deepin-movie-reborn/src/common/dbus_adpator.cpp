// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_adpator.h"
#include "utils.h"

ApplicationAdaptor::ApplicationAdaptor(MainWindow *pMainWid)
    : QDBusAbstractAdaptor(pMainWid)
{
    initMember();

    m_pMainWindow = pMainWid;

    m_oldTime = QTime::currentTime();
}

//cppcheck 单元测试 wayland下再用
void ApplicationAdaptor::openFiles(const QStringList &listFiles)
{
    // 快速点击，播放不正常问题
    QTime current = QTime::currentTime();
    if (abs(m_oldTime.msecsTo(current)) > 800) {
        m_oldTime = current;
        m_pMainWindow->play(listFiles);
    }
}

//cppcheck 单元测试在用
bool ApplicationAdaptor::openFile(const QString &sFile)
{
    qDebug() << "dbus openFile.........." << sFile;
    QRegExp url_re("\\w+://");

    QUrl url;
    if (url_re.indexIn(sFile) == 0) {
        url = QUrl(sFile);
    } else {
        url = QUrl::fromLocalFile(sFile);
    }

//    QTime current = QTime::currentTime();
//    if (abs(m_oldTime.msecsTo(current)) > 800) {
//        m_oldTime = current;
        m_pMainWindow->play({url.toString()});
//    }
    Raise();//激活窗口
    return true;
}

void ApplicationAdaptor::Raise()
{
    qInfo() << "raise window from dbus";
    m_pMainWindow->showNormal();
    m_pMainWindow->raise();
    m_pMainWindow->activateWindow();
}

void ApplicationAdaptor::initMember()
{
    m_pMainWindow = nullptr;
}

QVariant ApplicationAdaptor::redDBusProperty(const QString &sService, const QString &sPath, const QString &sInterface, const char *pPropert)
{
    // 创建QDBusInterface接口
    QDBusInterface ainterface(sService, sPath,
                              sInterface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qInfo() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
    // 调用远程的value方法
    QList<QByteArray> q = ainterface.dynamicPropertyNames();
    QVariant v = ainterface.property(pPropert);
    return  v;
}

QVariant ApplicationAdaptor::redDBusMethod(const QString &sService, const QString &sPath, const QString &sInterface, const char *pMethod)
{
    // 创建QDBusInterface接口
    QDBusInterface ainterface(sService, sPath,
                              sInterface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qInfo() <<  "error:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
    // 调用远程的value方法
    QDBusReply<QDBusVariant> reply = ainterface.call(pMethod);
    if (reply.isValid()) {
        QVariant v(0) ;
        return  v;
    } else {
        qInfo() << "error1:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        QVariant v(0) ;
        return  v;
    }
}
