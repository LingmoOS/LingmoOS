// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Application.h"
#include "Utils.h"
#include "MainWindow.h"
#include "PageRenderThread.h"
#include "DocSheet.h"
#include "DBusObject.h"

#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QMouseEvent>

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    loadTranslator();
    setAttribute(Qt::AA_UseHighDpiPixmaps);
    setApplicationName("deepin-reader");
    setOrganizationName("deepin");
    //setWindowIcon(QIcon::fromTheme("deepin-reader"));     //耗时40ms
    setApplicationVersion(DApplication::buildVersion(APP_VERSION));
    setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin_reader");
    setApplicationDisplayName(tr("Document Viewer"));
    setApplicationDescription(tr("Document Viewer is a tool for reading document files, supporting PDF, DJVU, DOCX etc."));
    setProductIcon(QIcon::fromTheme("deepin-reader"));
}

Application::~Application()
{
    PageRenderThread::destroyForever();
    DBusObject::destory();
    qDebug() << __FUNCTION__ << "退出应用！";
}

void Application::emitSheetChanged()
{
    emit sigSetPasswdFocus();
}

void Application::handleQuitAction()
{
    QList<MainWindow *> list = MainWindow::m_list;

    //倒序退出,如果取消了则停止
    for (int i = list.count() - 1; i >= 0; --i) {
        if (!list[i]->handleClose(true))
            break;
    }
}

bool Application::notify(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        //让所有按钮响应回车 DPushButton不响应回车 DIconButton会默认响应
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        if ((object->inherits("QAbstractButton")) && (keyevent->key() == Qt::Key_Return || keyevent->key() == Qt::Key_Enter)) {
            QAbstractButton *pushButton = dynamic_cast<QAbstractButton *>(object);
            if (pushButton) {
                emit pushButton->clicked(!pushButton->isChecked());
                return true;
            }
        }

        //alt+m 模拟右击菜单
        if ((keyevent->modifiers() == Qt::AltModifier) && keyevent->key() == Qt::Key_M) {
            // 光标中心点
            QPoint pos = QPoint(static_cast<int>(qApp->inputMethod()->cursorRectangle().x() + qApp->inputMethod()->cursorRectangle().width() / 2),
                                static_cast<int>(qApp->inputMethod()->cursorRectangle().y() + qApp->inputMethod()->cursorRectangle().height() / 2));

            // QPoint(0,0) 表示无法获取光标位置
            if (pos != QPoint(0, 0)) {
                QMouseEvent event(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::NoButton, Qt::NoModifier);
                QCoreApplication::sendEvent(object, &event);
            }
        }
    } else if (event->type() == QEvent::FocusIn) {
        QFocusEvent *fe =  dynamic_cast<QFocusEvent *>(event);
        QWidget *widget = qobject_cast<QWidget *>(object);

        if (widget && fe && fe->reason() == Qt::ActiveWindowFocusReason && !widget->isTopLevel()
                && (widget->focusPolicy() & Qt::StrongFocus) != Qt::StrongFocus) {
            // 针对激活窗口所获得的焦点，为了避免被默认给到窗口内部的控件上，此处将焦点还给主窗口
            // 并且只设置一次
#ifndef NON_FIRST_ACTIVE
#define NON_FIRST_ACTIVE "_d_dtk_non_first_active_focus"
#endif
            QWidget *top_window = widget->topLevelWidget();

            if (top_window->isWindow() && !top_window->property(NON_FIRST_ACTIVE).toBool()) {
                top_window->setFocus();
                top_window->setProperty(NON_FIRST_ACTIVE, true);
            }
        }
    }

    //保存最后一次操作的文件
    //标签页操作触发：ZOrderChange
    //新窗口操作触发：WindowActivate
    if (event->type() == QEvent::ZOrderChange ||
            event->type() == QEvent::WindowActivate) {
        if (DocSheet *doc = qobject_cast<DocSheet *>(object)) {
            DocSheet::g_lastOperationFile = doc->filePath();
        }
    }

    return DApplication::notify(object, event);
}

