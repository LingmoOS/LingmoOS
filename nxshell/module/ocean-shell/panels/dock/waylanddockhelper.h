// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "constants.h"
#include "dockhelper.h"
#include "dockpanel.h"

#include "qwayland-treeland-ocean-shell-v1.h"
#include "qwayland-treeland-wallpaper-color-v1.h"
#include <QWidget>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <qscopedpointer.h>

namespace dock {
class WallpaperColorManager;
class TreeLandDockTriggerArea;
class TreeLandOCEANShellManager;
class TreeLandWindowOverlapChecker;

class WaylandDockHelper : public DockHelper
{
    Q_OBJECT

public:
    WaylandDockHelper(DockPanel *panel);

    void setDockColorTheme(const ColorTheme &theme);
    QString dockScreenName();

protected:
    bool currentActiveWindowFullscreened() override;
    bool isWindowOverlap() override;
    [[nodiscard]] virtual DockWakeUpArea *createArea(QScreen *screen) override;
    void destroyArea(DockWakeUpArea *area) override;

protected Q_SLOTS:
    void setCurrentActiveWindowFullscreened(bool);

private:
    void updateOverlapCheckerPos();

private:
    friend class TreeLandWindowOverlapChecker;
    bool m_isWindowOverlap;
    bool m_isCurrentActiveWindowFullscreened;
    DockPanel *m_panel;
    QScopedPointer<WallpaperColorManager> m_wallpaperColorManager;
    QScopedPointer<TreeLandWindowOverlapChecker> m_overlapChecker;
    QScopedPointer<TreeLandOCEANShellManager> m_oceanShellManager;
};

class WallpaperColorManager : public QWaylandClientExtensionTemplate<WallpaperColorManager>, public QtWayland::treeland_wallpaper_color_manager_v1
{
    Q_OBJECT
public:
    explicit WallpaperColorManager(WaylandDockHelper *helper);

    void watchScreen(const QString &screeName);
    void treeland_wallpaper_color_manager_v1_output_color(const QString &output, uint32_t isDark) override;

Q_SIGNALS:
    void wallpaperColorChanged(uint32_t isDark);

private:
    WaylandDockHelper *m_helper;
};

class TreeLandOCEANShellManager : public QWaylandClientExtensionTemplate<TreeLandOCEANShellManager>, public QtWayland::treeland_ocean_shell_manager_v1
{
    Q_OBJECT

public:
    explicit TreeLandOCEANShellManager();
};

class TreeLandWindowOverlapChecker : public QWaylandClientExtensionTemplate<TreeLandWindowOverlapChecker>, public QtWayland::treeland_window_overlap_checker
{
    Q_OBJECT

public:
    TreeLandWindowOverlapChecker(WaylandDockHelper *helper, struct ::treeland_window_overlap_checker *);
    ~TreeLandWindowOverlapChecker();

protected:
    void treeland_window_overlap_checker_enter() override;
    void treeland_window_overlap_checker_leave() override;

private:
    WaylandDockHelper *m_helper;
};

class TreeLandDockWakeUpArea : public QWidget, public DockWakeUpArea
{
    Q_OBJECT
public:
    explicit TreeLandDockWakeUpArea(QScreen *screen, WaylandDockHelper *helper);

    void open() override;
    void close() override;

    void updateDockWakeArea(Position pos) override;

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Position m_pos;
};
}
