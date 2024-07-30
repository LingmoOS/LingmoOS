/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "quicksettings.h"

#include <KAuthorized>

#include <QCoreApplication>

Q_GLOBAL_STATIC(KNewStuffQuick::Settings, s_settings)

KNewStuffQuick::Settings *KNewStuffQuick::Settings::instance()
{
    return s_settings;
}

bool KNewStuffQuick::Settings::allowedByKiosk() const
{
    return KAuthorized::authorize(KAuthorized::GHNS);
}

#include "moc_quicksettings.cpp"
