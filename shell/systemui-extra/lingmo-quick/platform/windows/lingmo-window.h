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

#ifndef LINGMO_QUICK_LINGMO_WINDOW_H
#define LINGMO_QUICK_LINGMO_WINDOW_H

#include <QQuickWindow>

#include "wm-interface.h"

namespace LingmoUIQuick {

/**
 * @class LINGMOWindow
 *
 * intergration lingmo-shell
 *
 * 集成lingmo-shell
 * 1.设置窗口类型
 * 2.设置位置
 * 3.跳过任务栏，模糊特效等
 *
 */
class LINGMOWindow : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool skipTaskBar READ skipTaskBar WRITE setSkipTaskBar NOTIFY skipTaskBarChanged)
    Q_PROPERTY(bool skipSwitcher READ skipSwitcher WRITE setSkipSwitcher NOTIFY skipSwitcherChanged)
    Q_PROPERTY(bool removeTitleBar READ removeTitleBar WRITE setRemoveTitleBar NOTIFY removeTitleBarChanged)
    Q_PROPERTY(LingmoUIQuick::WindowType::Type windowType READ windowType WRITE setWindowType NOTIFY windowTypeChanged)
public:
    explicit LINGMOWindow(QWindow *parent = nullptr);
    ~LINGMOWindow() override;

    WindowType::Type windowType() const;
    void setWindowType(WindowType::Type type);

    bool skipTaskBar() const;
    bool skipSwitcher() const;
    bool removeTitleBar() const;

    void setSkipTaskBar(bool skip);
    void setSkipSwitcher(bool skip);
    void setRemoveTitleBar(bool remove);
    QPoint windowPosition() const;
    void setWindowPosition(const QPoint &pos);

Q_SIGNALS:
    void windowTypeChanged();
    void skipTaskBarChanged();
    void skipSwitcherChanged();
    void removeTitleBarChanged();

protected:
    bool event(QEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupProperty();

private:
    class Private;
    Private *d {nullptr};
};

/**
 * @class LINGMOWindowExtension
 *
 * 向qml中注册LINGMOWindow,并为LINGMOWindow设置x,y属性，适配wayland环境
 *
 * Usage:
     LINGMOWindow {
        id: window
        x: 10
        y: 10
    }
 *
 */
class LINGMOWindowExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
public:
    explicit LINGMOWindowExtension(QObject *parent);

    int x() const;
    int y() const;

    void setX(int x);
    void setY(int y);

Q_SIGNALS:
    void xChanged(int x);
    void yChanged(int y);

private:
    LINGMOWindow *m_window {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_LINGMO_WINDOW_H
