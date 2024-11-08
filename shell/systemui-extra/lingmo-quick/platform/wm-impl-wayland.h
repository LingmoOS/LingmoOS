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
 *
 */

#ifndef LINGMO_QUICK_WM_IMPL_WAYLAND_H
#define LINGMO_QUICK_WM_IMPL_WAYLAND_H

#include "wm-interface.h"
#include "wayland-integration_p.h"

#include <windowmanager/lingmoshell.h>

namespace LingmoUIQuick {

class WMImplWayland : public WMInterface
{
    Q_OBJECT
public:
    explicit WMImplWayland(QWindow *window);

    QPoint position() const override;
    void setPosition(const QPoint &point) override;
    void setWindowType(WindowType::Type type) override;
    void setSkipTaskBar(bool skip) override;
    void setSkipSwitcher(bool skip) override;
    void setRemoveTitleBar(bool remove) override;
    void setPanelAutoHide(bool autoHide) override;
    void setPanelTakesFocus(bool takesFocus) override;
    QScreen* currentScreen() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initSurface();
    inline void destroySurface();

private:
    QPoint m_pos;
    std::unique_ptr<LingmoUIShellSurface> m_shellSurface;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WM_IMPL_WAYLAND_H
