/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
*/

#include <KPluginFactory>
#include <KQuickConfigModule>
#include <QLibraryInfo>

#include "CommandOutputContext.h"

class KCMKWinSupportInfo : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CommandOutputContext *infoOutputContext READ outputContext CONSTANT FINAL)
public:
    explicit KCMKWinSupportInfo(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        m_outputContext = new CommandOutputContext(QLibraryInfo::path(QLibraryInfo::BinariesPath) + QStringLiteral("/qdbus"),
                                                   {QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"), QStringLiteral("supportInformation")},
                                                   parent);
    }
    CommandOutputContext *outputContext() const
    {
        return m_outputContext;
    }

private:
    CommandOutputContext *m_outputContext;
};

K_PLUGIN_CLASS_WITH_JSON(KCMKWinSupportInfo, "kcm_kwinsupportinfo.json")

#include "main.moc"
