/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KCONFIG_AUTOTESTS_HELPER_H
#define KCONFIG_AUTOTESTS_HELPER_H

#include <QLocale>

/**
 * @brief Helper to store the current default QLocale and resetting to it in the dtor.
 **/
class DefaultLocale
{
public:
    DefaultLocale()
        : m_defaultLocale()
    {
    }
    ~DefaultLocale()
    {
        QLocale::setDefault(m_defaultLocale);
    }

private:
    QLocale m_defaultLocale;
};

#endif
