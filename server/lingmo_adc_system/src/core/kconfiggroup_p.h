/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGGROUP_P_H
#define KCONFIGGROUP_P_H

#include "kconfiggroup.h"
#include <QVariant>

class KConfigGroup;

struct KConfigGroupGui {
    typedef bool (*kReadEntryGui)(const QByteArray &data, const char *key, const QVariant &input, QVariant &output);
    typedef bool (*kWriteEntryGui)(KConfigGroup *, const char *key, const QVariant &input, KConfigGroup::WriteConfigFlags flags);

    kReadEntryGui readEntryGui;
    kWriteEntryGui writeEntryGui;
};

extern KCONFIGCORE_EXPORT KConfigGroupGui _kde_internal_KConfigGroupGui;

#endif
