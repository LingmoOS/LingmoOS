/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cstdlib>
#include <iostream>

#include <KConfigGroup>
#include <KSharedConfig>

using namespace Qt::StringLiterals;

/**
 * For Lingmo 5 users, keep the existing panel floating settings.
 *
 * @see https://invent.kde.org/lingmo/lingmo-desktop/-/issues/73
 * @since 6.0
 */
int main()
{
    const KSharedConfigPtr configPtr = KSharedConfig::openConfig(QStringLiteral("lingmoshellrc"), KConfig::FullConfig);
    KConfigGroup views(configPtr, QStringLiteral("LingmoViews"));
    if (!views.exists() || views.groupList().empty()) {
        std::cout << "lingmoshellrc doesn't have any LingmoViews. No need to update config." << std::endl;
        return EXIT_SUCCESS;
    }

    // Update default floating setting in config groups like [LingmoViews][Panel 114][Defaults]
    const QStringList groupList = views.groupList();
    for (const QString &name : groupList) {
        if (!name.startsWith(QLatin1String("Panel "))) {
            continue;
        }

        KConfigGroup panelConfigGroup(&views, name);
        if (panelConfigGroup.hasKey("floating")) {
            // Respect the manual setting
            continue;
        }

        // Explicitly set the old default floating setting for panels from Lingmo 5
        panelConfigGroup.writeEntry("floating", 0);
    }

    return configPtr->sync() ? EXIT_SUCCESS : EXIT_FAILURE;
}
