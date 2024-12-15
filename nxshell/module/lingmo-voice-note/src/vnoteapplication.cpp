// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteapplication.h"
#include "globaldef.h"
#include "setting.h"
#include "eventlogutils.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonObject>

/**
 * @brief VNoteApplication::VNoteApplication
 * @param argc
 * @param argv
 */
VNoteApplication::VNoteApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &VNoteApplication::onNewProcessInstance);
}

/**
 * @brief VNoteApplication::activateWindow
 */
void VNoteApplication::activateWindow()
{
    //Init Normal window at first time
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new VNoteMainWindow());

        m_qspMainWnd->setMinimumSize(MIN_WINDOWS_WIDTH, MIN_WINDOWS_HEIGHT);
        m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);

        QByteArray mainWindowSize = setting::instance()->getOption(VNOTE_MAINWND_SZ_KEY).toByteArray();

        if (!mainWindowSize.isEmpty()) {
            m_qspMainWnd->restoreGeometry(mainWindowSize);
        }

        //Should be called befor show
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());

        m_qspMainWnd->show();
        QJsonObject obj{
            {"tid", EventLogUtils::Start},
            {"version", QCoreApplication::applicationVersion()},
            {"mode", 1}
        };
        EventLogUtils::get().writeLogs(obj);
    } else {
        if (m_qspMainWnd->needShowMax()) {
            m_qspMainWnd->setWindowState(Qt::WindowMaximized);
        } else {
            m_qspMainWnd->setWindowState(Qt::WindowActive);
        }
        m_qspMainWnd->activateWindow();
    }
}

/**
 * @brief VNoteApplication::mainWindow
 * @return 主窗口
 */
VNoteMainWindow *VNoteApplication::mainWindow() const
{
    return m_qspMainWnd.get();
}

bool VNoteApplication::notify(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress){
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) && keyEvent->key() == Qt::Key_Z){
            if(m_isChageKeyEvent){
                qInfo() << "KeyPress(模拟): ctrl + shift + z";
                return DApplication::notify(obj,event);
            }else{
                qInfo() << "KeyPress(主动): ctrl + shift + z";
                return false;
            }
        }else if(keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_Y){
            qInfo() << "KeyPress: ctrl + Y";
            QKeyEvent tevent(QEvent::KeyPress,Qt::Key_Z,Qt::ShiftModifier | Qt::ControlModifier);
            m_isChageKeyEvent = true;
            return DApplication::notify(obj,&tevent);
        }
    }else if(event->type() == QEvent::KeyRelease){
        QKeyEvent *key = static_cast<QKeyEvent*>(event);
        if(key->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) && key->key() == Qt::Key_Z){
            qInfo() << "KeyRelease: ctrl + shift + z";
            m_isChageKeyEvent = false;
            return DApplication::notify(obj,event);
        }else if(key->modifiers() == Qt::ControlModifier && key->key() == Qt::Key_Y){
            qInfo() << "KeyRelease: ctrl + Y";
            QKeyEvent tevent(QEvent::KeyRelease,Qt::Key_Z,Qt::ShiftModifier | Qt::ControlModifier);
            m_isChageKeyEvent = false;
            return DApplication::notify(obj,&tevent);
        }
    }
    return DApplication::notify(obj,event);
}

/**
 * @brief VNoteApplication::onNewProcessInstance
 * @param pid
 * @param arguments
 */
void VNoteApplication::onNewProcessInstance(qint64 pid, const QStringList &arguments)
{
    Q_UNUSED(pid);
    Q_UNUSED(arguments);

    //TODO:
    //Parase comandline here

    activateWindow();
}

/**
 * @brief VNoteApplication::handleQuitAction
 */
void VNoteApplication::handleQuitAction()
{
    QEvent event(QEvent::Close);
    DApplication::sendEvent(mainWindow(), &event);
}
