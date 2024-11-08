/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef LINGMO_MENU_MENU_MAIN_WINDOW_H
#define LINGMO_MENU_MENU_MAIN_WINDOW_H

#include <QObject>
#include <QRect>
#include <QScreen>
#include <QQuickView>
#include <QQuickWindow>
#include <QGSettings>
#include <window-helper.h>

namespace LingmoUIMenu {

class WindowModule final
{
public:
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
};

class WindowHelper final
{
public:
    static void windowBlur(QWindow *window, bool enable, const QRegion &region = QRegion());
    static void setRegion(QWindow *window, qreal x, qreal y, qreal w, qreal h, qreal radius);
};

class WindowGeometryHelper final : public QObject
{
    Q_OBJECT
public:
    explicit WindowGeometryHelper(QObject *parent = nullptr);

    const QRect &normalGeometry();
    const QRect &fullScreenGeometry();
    const int getPanelPos();

Q_SIGNALS:
    void geometryChanged();

private Q_SLOTS:
    void updateGeometry();
    void updatePrimaryScreen(QScreen *screen);

private:
    void initPanelSetting();
    void initScreenMonitor();

private:
    // 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
    int m_panelPos{4};
    int m_panelSize{0};

    QScreen *m_primaryScreen{nullptr};
    QRect m_normalGeometry;
    QRect m_fullScreenGeometry;
};

class MenuWindow : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(bool isFullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
    Q_PROPERTY(bool effectEnabled READ effectEnabled NOTIFY effectEnabledChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(double transparency READ transparency NOTIFY transparencyChanged)
    Q_PROPERTY(int panelPos READ panelPos NOTIFY panelPosChanged)
    Q_PROPERTY(QRect normalRect READ normalRect NOTIFY normalRectChanged)

public:
    explicit MenuWindow(QWindow *parent = nullptr);
    MenuWindow(QQmlEngine* engine, QWindow *parent);

    bool isFullScreen() const;
    void setFullScreen(bool isFullScreen);

    bool editMode() const;
    void setEditMode(bool mode);

    bool effectEnabled() const;
    double transparency() const;
    int panelPos() const;
    QRect normalRect() const;

    void activeMenuWindow(bool active);

    Q_INVOKABLE void changeWindowBlurRegion(qreal x, qreal y, qreal w, qreal h, qreal radius = 8);
    Q_INVOKABLE void exitFullScreen();

Q_SIGNALS:
    void geometryChanged();
    void effectEnabledChanged();
    void transparencyChanged();
    void fullScreenChanged();
    void beforeFullScreenChanged();
    void beforeFullScreenExited();
    void panelPosChanged();
    void editModeChanged();
    void normalRectChanged();

protected:
    void exposeEvent(QExposeEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *event) override;

private:
    void init();
    void initPanelSetting();
    void updateGeometry();
    void updateGeometryOfMask();
    void updateCurrentScreenGeometry();

private:
    // 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
    int m_panelPos{4};
    int m_panelSize{48};
    bool m_editMode {false};
    bool m_isFullScreen{false};
    QRect m_maskGeometry = QRect(0,0,0,0);
    QRect m_normalGeometry = QRect(0,0,0,0);
    QRect m_fullScreenGeometry = QRect(0,0,0,0);
    QGSettings *m_setting {nullptr};
    LingmoUIQuick::WindowProxy2 *m_windowProxy {nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_MENU_MAIN_WINDOW_H
