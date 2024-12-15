// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compressorapplication.h"
#include "mainwindow.h"

#include <DCheckBox>
#include <DSuggestButton>

#include <QKeyEvent>
#include <QTimer>

CompressorApplication::CompressorApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

CompressorApplication::~CompressorApplication()
{

}

bool CompressorApplication::notify(QObject *watched, QEvent *event)
{
    if (QEvent::KeyPress == event->type()) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        int keyOfEvent = keyEvent->key();
        if (Qt::Key_Enter == keyOfEvent || Qt::Key_Return == keyOfEvent) {
            //checkbox响应"回车键"

            const QMetaObject *pObj = watched->metaObject();
            if (pObj && pObj->className() == QStringLiteral("QCheckBox")) {
                DCheckBox *checkBox = qobject_cast<DCheckBox *>(watched);
                // 模拟空格键按下事件
                QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
                QApplication::sendEvent(checkBox, &pressSpace);
                // 设置定时
                QTimer::singleShot(80, this, [checkBox]() {
                    // 模拟空格键松开事件
                    QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
                    QApplication::sendEvent(checkBox, &releaseSpace);
                });

                return true;
            }
        } else if (Qt::Key_Tab == keyOfEvent || Qt::Key_Backtab == keyOfEvent
                   || Qt::Key_Left == keyOfEvent || Qt::Key_Up == keyOfEvent) {
            QWidget *pWgt = activeWindow();
            MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作
            if (pWindow) {
                bool rs = pWindow->handleApplicationTabEventNotify(watched, keyEvent);
                if (rs) {
                    return rs;
                }
            }
        }
    }

    return DApplication::notify(watched, event);

}

void CompressorApplication::handleQuitAction()
{
    QWidget *pWgt = activeWindow();
    MainWindow *pWindow = qobject_cast<MainWindow *>(pWgt); //在对应的MainWindow操作

    // 处理点击退出事件
    if (pWindow)
        pWindow->handleQuit();
}
