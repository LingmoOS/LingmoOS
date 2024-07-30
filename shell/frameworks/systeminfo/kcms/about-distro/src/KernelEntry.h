/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KERNELENTRY_H
#define KERNELENTRY_H

#include "Entry.h"

class KernelEntry : public Entry
{
public:
    KernelEntry();
    QString localizedValue(Language language = Language::System) const override;
};

#endif // KERNELENTRY_H
