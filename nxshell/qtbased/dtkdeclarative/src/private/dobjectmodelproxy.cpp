// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobjectmodelproxy_p.h"

#include <QQuickItem>
#include <QtQml>
#include <private/qqmlchangeset_p.h>

DQUICK_BEGIN_NAMESPACE

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(cfLog, "dtk.quick.core" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(cfLog, "dtk.quick.core");
#endif

template<class DataType>
class Compositor
{
public:
    using OriginIndex = int;
    using CurrentIndex = int;

    ~Compositor() {}

    CurrentIndex itemIndex(QObject *item) const {
        const OriginIndex originIndex = dataItems.indexOf(item);
        if (originIndex != -1) {
            return currentDatas.indexOf(datas[originIndex]);
        }
        return -1;
    }

    CurrentIndex itemIndex(OriginIndex originIndex) const {
        if (datas.count() <= originIndex || originIndex < 0) {
            return -1;
        }
        return currentDatas.indexOf(datas[originIndex]);
    }

    inline QObject *object(CurrentIndex index) const {
        if (index < 0 || index >= currentDatas.count())
            return nullptr;

        return dataItems[originIndex(index)];
    }
    inline const QVector<DataType *> &objects() const {
        return currentDatas;
    }
    inline void setObject(CurrentIndex index, QObject *obj) {
        dataItems[originIndex(index)] = obj;
    }

    void reset(const QVector<DataType *> &now)
    {
        currentDatas.clear();
        datas = now;
        dataItems.fill(nullptr, datas.count());
    }

    CurrentIndex insert(DataType *group)
    {
        const OriginIndex index = originIndex(group);
        for (int i = 0; i < currentDatas.count(); ++i)
        {
            // find insert position by origin order.
            if (index <= originIndex(currentDatas[i])) {
                currentDatas.insert(i, group);
                return i;
            }
        }
        currentDatas.append(group);
        return currentDatas.count() - 1;
    }

    CurrentIndex remove(DataType *group)
    {
        const CurrentIndex index = currentDatas.indexOf(group);
        if (index != -1) {
            currentDatas.remove(index);
        }

        return index;
    }

public:
    inline OriginIndex originIndex(CurrentIndex index) const {
        return originIndex(currentDatas[index]);
    }
    inline OriginIndex originIndex(DataType *group) const {
        return datas.indexOf(group);
    }

    // origin data
    QVector<DataType *> datas;
    // QuickItem for each origin data
    QVector<QObject *> dataItems;
    // display data
    QVector<DataType *> currentDatas;
};

class ObjectModelProxyPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(ObjectModelProxy)
public:
    explicit ObjectModelProxyPrivate()
    {
    }
    virtual ~ObjectModelProxyPrivate() override;

    const QVector<QQuickItem *> &datas() const
    {
        return compositor.objects();
    }

    bool isAcceptedItem(const QJSValueList &args)
    {
        if (!filterAcceptsItem.isCallable())
            return true;

        const auto &result = filterAcceptsItem.call(args);
        if (result.isError()) {
            qCWarning(cfLog) << "ObjectModelProxy::isAcceptedItem() failed: can't call filterAcceptsItem." << result.toString();
            return false;
        }
        return result.toBool();
    }

    void reset(const QVector<QQuickItem *> &datas)
    {
        Q_Q(ObjectModelProxy);
        compositor.reset(datas);

        QQmlChangeSet changeSet;
        const auto engine = qmlEngine(q);
        if (!engine)
            return;

        for (auto item :compositor.datas) {
            auto argu1 = engine->newQObject(item);
            const bool accept = isAcceptedItem({argu1});
            if (accept) {
                auto currIndex = compositor.insert(item);
                changeSet.insert(currIndex, 1);
            }
        }

        if (!changeSet.isEmpty()) {
            Q_EMIT q->modelUpdated(changeSet, true);
            Q_EMIT q->countChanged();
        }
    }

    void update()
    {
        Q_Q(ObjectModelProxy);
        QQmlChangeSet changeSet;
        auto objects = compositor.objects();
        const auto engine = qmlEngine(q);
        for (auto item :compositor.datas) {
            const bool exists = objects.contains(item);
            auto argu1 = engine->newQObject(item);
            const bool accept = isAcceptedItem({argu1});
            if (accept && !exists) {
                auto currIndex = compositor.insert(item);
                changeSet.insert(currIndex, 1);
            } else if (!accept && exists) {
                auto currIndex = compositor.remove(item);
                changeSet.remove(currIndex, 1);
            }
        }
        if (!changeSet.isEmpty()) {
            Q_Q(ObjectModelProxy);
            Q_EMIT q->modelUpdated(changeSet, false);
            Q_EMIT q->countChanged();
        }
    }

    QQmlObjectModel *impl = nullptr;
    Compositor<QQuickItem> compositor;
    QJSValue filterAcceptsItem;
};
ObjectModelProxyPrivate::~ObjectModelProxyPrivate() {}

