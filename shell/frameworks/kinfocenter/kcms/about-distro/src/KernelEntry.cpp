/*
    SPDX-FileCopyrightText: 2012-2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "KernelEntry.h"

#include <sys/utsname.h>

KernelEntry::KernelEntry()
    : Entry(ki18n("Kernel Version:"), QString())
{
}

QString KernelEntry::localizedValue(Language language) const
{
    struct utsname utsName;
    if (uname(&utsName) != 0) {
        return QString();
    }
    const int bits = QT_POINTER_SIZE == 8 ? 64 : 32;
    return localize(
        ki18nc("@label %1 is the kernel version, %2 CPU bit width (e.g. 32 or 64)", "%1 (%2-bit)").subs(utsName.release).subs(QString::number(bits)),
        language);
}
