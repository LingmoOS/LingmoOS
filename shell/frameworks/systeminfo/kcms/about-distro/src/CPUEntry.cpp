/*
    SPDX-FileCopyrightText: 2012-2021 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "CPUEntry.h"

#include <QMap>

#include <solid/processor.h>

#include "FancyString.h"

CPUEntry::CPUEntry()
    : Entry(KLocalizedString(), QString())
{
    m_label = ki18np("Processor:", "Processors:").subs(m_cpus.count());
}

QString CPUEntry::localizedValue(Language language) const
{
    const KLocalizedString l10nUnknown = ki18nc("unknown CPU type/product name; presented as `Processors: 4 × Unknown Type'", "Unknown Type");
    const QString unknownName = localize(l10nUnknown, language);

    // Format processor string
    // Group by processor name
    QMap<QString, int> processorMap;
    for (const Solid::Device &device : m_cpus) {
        const QString name = device.product();
        auto it = processorMap.find(name);
        if (it == processorMap.end()) {
            processorMap.insert(name, 1);
        } else {
            ++it.value();
        }
    }
    // Create a formatted list of grouped processors
    QStringList names;
    names.reserve(processorMap.count());
    const bool multipleProducts = processorMap.count() > 1;
    for (auto it = processorMap.constBegin(); it != processorMap.constEnd(); ++it) {
        const int count = it.value();
        QString name = FancyString::fromUgly(it.key());
        // In case solid doesn't have a product name available we'll either show no name or a placeholder.
        if (name.isEmpty() && !multipleProducts) {
            names.append(QString::number(count));
        } else {
            if (name.isEmpty()) {
                name = unknownName;
            }
            names.append(QStringLiteral("%1 × %2").arg(count).arg(name));
        }
    }

    return names.join(QLatin1String(", "));
}
