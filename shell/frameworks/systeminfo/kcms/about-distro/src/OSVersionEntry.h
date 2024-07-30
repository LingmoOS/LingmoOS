/*
    SPDX-FileCopyrightText: 2022 Jeremy Whiting <jeremy.whiting@collabora.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "Entry.h"

class OSVersionEntry : public Entry
{
public:
    OSVersionEntry(const QString &distroName, const QString &versionId, const QString &buildId);
    QString localizedValue(Language language = Language::System) const override;

private:
    KLocalizedString m_nameVersion;
};
