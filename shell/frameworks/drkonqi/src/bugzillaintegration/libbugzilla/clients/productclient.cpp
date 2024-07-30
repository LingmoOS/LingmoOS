/*
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "productclient.h"

#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaType>
#include <QSharedPointer>

namespace Bugzilla
{
Product::Ptr ProductClient::get(KJob *kjob)
{
    auto job = qobject_cast<APIJob *>(kjob);

    const QJsonArray productsArray = job->object().value(QLatin1String("products")).toArray();
    if (productsArray.isEmpty()) {
        throw Bugzilla::RuntimeException(QStringLiteral("Failed to resolve bugzilla product"));
    }
    Q_ASSERT(productsArray.size() == 1);

    auto obj = productsArray.at(0).toObject().toVariantHash();

    return Product::Ptr(new Product(obj, m_connection));
}

KJob *ProductClient::get(const QString &idOrName)
{
    return m_connection.get(QStringLiteral("/product/%1").arg(idOrName));
}

} // namespace Bugzilla
