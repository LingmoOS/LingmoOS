/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "drkonqi_globals.h"

#include <KLocalizedString>

KGuiItem2 DrStandardGuiItem::appRestart()
{
    return {i18nc("@action:button", "&Restart Application"),
            QIcon::fromTheme(QStringLiteral("system-reboot")),
            i18nc("@info:tooltip", "Use this button to restart the crashed application.")};
}
