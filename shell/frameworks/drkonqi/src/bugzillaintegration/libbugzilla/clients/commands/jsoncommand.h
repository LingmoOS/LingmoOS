/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef JSONCOMMAND_H
#define JSONCOMMAND_H

#include <QObject>

#include "commandbase.h"

namespace Bugzilla
{
class JsonCommand : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual QByteArray toJson() const;
    virtual QVariantHash toVariantHash() const;
};

} // namespace Bugzilla

Q_DECLARE_METATYPE(Bugzilla::JsonCommand *)

#endif // JSONCOMMAND_H
