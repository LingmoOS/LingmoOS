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

#ifndef LINGMO_QUICK_WIDGET_CONTAINER_ITEM_H
#define LINGMO_QUICK_WIDGET_CONTAINER_ITEM_H

#include <QObject>
#include <QQuickItem>
#include <QAbstractListModel>
#include "widget-item.h"

namespace LingmoUIQuick {

class Widget;
class WidgetItem;
class WidgetContainer;
class SharedEngineComponent;
class WidgetContainerItemPrivate;

class WidgetItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Key {
        WidgetItemRole
    };
    Q_ENUM(Key)

    explicit WidgetItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool insertWidgetItem(WidgetQuickItem *item);
    bool removeWidgetItem(WidgetQuickItem *item);

private:
    QList<WidgetQuickItem *> m_widgetItems;
    friend class WidgetContainerItem;
};

class WidgetContainerItem : public WidgetQuickItem
{
    Q_OBJECT
    Q_PROPERTY(LingmoUIQuick::WidgetItemModel *widgetItemModel READ widgetItemModel CONSTANT FINAL)
public:
    explicit WidgetContainerItem(QQuickItem *parent = nullptr);
    ~WidgetContainerItem() override;

    WidgetContainer *container() const;
    SharedEngineComponent *component() const;

    WidgetItemModel *widgetItemModel() const;


    Q_INVOKABLE LingmoUIQuick::WidgetItem *widgetItemForWidget(LingmoUIQuick::Widget *widget) const;

Q_SIGNALS:
    void widgetItemAdded(LingmoUIQuick::WidgetQuickItem *);
    void widgetItemRemoved(LingmoUIQuick::WidgetQuickItem *);

private Q_SLOTS:
    void onWidgetAdded(LingmoUIQuick::Widget *widget);
    void onWidgetRemoved(LingmoUIQuick::Widget *widget);

protected:
    void classBegin() override;
    bool event(QEvent *event) override;

private:
    void setContainer(WidgetContainer *container);
    bool showContextMenu(QMouseEvent *mouseEvent);

private:
    WidgetContainerItemPrivate *d {nullptr};
};

class WidgetContainerItemAttached : public QObject
{
    Q_OBJECT
public:
    // Attached prop for WidgetContainer
    static WidgetContainer *qmlAttachedProperties(QObject *object);
};

} // LingmoUIQuick

QML_DECLARE_TYPEINFO(LingmoUIQuick::WidgetContainerItemAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif //LINGMO_QUICK_WIDGET_CONTAINER_ITEM_H
