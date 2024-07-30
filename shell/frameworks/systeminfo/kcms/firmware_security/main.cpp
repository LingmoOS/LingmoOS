/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
*/

#include <KPluginFactory>
#include <KQuickConfigModule>
#include <QStandardPaths>

#include "CommandOutputContext.h"

class KCMFirmwareSecurity : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CommandOutputContext *infoOutputContext READ outputContext CONSTANT FINAL)
public:
    explicit KCMFirmwareSecurity(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        const QString executable = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                          QStringLiteral("kinfocenter/firmware_security/fwupdmgr.sh"),
                                                          QStandardPaths::LocateFile);
        m_outputContext = new CommandOutputContext({QStringLiteral("fwupdmgr"), QStringLiteral("aha")}, QStringLiteral("/bin/sh"), {executable}, parent);
    }
    CommandOutputContext *outputContext() const
    {
        return m_outputContext;
    }

private:
    CommandOutputContext *m_outputContext;
};

K_PLUGIN_CLASS_WITH_JSON(KCMFirmwareSecurity, "kcm_firmware_security.json")

#include "main.moc"
