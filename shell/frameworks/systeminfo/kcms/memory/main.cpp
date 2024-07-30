/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2024 Thomas Duckworth <tduck973564@gmail.com>
*/

#include <KPluginFactory>
#include <KQuickConfigModule>

#include "CommandOutputContext.h"

class KCMMemory : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CommandOutputContext *infoOutputContext READ outputContext CONSTANT FINAL)
public:
    explicit KCMMemory(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        m_outputContext = new CommandOutputContext(QStringLiteral("pkexec"), {"dmidecode", "--type", "17"}, parent);
    }
    CommandOutputContext *outputContext() const
    {
        return m_outputContext;
    }

private:
    CommandOutputContext *m_outputContext;
};

K_PLUGIN_CLASS_WITH_JSON(KCMMemory, "kcm_memory.json")

#include "main.moc"
