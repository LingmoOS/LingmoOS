/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knotifyconfigelement.h"

#include <KConfig>
#include <KConfigGroup>

KNotifyConfigElement::KNotifyConfigElement(const QString &eventid, KConfig *config)
    : m_config(new KConfigGroup(config, QStringLiteral("Event/") + eventid))
    , m_eventId(eventid)
{
}

KNotifyConfigElement::~KNotifyConfigElement()
{
    delete m_config;
}

QString KNotifyConfigElement::readEntry(const QString &entry, bool path)
{
    if (m_cache.contains(entry)) {
        return m_cache[entry];
    }
    return path ? m_config->readPathEntry(entry, QString()) : m_config->readEntry(entry, QString());
}

void KNotifyConfigElement::writeEntry(const QString &entry, const QString &data)
{
    m_cache[entry] = data;
}

QString KNotifyConfigElement::eventId() const
{
    return m_eventId;
}

void KNotifyConfigElement::save()
{
    QMap<QString, QString>::const_iterator it = m_cache.constBegin();
    for (; it != m_cache.constEnd(); ++it) {
        m_config->writeEntry(it.key(), it.value());
    }
}
