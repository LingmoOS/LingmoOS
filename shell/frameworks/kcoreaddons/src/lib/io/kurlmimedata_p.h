/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2023 Jin Liu <m.liu.jin@gmial.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

// Internal functions for tests

namespace KUrlMimeData
{
#if HAVE_QTDBUS
KCOREADDONS_EXPORT bool isDocumentsPortalAvailable();
KCOREADDONS_EXPORT void setSourceId(QMimeData *mimeData);
#endif // HAVE_QTDBUS
}
