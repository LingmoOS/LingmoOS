/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "enums.h"

Enums::Enums(QObject *parent)
    : QObject(parent)
{
}

Enums::~Enums() = default;

#include "moc_enums.cpp"
