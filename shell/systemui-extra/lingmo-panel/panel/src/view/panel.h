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

#ifndef LINGMO_PANEL_PANEL_H
#define LINGMO_PANEL_PANEL_H

#include <QQuickView>
#include <QScreen>
#include <QRect>
#include <QTimer>
#include <types.h>
#include <window-helper.h>
#include <island-view.h>
#include "screens-manager.h"
#include "general-config-define.h"
#include "common.h"

class QAction;

namespace LingmoUIPanel {

class Panel : public LingmoUIQuick::IslandView
{
    Q_OBJECT
    Q_PROPERTY(bool lockPanel READ lockPanel NOTIFY lockPanelChanged)
    Q_PROPERTY(bool enableCustomSize READ enableCustomSize NOTIFY enableCustomSizeChanged)
public:
    Panel(Screen *screen, const QString &id, QWindow *parent = nullptr);
    void setPanelScreen(Screen *screen);

    bool lockPanel() const;
    bool enableCustomSize() const;
    Q_INVOKABLE void customPanelSize(int distance);
    Q_INVOKABLE void changeCursor(const QString &type);
    Q_INVOKABLE void changePanelPos(LingmoUIQuick::Types::Pos pos);
public Q_SLOTS:
    /**
     * @brief primaryScreenChanged 主屏改变时修改托盘的显示状态
     * @param screen 改变之后的主屏
     * @return
     */
    void primaryScreenChanged(Screen *screen);
Q_SIGNALS:
    void lockPanelChanged();
    void enableCustomSizeChanged();

protected:
    bool event(QEvent *e) override;

private Q_SLOTS:
    void onScreenGeometryChanged(const QRect &geometry);
    void setPosition(LingmoUIQuick::Types::Pos position);
    void setPanelSize(int size);
    void setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy);
    void setAutoHide(bool autoHide);

    void resetWorkArea(bool cut);
    void setHidden(bool hidden);
    void activeHideTimer(bool active = true);

    void setLockPanel(bool locked, bool noEmit = false);
    // widget opt
    void widgetActionTriggered(const QAction *);
    void disableWidget(const QString &id, bool disable);
    void onContainerActiveChanged();
private:
    void initConfig();
    void initPanelConfig();
    void loadWidgetsConfig();
    void updateGeometry();
    void refreshRect();

    void loadActions();
    void initSizeAction();
    void updateSizeAction();
    void initPositionAction();
    void updatePositionAction();
    void initLockPanelAction();
    void updateLockPanelAction();
    void updateMask();
    void initIsland();
    //初始化系统托盘是否显示
    void setShowSystemTrayStatus();
private:
    int m_panelSize {48};
    GeneralConfigDefine::PanelSizePolicy m_sizePolicy {GeneralConfigDefine::Small};
    QString m_id;
    Screen *m_screen {nullptr};

    QAction *m_sizeAction {nullptr};
    QAction *m_positionAction {nullptr};
    QAction *m_lockPanelAction {nullptr};
    QAction *m_autoHideAction {nullptr};
    QList<QAction*> m_widgetActions;
    QStringList m_disabledWidgets;

    bool m_autoHide = true;
    bool m_isHidden = true;
    bool m_lockPanel = false;
    QTimer *m_hideTimer = nullptr;
    UnavailableArea m_unavailableArea;
    LingmoUIQuick::WindowProxy *m_windowProxy = nullptr;
    bool m_containsMouse = false;
    int m_panelMaxSize = 92;
    int m_panelMinSize = 48;
    int m_systemTrayInstance = -1;
};

} // LingmoUIPanel

Q_DECLARE_METATYPE(QList<int>)

#endif //LINGMO_PANEL_PANEL_H
