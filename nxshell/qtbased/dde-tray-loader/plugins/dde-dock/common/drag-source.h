// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DRAG_SOURCE_H
#define DRAG_SOURCE_H

#include "dock-constants.h"

#include <QObject>
#include <QWidget>
#include <QDropEvent>

class DragSource : public QObject
{
    Q_OBJECT

public:
    explicit DragSource(Dock::ItemType itemType, QObject *parent = nullptr)
        : QObject(parent)
        , m_itemType(itemType)
        , m_dockItem(nullptr)
    {

    }

    ~DragSource() {}

    Dock::ItemType itemType() const { return m_itemType; }

    void setDockItem(QWidget* dockItem) { m_dockItem = dockItem; }
    QWidget* dockItem() const { return m_dockItem; }

    static DragSource* event2DragSource(QDropEvent *e)
    {
        if (!e || !e->source()) {
            return nullptr;
        }

        return dynamic_cast<DragSource *>(e->source());
    }

    static QWidget* dockItem(QDropEvent *e)
    {
        auto dragSource = event2DragSource(e);
        if (!dragSource) {
            return nullptr;
        }

        return dragSource->dockItem();
    }

    static Dock::ItemType itemType(QDropEvent *e)
    {
        auto dragSource = event2DragSource(e);
        if (!dragSource) {
            return Dock::ItemType_TrayPlugin;
        }

        return dragSource->itemType();
    }

private:
    Dock::ItemType m_itemType;
    QWidget *m_dockItem;
};

#endif
