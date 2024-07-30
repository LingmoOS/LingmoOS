/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2012-2021 Harald Sitter <sitter@kde.org>
*/

#ifndef VERSION_H
#define VERSION_H

// NOTE: we want the cmakedefine so undefined cmake variables result in undefined macros result in build failures when they are used!
#cmakedefine PROJECT_VERSION "@PROJECT_VERSION@"

#endif // VERSION_H
