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

#include "shared-engine-component.h"
#include "shared-qml-engine.h"

#include <QQmlComponent>

namespace LingmoUIQuick {

class SharedEngineComponentPrivate
{
public:
    // raw
    QUrl source;
    SharedEngineComponent *q {nullptr};

    // env
    QVariantMap initData;
    QQmlComponent *component {nullptr};
    QQmlContext *rootContext {nullptr};

    // generate
    QObject *rootObject {nullptr};

    // func
    explicit SharedEngineComponentPrivate(SharedEngineComponent *parent);
    void printComponentError() const;
    void beginCreateRootObject();
};

SharedEngineComponentPrivate::SharedEngineComponentPrivate(SharedEngineComponent *parent) : q(parent)
{

}

void SharedEngineComponentPrivate::printComponentError() const
{
    const QList<QQmlError> errorList = component->errors();
    for (const QQmlError &error : errorList) {
        QMessageLogger(error.url().toString().toLatin1().constData(), error.line(), nullptr).warning() << error;
    }
}

void SharedEngineComponentPrivate::beginCreateRootObject()
{
    delete component;
    component = new QQmlComponent(SharedEngineComponent::sharedEngine(), q);
    QObject::connect(component, &QQmlComponent::statusChanged, q, &SharedEngineComponent::componentStatusChanged);

    component->loadUrl(source);
//    rootObject = component->beginCreate(rootContext ? rootContext : SharedEngineComponent::sharedEngine()->rootContext());
    rootObject = component->beginCreate(rootContext);

    if (component->isReady() || component->isError()) {
        q->completedCreateRootObject();
    } else {
        QObject::connect(component, &QQmlComponent::statusChanged, q, &SharedEngineComponent::completedCreateRootObject);
    }
}

// ====== SharedEngineComponent ====== //
QQmlEngine *SharedEngineComponent::sharedEngine()
{
    return SharedQmlEngine::sharedQmlEngine();
}

SharedEngineComponent::SharedEngineComponent(QQmlContext *rootContext, QObject *parent)
  : QObject(parent),
    d(new SharedEngineComponentPrivate(this))
{
    if (!rootContext) {
        rootContext = new QQmlContext(SharedEngineComponent::sharedEngine());
    }
    d->rootContext = rootContext;
}

SharedEngineComponent::SharedEngineComponent(const QVariantMap &initData, QQmlContext *rootContext, QObject *parent)
  : SharedEngineComponent(rootContext, parent)
{
    d->initData = initData;
}

SharedEngineComponent::~SharedEngineComponent()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void SharedEngineComponent::setSource(const QUrl &source)
{
    if (source.isEmpty()) {
        return;
    }

    d->source = source;
    d->beginCreateRootObject();
}

QUrl SharedEngineComponent::source() const
{
    return d->source;
}

QObject *SharedEngineComponent::rootObject() const
{
    return d->rootObject;
}

QQmlComponent *SharedEngineComponent::component() const
{
    return d->component;
}

QQmlContext *SharedEngineComponent::rootContext() const
{
    return d->rootContext;
}

QVariantMap SharedEngineComponent::initializeData() const
{
    return d->initData;
}

void SharedEngineComponent::completedCreateRootObject()
{
    if (!d->component) {
        qWarning() << "SharedEngineComponent: Component is null, source:" << d->source;
        return;
    }

    disconnect(d->component, &QQmlComponent::statusChanged, this, &SharedEngineComponent::completedCreateRootObject);
    if (!d->component->isReady() || d->component->isError()) {
        d->printComponentError();
        return;
    }

    QVariantMap::const_iterator iterator = d->initData.constBegin();
    for (; iterator != d->initData.constEnd() ; ++iterator) {
        d->rootObject->setProperty(iterator.key().toUtf8().data(), iterator.value());
    }

    d->component->completeCreate();
}

} // LingmoUIQuick
