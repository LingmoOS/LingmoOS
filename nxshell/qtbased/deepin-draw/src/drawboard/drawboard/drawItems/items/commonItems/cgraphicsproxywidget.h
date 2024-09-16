// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSPROXYWIDGET_H
#define CGRAPHICSPROXYWIDGET_H

#include <globaldefine.h>
#include <QGraphicsProxyWidget>

class PageItem;
class DRAWLIB_EXPORT ProxyWidgetItem : public QGraphicsProxyWidget
{
public:
    ProxyWidgetItem(PageItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());

    PageItem *parentDrawItem();

protected:
    bool event(QEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};

class DRAWLIB_EXPORT ProxyTextWidgetItem: public ProxyWidgetItem
{
public:
    ProxyTextWidgetItem(PageItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) override;
};

#endif // CGRAPHICSPROXYWIDGET_H