ObjectModelProxy::ObjectModelProxy(QObject *parent)
    : QQmlInstanceModel (*(new ObjectModelProxyPrivate()), parent)
{
}

bool ObjectModelProxy::isValid() const
{
    Q_D(const ObjectModelProxy);
    return d->impl;
}

int ObjectModelProxy::count() const
{
    Q_D(const ObjectModelProxy);

    return d->datas().count();
}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
QVariant ObjectModelProxy::variantValue(int index, const QString &name)
{
    Q_D(ObjectModelProxy);

    const auto &datas = d->datas();
    if (index < 0 || index >= datas.count())
        return QString();
    return QQmlEngine::contextForObject(datas.at(index))->contextProperty(name);
}

QQmlInstanceModel::ReleaseFlags ObjectModelProxy::release(QObject *object, ReusableFlag reusableFlag)
{
    Q_UNUSED(reusableFlag);
    Q_D(const ObjectModelProxy);

    if (d->compositor.itemIndex(object) >= 0) {
        return QQmlInstanceModel::Referenced;
    }
    return ReleaseFlags();
}
#else
QString ObjectModelProxy::stringValue(int index, const QString &name)
{
    Q_D(ObjectModelProxy);

    const auto &datas = d->datas();
    if (index < 0 || index >= datas.count())
        return QString();
    return QQmlEngine::contextForObject(datas.at(index))->contextProperty(name).toString();
}

QQmlInstanceModel::ReleaseFlags ObjectModelProxy::release(QObject *object)
{
    Q_D(const ObjectModelProxy);

    if (d->compositor.itemIndex(object) >= 0) {
        return QQmlInstanceModel::Referenced;
    }
    return nullptr;
}
#endif

void ObjectModelProxy::setWatchedRoles(const QList<QByteArray> &roles)
{
    Q_UNUSED(roles);
}

QQmlIncubator::Status ObjectModelProxy::incubationStatus(int index)
{
    Q_D(ObjectModelProxy);
    if (d->compositor.object(index)) {
        return QQmlIncubator::Ready;
    }
    return QQmlIncubator::Loading;
}

int ObjectModelProxy::indexOf(QObject *object, QObject *objectContext) const
{
    Q_UNUSED(objectContext)

    Q_D(const ObjectModelProxy);
    return d->compositor.itemIndex(object);
}

QObject *ObjectModelProxy::object(int index, QQmlIncubator::IncubationMode incubationMode)
{
    Q_D(ObjectModelProxy);

    if (auto obj = d->compositor.object(index))
        return obj;

    auto originIndex = d->compositor.originIndex(index);
    auto item = d->impl->object(originIndex, incubationMode);

    d->compositor.setObject(index, item);
    return item;
}

QQmlObjectModel *ObjectModelProxy::sourceModel() const
{
    Q_D(const ObjectModelProxy);
    return d->impl;
}

QJSValue ObjectModelProxy::filterAcceptsItem() const
{
    Q_D(const ObjectModelProxy);
    return d->filterAcceptsItem;
}

void ObjectModelProxy::update()
{
    Q_D(ObjectModelProxy);
    d->update();
}

QObject *ObjectModelProxy::get(const int i) const
{
    Q_D(const ObjectModelProxy);
    if (count() <= i || i < 0)
        return nullptr;

    return d->datas().at(i);
}

int ObjectModelProxy::mapToSource(const int i) const
{
    Q_D(const ObjectModelProxy);
    if (count() <= i || i < 0)
        return -1;

    return d->compositor.originIndex(i);
}

int ObjectModelProxy::mapFromSource(const int i) const
{
    Q_D(const ObjectModelProxy);
    return d->compositor.itemIndex(i);
}

void ObjectModelProxy::setFilterAcceptsItem(QJSValue filterAcceptsItem)
{
    Q_D(ObjectModelProxy);
    if (!filterAcceptsItem.isCallable() || d->filterAcceptsItem.equals(filterAcceptsItem))
        return;

    d->filterAcceptsItem = filterAcceptsItem;
    Q_EMIT filterAcceptsItemChanged();
}

void ObjectModelProxy::syncDataFromItem()
{
    Q_D(ObjectModelProxy);
    QVector<QQuickItem *> items;
    for (int i = 0; i < d->impl->count(); ++i) {
        if (auto item = qobject_cast<QQuickItem *>(d->impl->get(i))) {
            items << item;
        }
    }
    d->reset(items);
}

void ObjectModelProxy::setSourceModel(QQmlObjectModel *sourceModel)
{
    Q_D(ObjectModelProxy);
    if (d->impl) {
        disconnect(d->impl, &QQmlObjectModel::countChanged, this, &ObjectModelProxy::syncDataFromItem);
    }

    d->impl = sourceModel;
    syncDataFromItem();
    connect(d->impl, &QQmlObjectModel::countChanged, this, &ObjectModelProxy::syncDataFromItem);
}

DQUICK_END_NAMESPACE
