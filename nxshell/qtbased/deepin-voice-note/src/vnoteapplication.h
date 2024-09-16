// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEAPPLICATION_H
#define VNOTEAPPLICATION_H
#include "views/vnotemainwindow.h"

#include <DApplication>

DWIDGET_USE_NAMESPACE

class VNoteApplication : public DApplication
{
    Q_OBJECT
public:
    explicit VNoteApplication(int &argc, char **argv);
    //激活窗口
    void activateWindow();
    /*
     * Comment:
     *     In order put our config file to deepin's config
     * dir,we should call the this function in main window.
     * We can't get correct path when app's consturctor.
     * */
    VNoteMainWindow *mainWindow() const;

    bool notify(QObject *obj,QEvent* event);

signals:

public slots:
    //进程单例处理
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);

protected:
    //重写标题栏退出事件
    virtual void handleQuitAction() override;

protected:
    QScopedPointer<VNoteMainWindow> m_qspMainWnd {nullptr};
    bool m_isChageKeyEvent = false;
};

#endif // VNOTEAPPLICATION_H
