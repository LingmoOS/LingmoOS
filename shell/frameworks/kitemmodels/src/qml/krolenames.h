/*
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KROLENAMES_H
#define KROLENAMES_H

#include <QObject>
#include <qqml.h>

#include <memory>

class QAbstractItemModel;
class KRoleNamesPrivate;

/**
 * @class KRoleNames
 *
 * @brief A mapper between roles and role names of an attachee model.
 *
 * KRoleNames exposes runtime-invokable methods to map from roles to role names
 * and vice-versa. It can be used to retrieve data from a model in an imperative
 * fashion when enum with roles is not available at runtime (i.e. not exported
 * via Q_ENUM macro) but role names are known; or just to maintain consistency
 * with view delegates (which use role names as properties).
 *
 * @since 6.0
 */
class KRoleNames : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("KRoleNames can only be used as an attached property")
    QML_ATTACHED(KRoleNames)
    QML_ADDED_IN_MINOR_VERSION(1)
public:
    explicit KRoleNames(QObject *parent = nullptr);
    ~KRoleNames() override;

    /**
     * Maps role number to role name.
     *
     * Returns an empty string if role is not found in attachee model's
     * roleNames() hash map.
     *
     * @since 6.0
     */
    Q_INVOKABLE QByteArray roleName(int role) const;

    /**
     * Maps role name to role number.
     *
     * Returns -1 if role name is not found in attachee model's
     * roleNames() hash map.
     *
     * @since 6.0
     */
    Q_INVOKABLE int role(const QByteArray &roleName) const;

    static KRoleNames *qmlAttachedProperties(QObject *object);

private:
    std::unique_ptr<KRoleNamesPrivate> const d;
};

QML_DECLARE_TYPEINFO(KRoleNames, QML_HAS_ATTACHED_PROPERTIES)

#endif
