/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef COMMANDBASE_H
#define COMMANDBASE_H

#define BUGZILLA_MEMBER_PROPERTY(type, member)                                                                                                                 \
private:                                                                                                                                                       \
    Q_PROPERTY(type member MEMBER member)                                                                                                                      \
public:                                                                                                                                                        \
    type member

#endif // COMMANDBASE_H
