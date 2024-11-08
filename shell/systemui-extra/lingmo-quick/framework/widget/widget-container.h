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

#ifndef LINGMO_QUICK_WIDGET_CONTAINER_H
#define LINGMO_QUICK_WIDGET_CONTAINER_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QScreen>
#include <QAction>
#include <QList>

#include "types.h"
#include "margin.h"
#include "widget.h"
#include "widget-loader.h"

namespace LingmoUIQuick {

class Island;
class Widget;
class WidgetContainerPrivate;

class WidgetContainer : public Widget
{
    Q_OBJECT
    Q_PROPERTY(QString appid READ appid CONSTANT FINAL)
    Q_PROPERTY(LingmoUIQuick::Margin* margin READ margin CONSTANT FINAL)
    Q_PROPERTY(LingmoUIQuick::Margin* padding READ padding CONSTANT FINAL)
    Q_PROPERTY(int radius READ radius NOTIFY radiusChanged FINAL)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged FINAL)
    Q_PROPERTY(LingmoUIQuick::Types::Pos position READ position NOTIFY positionChanged FINAL)
    Q_PROPERTY(QScreen* screen READ screen NOTIFY screenChanged FINAL)
    Q_PROPERTY(WidgetMetadata::Host host READ host WRITE setHost NOTIFY hostChanged)
public:
    explicit WidgetContainer(QObject *parent = nullptr);
    explicit WidgetContainer(const WidgetMetadata &metaData, QObject *parent = nullptr);
    ~WidgetContainer();

    Types::WidgetType type() const override;

    // prop
    QString appid() const;

    QRect geometry() const;
    void setGeometry(QRect geometry);

    int radius() const;
    void setRadius(int radius);

    Types::Pos position() const;
    void setPosition(Types::Pos position);

    Margin* margin() const;
    Margin* padding() const;

    // widgets
    void addWidget(Widget *widget);
    void addWidget(const QString &id, int instanceId);
    QVector<Widget *> widgets() const;

    // loader
    static WidgetLoader &widgetLoader();

    void removeWidget(QString id);
    void removeWidget(int index);
    void removeWidget(Widget *widget);

    QScreen* screen() const;
    void setScreen(QScreen *screen);

    bool active() const override;

    WidgetMetadata::Host host() const;
    void setHost(WidgetMetadata::Host host);

    const Island *island() const;
    void setIsland(Island *island);
    void setContainer(WidgetContainer* container) override;
    void setConfig(Config* config) override;

    void setDisableInstances(const QList<int>& instanceIds);
    void addDisableInstance(int id);
    void removeDisableInstance(int id);
    QList<int> disableInstances();

Q_SIGNALS:
    void widgetAdded(LingmoUIQuick::Widget *widget);
    void widgetRemoved(LingmoUIQuick::Widget *widget);
    void geometryChanged();
    void screenChanged();
    void radiusChanged();
    void positionChanged();
    void hostChanged();

private:
    WidgetContainerPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_CONTAINER_H
