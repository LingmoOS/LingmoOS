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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_WINDOW_BLUR_BEHIND_H
#define LINGMO_QUICK_WINDOW_BLUR_BEHIND_H
#include <QObject>
#include <QQmlListProperty>
#include <QQuickItem>

namespace LingmoUIQuick {
class RectRegion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
public:
    explicit RectRegion(QObject *parent = nullptr);
    void setX(int x);
    int x() const;

    void setY(int y);
    int y() const;

    void setWidth(int width);
    int width() const;

    void setHeight(int height);
    int height() const;

    void setRadius(int radius);
    int radius() const;

Q_SIGNALS:
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void radiusChanged();

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    int m_radius = 0;
};
/**
 * @class WindowBlurBehind
 * QAction只能通过指定QICon设置图标
 * 扩展指定图标名称设置图标功能，增加指定为分割线功能，增加设置子菜单功能
 * @see https://doc.qt.io/qt-5/qtqml-referenceexamples-extended-example.html
 * Usage:
        WindowBlurBehind {
            rectRegions: [
                RectRegion {
                    x: 0
                    y: 0
                    width: 100
                    height: 200
                    radius: 8
                },
                RectRegion {
                    x: 100
                    y: 200
                    width: 100
                    height: 200
                    radius: 0
                }
            ]
            enable: true
        }
 *
 *
 */
class WindowBlurBehind : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<LingmoUIQuick::RectRegion> rectRegions READ rectRegions)
    Q_PROPERTY(bool enable READ enable WRITE setEnable NOTIFY enableChanged)
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    Q_CLASSINFO("DefaultProperty", "rectRegions")

public:
    explicit WindowBlurBehind(QQuickItem *parent = nullptr);
    QQmlListProperty<RectRegion> rectRegions();

    void setEnable(bool enable);
    bool enable() const;

    void setWindow(QWindow *window);
    QWindow* window();

    /**
     * You cannot append elements to or clear a QQmlListProperty directly in QML.
     * To edit QQmlListProperty in QML, assign a new list to it.
     *
     * Note that objects cannot be individually added to or removed from the list once created;
     * to modify the contents of a list, it must be reassigned to a new list.
     */
    static void appendRect(QQmlListProperty<RectRegion> *list, RectRegion *rect);
    static int rectCount(QQmlListProperty<RectRegion> *list);
    static RectRegion *rectAt(QQmlListProperty<RectRegion> *list, int index);
    static void clearRect(QQmlListProperty<RectRegion> *list);
    static void replaceRect(QQmlListProperty<RectRegion> *list, int index, RectRegion *rect);
    static void removeLastRect(QQmlListProperty<RectRegion> *list);

    Q_INVOKABLE void addRectRegion(RectRegion *rect);
    Q_INVOKABLE void clearRectRegion();

Q_SIGNALS:
    void enableChanged();
    void windowChanged();

private:
    void updateBlurRegion();
    QList<RectRegion *> m_rects;
    bool m_enable = false;
    QWindow *m_window = nullptr;

};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WINDOW_BLUR_BEHIND_H
