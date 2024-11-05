/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPBACKGROUNDWINDOW_H
#define DESKTOPBACKGROUNDWINDOW_H

#include <QMainWindow>
#include <QGSettings>
#include "desktop-icon-view.h"
#include <KF5/KScreen/kscreen/output.h>
#include "windowmanager/windowmanager.h"

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Peony {
class DesktopMenu;
}
class DesktopBackgroundWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DesktopBackgroundWindow(const KScreen::OutputPtr &output, int desktopWindowId, QWidget *parent = nullptr);
    ~DesktopBackgroundWindow() override;

    int id() const;

    KScreen::OutputPtr screen() const;
    QRect getLogicalGeometryFromScreen();

    Peony::DesktopIconView *getIconView();
    void setId(int id);
    void setWindowId(kdk::WindowId id);
    kdk::WindowId getWindowId();

    bool event(QEvent *event) override;

Q_SIGNALS:
    void setDefaultZoomLevel(Peony::DesktopIconView::ZoomLevel level);
    void setSortType(int sortType);
    void destroyed();

public Q_SLOTS:
    void setWindowGeometry(const QRect &geometry);
    void invaidScreen();

    void setCentralView();

protected Q_SLOTS:
    void updateWindowGeometry();

protected:
    void paintEvent(QPaintEvent *event) override;
    QPoint getRelativePos(const QPoint &pos);
    /**
     * 图片填充方式
     * @brief 从给定的图片中，截取一个与屏幕比例相同的矩形。(Rect,居中)。
     * @param pixmap 图片
     * @return
     */
    QRect getSourceRect(const QPixmap &pixmap);
    QRect getSourceRect(const QPixmap &pixmap, const QRect &screenGeometery);
    QRect getDestRect(const QPixmap &pixmap);

private:
    int m_id = -1;
    //QScreen *m_screen = nullptr;
    QGSettings *m_panelSetting = nullptr;
    Peony::DesktopIconView *m_desktopIconView = nullptr;

    KWayland::Client::PlasmaShellSurface *m_shellSurface = nullptr;
    Peony::DesktopMenu *m_menu = nullptr;
    KScreen::OutputPtr m_output = nullptr;
    kdk::WindowId m_windowId;
};

#endif // DESKTOPBACKGROUNDWINDOW_H
