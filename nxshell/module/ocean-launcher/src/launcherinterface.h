// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

class QString;
class QPoint;
class MenuWorker;
class QRect;

class LauncherInterface
{
public:
    virtual ~LauncherInterface() {}
    virtual void showLauncher() = 0;
    virtual void hideLauncher() = 0;
    virtual bool visible() = 0;

    virtual void launchCurrentApp() = 0;
    virtual void appendToSearchEdit(const char ch) = 0;
    virtual void moveCurrentSelectApp(const int key) = 0;
    virtual void regionMonitorPoint(const QPoint &point, int flag = 1) = 0;

    virtual void uninstallApp(const QString &desktopPath) = 0;
};


#endif // LAUNCHERINTERFACE_H
