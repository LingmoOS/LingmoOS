// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "spellcheckattached.h"
#include <QQuickItem>

SpellCheckAttached::SpellCheckAttached(QObject *parent)
    : QObject(parent)
{
}

SpellCheckAttached::~SpellCheckAttached()
{
}

void SpellCheckAttached::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        return;
    }

    m_enabled = enabled;
    Q_EMIT enabledChanged();
}

bool SpellCheckAttached::enabled() const
{
    return m_enabled;
}

SpellCheckAttached *SpellCheckAttached::qmlAttachedProperties(QObject *object)
{
    return new SpellCheckAttached(object);
}

#include "moc_spellcheckattached.cpp"
