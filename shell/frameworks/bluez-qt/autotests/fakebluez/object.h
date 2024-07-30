/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <QDBusObjectPath>
#include <QVariant>

class Object
{
public:
    explicit Object();
    virtual ~Object() = default;

    QObject *objectParent() const;
    void setObjectParent(QObject *parent);

    QDBusObjectPath path() const;
    void setPath(const QDBusObjectPath &path);

    QString name() const;
    void setName(const QString &name);

    bool haveProperties() const;

    QVariantMap properties() const;
    void setProperties(const QVariantMap &properties);

    QVariant property(const QString &name) const;
    void changeProperty(const QString &name, const QVariant &value);

private:
    QObject *m_parent;
    QDBusObjectPath m_path;
    QString m_name;
    QVariantMap m_properties;
};

#endif // OBJECT_H
