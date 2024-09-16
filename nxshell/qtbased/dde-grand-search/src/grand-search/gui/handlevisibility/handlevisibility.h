// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HANDLEVISIBILITY_H
#define HANDLEVISIBILITY_H

#include <com_deepin_sessionmanager.h>

#include <DRegionMonitor>

#include <QObject>

namespace GrandSearch {

class MainWindow;

class HandleVisibility : public QObject
{
    Q_OBJECT
public:
    explicit HandleVisibility(MainWindow *mainWindow, QObject *parent);

public slots:
    void onApplicationStateChanged(const Qt::ApplicationState state);
    void onLockedChanged(const bool locked);
    void onCloseWindow();

    void registerRegion(const bool isRegister);
    void regionMousePress(const QPoint &p, const int flag);

private:
    void init();
private:
    MainWindow *m_mainWindow = nullptr;
    Dtk::Gui::DRegionMonitor *m_regionMonitor = nullptr;
    __SessionManager *m_sessionManagerInter = nullptr;
};

}

#endif // HANDLEVISIBILITY_H
