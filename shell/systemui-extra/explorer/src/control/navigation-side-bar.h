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

#ifndef NAVIGATIONSIDEBAR_H
#define NAVIGATIONSIDEBAR_H
#include "side-bar.h"
#include "file-label-box.h"

#include <QTreeView>
#include <QStyledItemDelegate>
#include <QProxyStyle>
#include <QGSettings>

namespace Peony {
class SideBarModel;
class SideBarProxyFilterSortModel;
class SideBarAbstractItem;
}

class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

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
    void sendKdkDataAsync();  //异步上传数据埋点信息

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

    bool viewportEvent(QEvent *event) override;

private:
    Peony::SideBarProxyFilterSortModel *m_proxy_model = nullptr;
    Peony::SideBarModel *m_model = nullptr;
    Peony::SideBarAbstractItem* m_currSelectedItem = nullptr;
    bool m_notAllowHorizontalMove = false;/* 按下左右键不可使侧边栏内容左右平移显示 */
};

class NavigationSideBarContainer : public Peony::SideBar
{
    Q_OBJECT
public:
    explicit NavigationSideBarContainer(QWidget *parent = nullptr);
    void addSideBar(NavigationSideBar *sidebar);

    QSize sizeHint() const override;

private:
    NavigationSideBar *m_sidebar = nullptr;
    QVBoxLayout *m_layout = nullptr;

    FileLabelBox *m_labelDialog = nullptr;
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

class TitleLabel : public QWidget
{
    Q_OBJECT
public:
    explicit TitleLabel(QWidget *parent);

private:
    QLabel *m_pix_label;
    QLabel *m_text_label;
    QGSettings *m_gSettings;
};

#include <QPushButton>
class LabelButton : public QPushButton
{
    Q_OBJECT
public:
    explicit LabelButton(QWidget *parent = nullptr);
    void setLastIcon(const QString &symbolic);
    void setFirstIcon(const QString &symbolic);
    void setText(QString text);
    void setShow(bool isShow);
    bool getShow();

Q_SIGNALS:
    void clicked (bool show);

protected:
//    void mousePressEvent(QMouseEvent *event) override;
//    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QLabel *m_firstSymbolic = nullptr;
    QLabel *m_lastSymbolic = nullptr;
    QLabel *m_text = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    bool m_isPress = false;
    bool m_show = false;

};
#endif // NAVIGATIONSIDEBAR_H
