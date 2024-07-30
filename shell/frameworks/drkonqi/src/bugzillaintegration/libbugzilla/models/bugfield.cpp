/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bugfield.h"

namespace Bugzilla
{
BugField::BugField(const QVariantHash &obj, QObject *parent)
    : QObject(parent)
{
    registerVariantConverters();

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        setProperty(qPrintable(it.key()), it.value());
    }
}

QList<BugFieldValue *> BugField::values() const
{
    return m_values;
}

void BugField::registerVariantConverters()
{
    static bool convertersRegistered = false;
    if (convertersRegistered) {
        return;
    }
    convertersRegistered = true;

    QMetaType::registerConverter<QVariantList, QList<BugFieldValue *>>([](QVariantList v) -> QList<BugFieldValue *> {
        QList<BugFieldValue *> list;
        list.reserve(v.size());
        for (const QVariant &variant : std::as_const(v)) {
            list.append(new BugFieldValue(variant.toHash()));
        }
        return list;
    });
}

BugFieldValue::BugFieldValue(const QVariantHash &obj, QObject *parent)
    : QObject(parent)
{
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        setProperty(qPrintable(it.key()), it.value());
    }
}

QString BugFieldValue::name() const
{
    return m_name;
}

} // namespace Bugzilla

#include "moc_bugfield.cpp"
