/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KPluginFactory>
#include <KQuickConfigModule>

class KCMPulseAudio : public KQuickConfigModule
{
    Q_OBJECT
public:
    KCMPulseAudio(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        setButtons(Help);
    }
};
K_PLUGIN_CLASS_WITH_JSON(KCMPulseAudio, "kcm_pulseaudio.json")

#include "module.moc"
