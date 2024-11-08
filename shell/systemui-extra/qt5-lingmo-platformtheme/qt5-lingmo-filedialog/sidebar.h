/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#ifndef SIDEBAR_H
#define SIDEBAR_H
#include <QTreeView>
#include <QPaintEvent>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOption>
#include <PeonySideBarProxyModel>
#include <QComboBox>
#include <QSize>
#include <PeonySideBarModel>

namespace LINGMOFileDialog {
class FileDialogComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit FileDialogComboBox(QWidget *parent = nullptr);
    ~FileDialogComboBox(){}
    QSize minimumSizeHint() const;

public Q_SLOTS:
    void tableModeChanged(bool isTableMode);

Q_SIGNALS:
    void setStyleChanged(bool change) const;

private:
    bool m_styleChanged = false;
    QSize m_minSize;
};

class SideBarItemDelegate : public QStyledItemDelegate
{
    friend class FileDialogSideBar;
    explicit SideBarItemDelegate(QObject *parent = nullptr);
    ~SideBarItemDelegate(){}

//    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


class FileDialogSideBar : public QTreeView
{

    Q_OBJECT
public:
    explicit FileDialogSideBar(QWidget *parent = nullptr);
    ~FileDialogSideBar();
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void menuRequest(const QPoint &pos);
    void tableModeChanged(bool isTableMode);

private:
    Peony::SideBarProxyFilterSortModel *m_proxyModel = nullptr;
    Peony::SideBarModel *m_model = nullptr;
    SideBarItemDelegate *m_delegate = nullptr;

Q_SIGNALS:
    void goToUriRequest(const QString &uri, bool addToHistory = true, bool forceUpdate = false);

};


class SideBarStyle : public QProxyStyle
{
public:
    explicit SideBarStyle();
    static SideBarStyle* getStyle();
    ~SideBarStyle(){}
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};
}
#endif // SIDEBAR_H
