/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptor_p.h"
#include "gattcharacteristic.h"

namespace BluezQt
{

GattDescriptorPrivate::GattDescriptorPrivate(const QString &uuid,
                                             const QStringList &flags,
                                             const QByteArray &initialValue,
                                             const GattCharacteristic *characteristic)
    : m_uuid(uuid)
    , m_characteristic(characteristic)
    , m_value(initialValue)
    , m_flags(flags)
{
    static std::uint8_t descNumber = 0;
    m_objectPath.setPath(m_characteristic->objectPath().path() + QStringLiteral("/desc") + QString::number(descNumber++));
}

}
