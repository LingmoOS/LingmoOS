/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "newbug.h"

#include <QMetaProperty>

namespace Bugzilla
{
NewBug::NewBug(QObject *parent)
    : JsonCommand(parent)
{
}

NewBug::NewBug(const NewBug &other)
    : JsonCommand(other.parent())
{
    const auto propertyCount = staticMetaObject.propertyCount();
    for (int i = 0; i < propertyCount; ++i) {
        const auto property = staticMetaObject.property(i);
        property.write(this, property.read(&other));
    }
}

} // namespace Bugzilla

#include "moc_newbug.cpp"
