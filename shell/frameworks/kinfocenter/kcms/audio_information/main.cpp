/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2024 Shubham Arora <shubhamarora@protonmail.com>
*/

#include <KPluginFactory>
#include <KQuickConfigModule>

#include "CommandOutputContext.h"

class KCMAudioInformation : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CommandOutputContext *infoOutputContext READ outputContext CONSTANT FINAL)
public:
    explicit KCMAudioInformation(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        m_outputContext = new CommandOutputContext(QStringLiteral("pactl"), {"list"}, parent);
    }
    CommandOutputContext *outputContext() const
    {
        return m_outputContext;
    }

private:
    CommandOutputContext *m_outputContext;
};

K_PLUGIN_CLASS_WITH_JSON(KCMAudioInformation, "kcm_audio_information.json")

#include "main.moc"
