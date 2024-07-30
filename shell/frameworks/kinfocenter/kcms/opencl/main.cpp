/*
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2022 Linus Dierheimer <linus@dierheimer.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KPluginFactory>
#include <KQuickConfigModule>

#include "CommandOutputContext.h"

class KCMOpenCL : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CommandOutputContext *infoOutputContext READ outputContext CONSTANT FINAL)
public:
    explicit KCMOpenCL(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        m_outputContext = new CommandOutputContext(QStringLiteral("clinfo"), {}, parent);
    }
    CommandOutputContext *outputContext() const
    {
        return m_outputContext;
    }

private:
    CommandOutputContext *m_outputContext;
};

K_PLUGIN_CLASS_WITH_JSON(KCMOpenCL, "kcm_opencl.json")

#include "main.moc"
