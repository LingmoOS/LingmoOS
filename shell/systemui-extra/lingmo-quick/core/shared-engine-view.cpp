/*
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "shared-engine-view.h"
#include "shared-qml-engine.h"

#include <QQuickItem>
#include <QQmlComponent>

namespace LingmoUIQuick {

class SharedEngineViewPrivate
{
public:
    SharedEngineViewPrivate() = delete;
    SharedEngineViewPrivate(QQmlEngine *e, SharedEngineView *q);

    void createObject();
    bool setRootObject(QObject *obj);
    void initResizeSignal() const;
    void updateSize() const;

    void printComponentError() const;

    // env
    SharedEngineView *view {nullptr};
    QQmlEngine *engine {nullptr};
    QQmlContext *rootContext {nullptr};

    SharedEngineView::ResizeMode resizeMode {SharedEngineView::SizeRootObjectToView};

    // object item
    QUrl source;
    QQuickItem *rootItem {nullptr};
    QQmlComponent *component {nullptr};
};

SharedEngineViewPrivate::SharedEngineViewPrivate(QQmlEngine *e, SharedEngineView *q)
  : view(q), engine(e), rootContext(new QQmlContext(engine->rootContext(), q))
{

}

void SharedEngineViewPrivate::createObject()
{
    if (!engine) {
        qWarning() << "SharedEngineView: invalid qml engine.";
        return;
    }

    if (rootItem) {
        delete rootItem;
        rootItem = nullptr;
    }

    if (component) {
        delete component;
        component = nullptr;
    }

    if (source.isEmpty()) {
        return;
    }

    component = new QQmlComponent(engine, source, view);
    if (component->isLoading()) {
        QObject::connect(component, &QQmlComponent::statusChanged, view, &SharedEngineView::continueCreate);
    } else {
        view->continueCreate();
    }
}

bool SharedEngineViewPrivate::setRootObject(QObject *obj)
{
    if (rootItem == obj) {
        return true;
    }

    delete rootItem;
    if (!obj) {
        return true;
    }

    if (auto item = qobject_cast<QQuickItem *>(obj)) {
        rootItem = item;
        item->setParentItem(view->contentItem());
        item->setParent(view->contentItem());

        initResizeSignal();
        return true;
    }

    if (qobject_cast<QWindow *>(obj)) {
        qWarning() << "SharedEngineView does not support using a window as a root item." << Qt::endl
                   << Qt::endl
                   << "If you wish to create your root window from QML, consider using QQmlApplicationEngine instead." << Qt::endl;
        return false;
    }

    qWarning() << "SharedEngineView only supports loading of root objects that derive from QQuickItem." << Qt::endl
               << Qt::endl
               << "Ensure your QML code is written for QtQuick 2, and uses a root that is or" << Qt::endl
               << "inherits from QtQuick's Item (not a Timer, QtObject, etc)." << Qt::endl;
    return false;
}

void SharedEngineViewPrivate::initResizeSignal() const
{
    if (!rootItem) {
        return;
    }

    if (resizeMode == SharedEngineView::SizeViewToRootObject) {
        QObject::connect(rootItem, &QQuickItem::widthChanged, view, &SharedEngineView::updateViewSize);
        QObject::connect(rootItem, &QQuickItem::heightChanged, view, &SharedEngineView::updateViewSize);

    } else {
        QObject::disconnect(rootItem, &QQuickItem::widthChanged, view, &SharedEngineView::updateViewSize);
        QObject::disconnect(rootItem, &QQuickItem::heightChanged, view, &SharedEngineView::updateViewSize);
    }

    updateSize();
}

void SharedEngineViewPrivate::updateSize() const
{
    if (!rootItem) {
        return;
    }

    if (resizeMode == SharedEngineView::SizeViewToRootObject) {
        QSize size;
        // 最小为1x1
        size.setWidth(rootItem->width() < 1 ? 1 : (int)rootItem->width());
        size.setHeight(rootItem->height() < 1 ? 1 : (int)rootItem->height());

        if (size.isValid() && size != view->size()) {
            view->resize(size);
        }

    } else {
        bool updateWidth = rootItem->width() != view->width();
        bool updateHeight = rootItem->height() != view->height();

        if (updateWidth && updateHeight) {
            rootItem->setSize(view->size());

        } else if (updateWidth) {
            rootItem->setWidth(view->width());

        } else if (updateHeight) {
            rootItem->setHeight(view->height());
        }
    }
}

void SharedEngineViewPrivate::printComponentError() const
{
    const QList<QQmlError> errorList = component->errors();
    for (const QQmlError &error : errorList) {
        QMessageLogger(error.url().toString().toLatin1().constData(), error.line(), nullptr).warning() << error;
    }
}

// ====== SharedEngineView Begin ======//
QQmlEngine *SharedEngineView::sharedEngine()
{
    return SharedQmlEngine::sharedQmlEngine();
}

SharedEngineView::SharedEngineView(QWindow *parent)
  : QQuickWindow(parent), d(new SharedEngineViewPrivate(SharedEngineView::sharedEngine(), this))
{

}

QQmlEngine *SharedEngineView::engine() const
{
    return d->engine;
}

QQmlContext *SharedEngineView::rootContext() const
{
    return d->rootContext;
}

void SharedEngineView::setSource(const QUrl &source) const
{
    d->source = source;
    d->createObject();
}

void SharedEngineView::continueCreate() const
{
    QObject::disconnect(d->component, &QQmlComponent::statusChanged, this, &SharedEngineView::continueCreate);

    if (d->component->isError()) {
        d->printComponentError();
        return;
    }

    // 改为beginCreate
    QObject *obj = d->component->create(d->rootContext ? d->rootContext : d->engine->rootContext());
    if (d->component->isError()) {
        d->printComponentError();
        if (obj) {
            obj->deleteLater();
        }
        return;
    }

    d->setRootObject(obj);
}

void SharedEngineView::setResizeMode(SharedEngineView::ResizeMode resizeMode)
{
    if (d->resizeMode == resizeMode) {
        return;
    }

    d->resizeMode = resizeMode;
    if (d->rootItem) {
        d->initResizeSignal();
    }
}

void SharedEngineView::updateViewSize() const
{
    d->updateSize();
}

void SharedEngineView::resizeEvent(QResizeEvent *event)
{
    if (d->resizeMode == SizeRootObjectToView) {
        d->updateSize();
    }

    QQuickWindow::resizeEvent(event);
}

QQuickItem *SharedEngineView::rootObject() const
{
    return d->rootItem;
}

SharedEngineView::ResizeMode SharedEngineView::resizeMode() const
{
    return d->resizeMode;
}

} // LingmoUIQuick
