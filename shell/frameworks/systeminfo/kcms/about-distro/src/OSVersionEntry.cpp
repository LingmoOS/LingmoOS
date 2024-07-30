/*
    SPDX-FileCopyrightText: 2022 Jeremy Whiting <jeremy.whiting@collabora.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "OSVersionEntry.h"

OSVersionEntry::OSVersionEntry(const QString &distroName, const QString &versionId, const QString &buildId)
    : Entry(ki18n("Operating System:"), QString())
{
    m_nameVersion = (buildId.isEmpty() ? ki18nc("@label %1 is the distro name, %2 is the version", "%1 %2").subs(distroName).subs(versionId)
                                       : ki18nc("@label %1 is the distro name, %2 is the version, %3 is the 'build' which should be a number, or 'rolling'",
                                                "%1 %2 Build: %3")
                                             .subs(distroName)
                                             .subs(versionId)
                                             .subs(buildId));
}

QString OSVersionEntry::localizedValue(Language language) const
{
    return localize(m_nameVersion, language);
}
