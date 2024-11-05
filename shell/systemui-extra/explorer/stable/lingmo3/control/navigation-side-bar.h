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

#ifndef DEVELNAVIGATIONSIDEBAR_H
#define DEVELNAVIGATIONSIDEBAR_H

#include <QTreeView>
#include <QStyledItemDelegate>
#include <QProxyStyle>

namespace Peony {
class SideBarModel;
class SideBarProxyFilterSortModel;
class SideBarAbstractItem;
}

class QPushButton;
class QVBoxLayout;

class NavigationSideBar : public QTreeView
{
    Q_OBJECT
public:
    explicit NavigationSideBar(QWidget *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *e);

    void updateGeometries();
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *e);

    void dropEvent(QDropEvent *e);

    QSize sizeHint() const;
    void JumpDirectory(const QString& uri);/* 跳转目录 */

Q_SIGNALS:
    void updateWindowLocationRequest(const QString &uri, bool addHistory = true, bool force = false);
    void labelButtonClicked(bool checked);

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void wheelEvent(QWheelEvent *event);
    void dragEnterEvent(QDragEnterEvent *event) override;

    int sizeHintForColumn(int column) const override;

    QStyleOptionViewItem viewOptions() const override;

private:
    Peony::SideBarProxyFilterSortModel *m_proxy_model = nullptr;
    Peony::SideBarModel *m_model = nullptr;
    Peony::SideBarAbstractItem* m_currSelectedItem = nullptr;
    bool m_notAllowHorizontalMove = false;/* 按下左右键不可使侧边栏内容左右平移显示 */
};

class NavigationSideBarContainer : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationSideBarContainer(QWidget *parent = nullptr);
    void addSideBar(NavigationSideBar *sidebar);

    QSize sizeHint() const override;

private:
    NavigationSideBar *m_sidebar = nullptr;
    QVBoxLayout *m_layout = nullptr;

    QPushButton *m_label_button = nullptr;
};

class NavigationSideBarItemDelegate : public QStyledItemDelegate
{
    friend class NavigationSideBar;
    explicit NavigationSideBarItemDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class NavigationSideBarStyle : public QProxyStyle
{
public:
    explicit NavigationSideBarStyle();
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};

#endif // DEVELNAVIGATIONSIDEBAR_H
