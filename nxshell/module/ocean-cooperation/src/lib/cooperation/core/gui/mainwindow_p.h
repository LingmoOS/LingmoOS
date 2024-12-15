// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_P_H
#define MAINWINDOW_P_H

#include "widgets/workspacewidget.h"
#ifdef ENABLE_PHONE
#include "phone/phonewidget.h"
#endif

#include <QObject>
#include <QPainter>
#include <QMouseEvent>

class QSystemTrayIcon;
class QStackedLayout;

namespace cooperation_core {
class DeviceListWidget;
class MainWindow;
class BottomLabel;
class MainWindowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowPrivate(MainWindow *qq);
    virtual ~MainWindowPrivate();

    void initWindow();
    void initTitleBar();
    void initConnect();
    void moveCenter();

    void handleSettingMenuTriggered(int action);
    void setIP(const QString &ip);

public:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

public:
    MainWindow *q { nullptr };
    QStackedLayout *stackedLayout { nullptr };
    WorkspaceWidget *workspaceWidget { nullptr };
#ifdef ENABLE_PHONE
    PhoneWidget *phoneWidget { nullptr };
#endif
    bool leftButtonPressed { false };
    QPoint lastPosition;
    BottomLabel *bottomLabel { nullptr };
    QSystemTrayIcon *trayIcon { nullptr };
};

}   // namespace cooperation_core

#endif   // MAINWINDOW_P_H
