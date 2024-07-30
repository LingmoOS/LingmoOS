/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATIONPRIVATE_EXPORT_H
#define SYNDICATIONPRIVATE_EXPORT_H

#include "syndication_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef SYNDICATION_TESTS_EXPORT
#define SYNDICATION_TESTS_EXPORT SYNDICATION_EXPORT
#endif
#else /* not compiling tests */
#define SYNDICATION_TESTS_EXPORT
#endif

#endif
