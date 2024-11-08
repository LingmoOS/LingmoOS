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

#ifndef LINGMO_QUICK_WIDGET_H
#define LINGMO_QUICK_WIDGET_H

#include <QObject>
#include <QAction>
#include <QQmlListProperty>

#include "widget-metadata.h"
#include "widget-content.h"
#include "types.h"
#include "config.h"

namespace LingmoUIQuick {

class Config;
class ConfigPropertyMap;
class WidgetPrivate;
class WidgetContainer;
class WidgetInterface;

/**
 * 一个"Widget"。
 * widget包含元数据和内容(Content)数据，存放插件的基础数据属性。
 * widget通过元数据进行初始化，被加载到某个容器后，容器赋予其更多新属性。
 */
class Widget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(QString website READ website CONSTANT FINAL)
    Q_PROPERTY(QString bugReport READ bugReport CONSTANT FINAL)
    Q_PROPERTY(QString description READ description CONSTANT FINAL)
    Q_PROPERTY(QVariantList authors READ authors CONSTANT FINAL)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QQmlListProperty<QAction> actions READ qmlActions)
    Q_PROPERTY(int instanceId READ instanceId CONSTANT FINAL)
    /**
     * 将Widget置为激活状态，应用可能会根据这个状态设置一些UI交互，比如任务栏不会自动隐藏。
     */
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)
    /**
     * 控制Widget希望显示的位置
     */
    Q_PROPERTY(WidgetMetadata::Hosts showIn READ showIn CONSTANT FINAL)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)

    /**
     * Widget的局部配置接口,该配置仅在当前Container内有效
     */
    Q_PROPERTY(LingmoUIQuick::ConfigPropertyMap *config READ qmlLocalConfig CONSTANT FINAL)

    /**
     *Widget的独立配置接口,可由ConfigPolicy控制,可在多个Container间共享
     */
    Q_PROPERTY(LingmoUIQuick::ConfigPropertyMap *globalConfig READ qmlGlobalConfig CONSTANT FINAL)
    Q_PROPERTY(LingmoUIQuick::WidgetContainer *container READ container NOTIFY containerChanged FINAL)
    Q_PROPERTY(LingmoUIQuick::Types::Orientation orientation READ orientation NOTIFY orientationChanged FINAL)
    /**
     *widget中加载的以动态库形式安装的插件
     */
    Q_PROPERTY(const LingmoUIQuick::WidgetInterface* plugin READ plugin CONSTANT FINAL)
public:
    explicit Widget(const WidgetMetadata& metaData, QObject *parent=nullptr);
    ~Widget() override;

    QString id() const;

    QString icon() const;
    void setIcon(const QString &icon);

    QString name() const;
    void setName(const QString &name);

    QString tooltip() const;
    void setTooltip(const QString &tooltip);

    QString version() const;
    void setVersion(const QString &version);

    QString website() const;
    QString bugReport() const;
    QString description() const;
    QVariantList authors() const;

    const WidgetMetadata &metadata() const;
    const WidgetContent &content() const;

    // 初始化UI组件时，如果出现错误，会存放数据该属性
    void setUiError(const QString &error);
    QString uiError() const;
    bool hasUiError() const;

    QList<QAction *> actions() const;
    void setActions(const QList<QAction *>& actions);
    void setAppendActions(QList<QAction *> actions);
    // qml接口
    QQmlListProperty<QAction> qmlActions();

    // widget的唯一id，在某个应用中是唯一的，用于确认widget身份
    int instanceId() const;
    void setInstanceId(int instanceId);

    virtual bool active() const;
    virtual void setActive(bool active);

    virtual Types::WidgetType type() const;
    Types::Orientation orientation() const;
    void setOrientation(Types::Orientation o);

    // qml api
    ConfigPropertyMap *qmlLocalConfig() const;
    ConfigPropertyMap *qmlGlobalConfig() const;
    Config *config() const;
    virtual void setConfig(Config *config);
    Config *globalConfig() const;

    WidgetContainer *container() const;
    WidgetMetadata::Hosts showIn() const;

    bool visible() const;
    void setVisible(bool visible);

    WidgetInterface *plugin() const;
    virtual void setContainer(WidgetContainer *container);

Q_SIGNALS:
    void iconChanged();
    void nameChanged();
    void tooltipChanged();
    void versionChanged();
    void actionsChanged();
    void activeChanged();
    void visibleChanged();
    void orientationChanged();

    // widget生命周期信号
    void aboutToDeleted();
    void containerChanged(const QString &oldId, const QString &newId);

private:
    void loadPlugin() const;

private:
    WidgetPrivate *d {nullptr};

    friend class WidgetContainer;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_H
