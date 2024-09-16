// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logapplication.h"
#include "logcollectormain.h"

#include <DWidgetUtil>

#include <QKeyEvent>
#include <QDebug>
#include <QEvent>

/**
 * @brief LogApplication::LogApplication 构造函数
 * @param argc 参数个数
 * @param argv 参数数组指针
 */
LogApplication::LogApplication(int &argc, char **argv): DApplication(argc, argv)
{

}

/**
 * @brief LogApplication::setMainWindow 设置mainwindow
 * @param iMainWindow mainwindow指针
 */
void LogApplication::setMainWindow(LogCollectorMain *iMainWindow)
{
    m_mainWindow = iMainWindow;
}

/**
 * @brief LogApplication::notify 重写全局notify虚函数
 * @param obj 收到事件的对象
 * @param evt 事件
 * @return 是否截获或继续传递
 */
bool LogApplication::notify(QObject *obj, QEvent *evt)
{
    switch (evt->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evt);
        //全局截获tab按键事件，调用mainwindow的对应函数去处理tab按键控件间切换
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            if (m_mainWindow) {
                bool rs =  m_mainWindow->handleApplicationTabEventNotify(obj, keyEvent);
                if (rs) {
                    return rs;
                }
            }
        }
        break;
    }
    default:
        break;
    }
    return  DApplication::notify(obj, evt) ;
}
