/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 */

#ifndef LINGMO_PANEL_SHELL_H
#define LINGMO_PANEL_SHELL_H

#include <QObject>
#include <QMap>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <remote-config.h>
#include "screens-manager.h"

class QScreen;

namespace LingmoUIPanel {

class Panel;

class Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent=nullptr);
    void start();
protected:
    bool event(QEvent* e);
private Q_SLOTS:
    void onTabletModeChanged(bool);

private:
    void initRemoteConfig();
    void initDbus();
    void initPanelConfig();

private:
    bool m_isTabletMode {false};
    QMap<Screen*, Panel*> m_panels;
    RemoteConfig *m_remoteConfig = nullptr;
    QRemoteObjectHost m_qroHost;
    ScreensManager *m_screenManager = nullptr;
    //是否在所有屏幕上显示任务栏
    bool m_showPanelOnAllScreens = true;
    LingmoUIQuick::Config *m_panelConfig = nullptr;
    bool m_configDataChange = false;
};

} // LingmoUIPanel

#endif //LINGMO_PANEL_SHELL_H
