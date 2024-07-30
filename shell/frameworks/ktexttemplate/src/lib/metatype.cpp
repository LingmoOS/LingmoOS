/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Michael Jansen <kde@michael-jansen.biz>
  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "metatype.h"

#include "customtyperegistry_p.h"
#include "metaenumvariable_p.h"

#include <QAssociativeIterable>
#include <QDebug>
#include <QSequentialIterable>

using namespace KTextTemplate;

Q_GLOBAL_STATIC(CustomTypeRegistry, customTypes)

void KTextTemplate::MetaType::internalLock()
{
    return customTypes()->mutex.lock();
}

void KTextTemplate::MetaType::internalUnlock()
{
    return customTypes()->mutex.unlock();
}

void KTextTemplate::MetaType::registerLookUpOperator(int id, LookupFunction f)
{
    Q_ASSERT(id > 0);
    Q_ASSERT(f);

    customTypes()->registerLookupOperator(id, f);
}

static QVariant doQobjectLookUp(const QObject *const object, const QString &property)
{
    if (!object)
        return {};
    if (property == QStringLiteral("children")) {
        const auto &childList = object->children();
        if (childList.isEmpty())
            return {};
        QVariantList children;

        auto it = childList.constBegin();
        const auto end = childList.constEnd();
        for (; it != end; ++it)
            children.append(QVariant::fromValue(*it));
        return children;
    }

    if (property == QStringLiteral("objectName")) {
        return object->objectName();
    }
    // Can't be const because of invokeMethod.
    auto metaObj = object->metaObject();

    QMetaProperty mp;
    for (auto i = 0; i < metaObj->propertyCount(); ++i) {
        // TODO only read-only properties should be allowed here.
        // This might also handle the variant messing I hit before.
        mp = metaObj->property(i);

        if (QString::fromUtf8(mp.name()) != property)
            continue;

        if (mp.isEnumType()) {
            MetaEnumVariable mev(mp.enumerator(), mp.read(object).value<int>());
            return QVariant::fromValue(mev);
        }

        return mp.read(object);
    }
    QMetaEnum me;
    for (auto i = 0; i < metaObj->enumeratorCount(); ++i) {
        me = metaObj->enumerator(i);

        if (QLatin1String(me.name()) == property) {
            MetaEnumVariable mev(me);
            return QVariant::fromValue(mev);
        }

        const auto value = me.keyToValue(property.toLatin1().constData());

        if (value < 0)
            continue;

        const MetaEnumVariable mev(me, value);

        return QVariant::fromValue(mev);
    }
    return object->property(property.toUtf8().constData());
}

QVariant KTextTemplate::MetaType::lookup(const QVariant &object, const QString &property)
{
    if (object.canConvert<QObject *>()) {
        return doQobjectLookUp(object.value<QObject *>(), property);
    }
    if (object.canConvert<QVariantList>()) {
        auto iter = object.value<QSequentialIterable>();
        if (property == QStringLiteral("size") || property == QStringLiteral("count")) {
            return iter.size();
        }

        auto ok = false;
        const auto listIndex = property.toInt(&ok);

        if (!ok || listIndex >= iter.size()) {
            return {};
        }

        return iter.at(listIndex);
    }
    if (object.canConvert<QVariantHash>()) {
        auto iter = object.value<QAssociativeIterable>();

        if (iter.find(property) != iter.end()) {
            return iter.value(property);
        }

        if (property == QStringLiteral("size") || property == QStringLiteral("count")) {
            return iter.size();
        }

        if (property == QStringLiteral("items")) {
            auto it = iter.begin();
            const auto end = iter.end();
            QVariantList list;
            for (; it != end; ++it) {
                list.push_back(QVariantList{it.key(), it.value()});
            }
            return list;
        }

        if (property == QStringLiteral("keys")) {
            auto it = iter.begin();
            const auto end = iter.end();
            QVariantList list;
            for (; it != end; ++it) {
                list.push_back(it.key());
            }
            return list;
        }

        if (property == QStringLiteral("values")) {
            auto it = iter.begin();
            const auto end = iter.end();
            QVariantList list;
            for (; it != end; ++it) {
                list.push_back(it.value());
            }
            return list;
        }

        return {};
    }
    auto mo = QMetaType(object.userType()).metaObject();
    if (mo) {
        QMetaType mt(object.userType());
        if (mt.flags().testFlag(QMetaType::IsGadget)) {
            const auto idx = mo->indexOfProperty(property.toUtf8().constData());
            if (idx >= 0) {
                const auto mp = mo->property(idx);

                if (mp.isEnumType()) {
                    MetaEnumVariable mev(mp.enumerator(), mp.readOnGadget(object.constData()).value<int>());
                    return QVariant::fromValue(mev);
                }

                return mp.readOnGadget(object.constData());
            }

            QMetaEnum me;
            for (auto i = 0; i < mo->enumeratorCount(); ++i) {
                me = mo->enumerator(i);

                if (QLatin1String(me.name()) == property) {
                    MetaEnumVariable mev(me);
                    return QVariant::fromValue(mev);
                }

                const auto value = me.keyToValue(property.toLatin1().constData());

                if (value < 0) {
                    continue;
                }

                MetaEnumVariable mev(me, value);
                return QVariant::fromValue(mev);
            }
        }
    }

    return customTypes()->lookup(object, property);
}

bool KTextTemplate::MetaType::lookupAlreadyRegistered(int id)
{
    return customTypes()->lookupAlreadyRegistered(id);
}
