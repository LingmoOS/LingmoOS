/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2021 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OCEANICONS_H
#define OCEANICONS_H

#include <oceanicons_export.h>

/**
 * This namespace contains utility functions to integrate the
 * Ocean icons into your application.
 */
namespace OceanIcons
{

/**
 * Ensures the Ocean icon theme is available inside the :/icons resource folder
 * and sets it up as QIcon fallback theme if no other fallback theme
 * is already set (or it's set to hicolor which is basically another way of
 * no other fallback theme set).
 *
 * @since 6.0
 */
OCEANICONS_EXPORT void initIcons();

}

#endif
