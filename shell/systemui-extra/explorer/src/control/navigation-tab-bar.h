/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef NAVIGATIONTABBAR_H
#define NAVIGATIONTABBAR_H

#include <QTabBar>
#include <QProxyStyle>
#include <QTimer>
#include <memory>

class QToolButton;
class QDrag;
namespace Peony {
class FileInfo;
}

class NavigationTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit NavigationTabBar(QWidget *parent = nullptr);
    QStringList getCurrentUris();

Q_SIGNALS:
    void pageAdded(const QString &uri);
    void pageRemoved();
    void closeWindowRequest();
    void addPageRequest(const QString &uri, bool jumpTo);
    void locationUpdated(const QString &uri);

public Q_SLOTS:
    void addPage(const QString &uri = nullptr, bool jumpToNewTab = false);
    void addPages(const QStringList &uri);
    void updateLocation(int index, const QString &uri);

protected:
    void tabRemoved(int index) override;
    void tabInserted(int index) override;

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void resizeEvent(QResizeEvent *e) override;

private:
    QTimer m_drag_timer;
    bool m_start_drag = false;
    QPoint m_press_pos;
    QDrag *m_drag = nullptr;
    bool m_should_trigger_drop = false;
    QStringList m_has_uris;

    const int ELIDE_TEXT_LENGTH = 16;
};

class TabBarStyle : public QProxyStyle
{
    friend class NavigationTabBar;
    friend class TabWidget;
    static TabBarStyle *getStyle();
    TabBarStyle();

    void polish(QWidget *widget) override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

private:
    bool m_need_adjust = false;
};

#endif // NAVIGATIONTABBAR_H
