/*
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "abstractbackend.h"

void KScreen::AbstractBackend::init(const QVariantMap &arguments)
{
    Q_UNUSED(arguments);
}

QByteArray KScreen::AbstractBackend::edid(int outputId) const
{
    Q_UNUSED(outputId);
    return QByteArray();
}

#include "moc_abstractbackend.cpp"
