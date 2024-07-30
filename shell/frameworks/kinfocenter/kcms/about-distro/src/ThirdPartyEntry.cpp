/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jeremy.whiting@collabora.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ThirdPartyEntry.h"

#include <QDebug>
#include <QProcess>

ThirdPartyEntry::ThirdPartyEntry(const QString &scriptPath)
    : Entry(ki18nc("Unused but needs to be : to avoid assertion in Entry constructor", ":"), QString())
    , m_scriptPath(scriptPath)
{
    for (const auto &language : {Entry::Language::System, Entry::Language::English}) {
        QProcess process;

        switch (language) {
        case Entry::Language::English: {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert("LANGUAGE", "en_US:C"); // Add an environment variable
            process.setProcessEnvironment(env);
            break;
        }
        case Entry::Language::System:
            break;
        }

        process.start(scriptPath, QStringList());
        process.waitForFinished();
        processOutput(QString::fromUtf8(process.readAllStandardOutput()).split('\n'), language);
    }
}

QString ThirdPartyEntry::localizedLabel(Language language) const
{
    return m_labels[language];
}

QString ThirdPartyEntry::localizedValue(Language language) const
{
    return m_values[language];
}

void ThirdPartyEntry::processOutput(const QStringList &output, Language language)
{
    Q_ASSERT(output.size() >= 2);
    m_labels[language] = output.at(0);
    m_values[language] = output.at(1);
}
