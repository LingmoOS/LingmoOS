/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexfiletransferentry.h"

#include <QDateTime>
#include <QVariant>

namespace BluezQt
{
class ObexFileTransferEntryPrivate
{
public:
    QString m_name;
    QString m_label;
    ObexFileTransferEntry::Type m_type;
    quint64 m_size;
    QString m_permissions;
    QString m_memoryType;
    QDateTime m_modified;
};

static QDateTime dateTimeFromTransfer(const QString &value)
{
    return QDateTime::fromString(value, QStringLiteral("yyyyMMddThhmmssZ"));
}

ObexFileTransferEntry::ObexFileTransferEntry()
    : d(new ObexFileTransferEntryPrivate)
{
    d->m_type = Invalid;
    d->m_size = 0;
}

ObexFileTransferEntry::ObexFileTransferEntry(const QVariantMap &properties)
    : d(new ObexFileTransferEntryPrivate)
{
    d->m_name = properties.value(QStringLiteral("Name")).toString();
    d->m_label = properties.value(QStringLiteral("Label")).toString();
    d->m_size = properties.value(QStringLiteral("Size")).toUInt();
    d->m_permissions = properties.value(QStringLiteral("User-perm")).toString();
    d->m_memoryType = properties.value(QStringLiteral("Mem-type")).toString();
    d->m_modified = dateTimeFromTransfer(properties.value(QStringLiteral("Modified")).toString());

    const QString &type = properties.value(QStringLiteral("Type")).toString();
    if (type == QLatin1String("folder")) {
        d->m_type = ObexFileTransferEntry::Folder;
    } else if (type == QLatin1String("file")) {
        d->m_type = ObexFileTransferEntry::File;
    } else {
        d->m_type = ObexFileTransferEntry::Invalid;
    }
}

ObexFileTransferEntry::~ObexFileTransferEntry()
{
}

ObexFileTransferEntry::ObexFileTransferEntry(const ObexFileTransferEntry &other)
    : d(other.d)
{
}

ObexFileTransferEntry &ObexFileTransferEntry::operator=(const ObexFileTransferEntry &other)
{
    if (d != other.d) {
        d = other.d;
    }
    return *this;
}

bool ObexFileTransferEntry::isValid() const
{
    return d->m_type != Invalid;
}

QString ObexFileTransferEntry::name() const
{
    return d->m_name;
}

QString ObexFileTransferEntry::label() const
{
    return d->m_label;
}

ObexFileTransferEntry::Type ObexFileTransferEntry::type() const
{
    return d->m_type;
}

quint64 ObexFileTransferEntry::size() const
{
    return d->m_size;
}

QString ObexFileTransferEntry::permissions() const
{
    return d->m_permissions;
}

QString ObexFileTransferEntry::memoryType() const
{
    return d->m_memoryType;
}

QDateTime ObexFileTransferEntry::modificationTime() const
{
    return d->m_modified;
}

} // namespace BluezQt
