/*
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "krolenames.h"

#include <QAbstractItemModel>
#include <QQmlInfo>

class KRoleNamesPrivate
{
    KRoleNames *const q;

public:
    explicit KRoleNamesPrivate(KRoleNames *qq)
        : q(qq)
    {
    }

    QHash<int, QByteArray> roleNames() const;
    QAbstractItemModel *model() const;
};

KRoleNames::KRoleNames(QObject *parent)
    : QObject(parent)
    , d(new KRoleNamesPrivate(this))
{
    Q_ASSERT(parent);
    if (!d->model()) {
        qmlWarning(parent) << "KRoleNames must be attached to a QAbstractItemModel";
        return;
    }
}

KRoleNames::~KRoleNames() = default;

QByteArray KRoleNames::roleName(int role) const
{
    const auto map = d->roleNames();
    return map.value(role, QByteArray());
}

int KRoleNames::role(const QByteArray &roleName) const
{
    const auto map = d->roleNames();
    return map.key(roleName, -1);
}

KRoleNames *KRoleNames::qmlAttachedProperties(QObject *object)
{
    return new KRoleNames(object);
}

QHash<int, QByteArray> KRoleNamesPrivate::roleNames() const
{
    if (const auto m = model()) {
        return m->roleNames();
    }
    return {};
}

QAbstractItemModel *KRoleNamesPrivate::model() const
{
    return qobject_cast<QAbstractItemModel *>(q->parent());
}

#include "moc_krolenames.cpp"
