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

#ifndef LINGMO_QUICK_SHARED_ENGINE_COMPONENT_H
#define LINGMO_QUICK_SHARED_ENGINE_COMPONENT_H

#include <QUrl>
#include <QVariant>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QObject>

namespace LingmoUIQuick {

class SharedEngineComponentPrivate;

/**
 * @class SharedEngineComponent
 *
 * 一个Component，用于加载并实例化一个qml文件的顶级对象，该对象为rootObject。
 * 基本功能由QQmlComponent提供。
 *
 * 所有通过SharedEngineComponent实例化的object都运行在同一个共享engine中。
 * 每个object在创建前都需要一个上下文，如果未指定那么会从共享engine根上下文创建一个。
 * 可以在构造SharedEngineComponent时传入初始化参数，用于初始化rootObject的属性。
 *
 */
class SharedEngineComponent : public QObject
{
    Q_OBJECT
public:
    static QQmlEngine *sharedEngine();

    explicit SharedEngineComponent(QQmlContext *rootContext = nullptr, QObject *parent = nullptr);
    explicit SharedEngineComponent(const QVariantMap &initData, QQmlContext *rootContext, QObject *parent = nullptr);

    ~SharedEngineComponent() override;

    // raw materials
    QUrl source() const;
    void setSource(const QUrl &source);
    QQmlContext *rootContext() const;
    QVariantMap initializeData() const;

    // products
    QObject *rootObject() const;
    QQmlComponent *component() const;

Q_SIGNALS:
    void componentStatusChanged(QQmlComponent::Status status);

private Q_SLOTS:
    void completedCreateRootObject();

private:
    SharedEngineComponentPrivate *d {nullptr};

    friend class SharedEngineComponentPrivate;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_SHARED_ENGINE_COMPONENT_H
