/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Property.h"

Property::Property()
{
}

bool Property::finalize()
{
    for (auto tag : m_stringTags) {
        m_tags.isOptional |= tag.contains(QLatin1String("optional"), Qt::CaseInsensitive);
        m_tags.isExperimental |= tag.contains(QLatin1String("experimental"), Qt::CaseInsensitive);
        m_tags.isReadOnly |= tag.contains(QLatin1String("read-only"), Qt::CaseInsensitive);
    }
    m_tags.isServerOnly = m_limitation.contains(QLatin1String("server only"), Qt::CaseInsensitive);

    bool success = true;
    success &= m_comment.finalize();

    return success;
}

QString Property::name() const
{
    return m_name;
}

QString Property::type() const
{
    return m_type;
}

Property::Tags Property::tags() const
{
    return m_tags;
}

QStringList Property::comment() const
{
    return m_comment;
}
