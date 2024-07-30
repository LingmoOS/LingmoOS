/*
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PRODUCT_H
#define PRODUCT_H

#include <QObject>

#include "connection.h"

namespace Bugzilla
{
class ProductVersion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id MEMBER m_id NOTIFY changed)
    Q_PROPERTY(QString name READ name MEMBER m_name NOTIFY changed)
    Q_PROPERTY(bool is_active READ isActive MEMBER m_active NOTIFY changed)
public:
    int id() const
    {
        return m_id;
    }
    QString name() const
    {
        return m_name;
    }
    bool isActive() const
    {
        return m_active;
    }

    explicit ProductVersion(const QVariantHash &object, QObject *parent = nullptr);

Q_SIGNALS:
    void changed();

private:
    int m_id = -1;
    QString m_name = QString();
    bool m_active = false;
};

class ProductComponent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id MEMBER m_id NOTIFY changed)
    Q_PROPERTY(QString name READ name MEMBER m_name NOTIFY changed)
public:
    int id() const
    {
        return m_id;
    }
    QString name() const
    {
        return m_name;
    }

    explicit ProductComponent(const QVariantHash &object, QObject *parent = nullptr);

Q_SIGNALS:
    void changed();

private:
    int m_id = -1;
    QString m_name = QString();
};

class Product final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool is_active READ isActive MEMBER m_active NOTIFY changed)
    Q_PROPERTY(QList<Bugzilla::ProductComponent *> components READ components MEMBER m_components NOTIFY changed)
    Q_PROPERTY(QList<Bugzilla::ProductVersion *> versions READ versions MEMBER m_versions NOTIFY changed)
public:
    using Ptr = QSharedPointer<Product>;

    explicit Product(const QVariantHash &object, const Connection &connection = Bugzilla::connection(), QObject *parent = nullptr);
    ~Product() final;

    bool isActive() const;
    QList<ProductComponent *> components() const;
    QList<ProductVersion *> versions() const;

    // Convenience methods to get useful content out of the
    QStringList componentNames() const;
    QStringList allVersions() const;
    QStringList inactiveVersions() const;

Q_SIGNALS:
    void changed();

private:
    static void registerVariantConverters();

    const Connection &m_connection;

    bool m_active = false;
    QList<ProductComponent *> m_components;
    QList<ProductVersion *> m_versions;

    Q_DISABLE_COPY_MOVE(Product)
};

} // namespace Bugzilla

Q_DECLARE_METATYPE(Bugzilla::ProductComponent *)
Q_DECLARE_METATYPE(QList<Bugzilla::ProductComponent *>)
Q_DECLARE_METATYPE(Bugzilla::ProductVersion *)
Q_DECLARE_METATYPE(QList<Bugzilla::ProductVersion *>)

#endif // PRODUCT_H
