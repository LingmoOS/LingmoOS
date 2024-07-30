/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KQuickConfigModule>

#include "ksambasharemodel.h"
#include "smbmountmodel.h"

class SambaModule : public KQuickConfigModule
{
    Q_OBJECT
public:
    SambaModule(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        qmlRegisterType<SmbMountModel>("org.kde.kinfocenter.samba", 1, 0, "MountModel");
        qmlRegisterType<KSambaShareModel>("org.kde.kinfocenter.samba", 1, 0, "ShareModel");

        setButtons(Help);
    }
};

K_PLUGIN_CLASS_WITH_JSON(SambaModule, "smbstatus.json")

#include "main.moc"
