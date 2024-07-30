/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PRODUCTCLIENT_H
#define PRODUCTCLIENT_H

#include "clientbase.h"
#include "models/product.h"

namespace Bugzilla
{
class ProductClient : public ClientBase
{
public:
    using ClientBase::ClientBase;

    /// Gets a single Product by its name
    Product::Ptr get(KJob *kjob);
    KJob *get(const QString &idOrName);
};

} // namespace Bugzilla

#endif // PRODUCTCLIENT_H
