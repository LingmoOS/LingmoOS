/*
    SPDX-FileCopyrightText: 2000 Alex Zepeda <zipzippy@sonic.net>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include "kemailsettings.h"

#include <kconfig.h>
#include <kconfiggroup.h>

class KEMailSettingsPrivate
{
public:
    KEMailSettingsPrivate()
        : m_pConfig(nullptr)
    {
    }
    ~KEMailSettingsPrivate()
    {
        delete m_pConfig;
    }
    KConfig *m_pConfig;
    QStringList profiles;
    QString m_sDefaultProfile, m_sCurrentProfile;
};

QString KEMailSettings::defaultProfileName() const
{
    return p->m_sDefaultProfile;
}

QString KEMailSettings::getSetting(KEMailSettings::Setting s) const
{
    KConfigGroup cg(p->m_pConfig, QLatin1String("PROFILE_") + p->m_sCurrentProfile);
    switch (s) {
    case ClientProgram: {
        return cg.readEntry("EmailClient");
    }
    case ClientTerminal: {
        return cg.readEntry("TerminalClient", QVariant(false)).toString();
    }
    case RealName: {
        return cg.readEntry("FullName");
    }
    case EmailAddress: {
        return cg.readEntry("EmailAddress");
    }
    case ReplyToAddress: {
        return cg.readEntry("ReplyAddr");
    }
    case Organization: {
        return cg.readEntry("Organization");
    }
    case OutServer: {
        return cg.readEntry("OutgoingServer");
    }
    case OutServerLogin: {
        return cg.readEntry("OutgoingUserName");
    }
    case OutServerPass: {
        return cg.readEntry("OutgoingPassword");
    }
#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
    case OutServerType: {
        return cg.readEntry("OutgoingServerType");
    }
    case OutServerCommand: {
        return cg.readEntry("OutgoingCommand");
    }
    case OutServerTLS: {
        return cg.readEntry("OutgoingServerTLS", QVariant(false)).toString();
    }
#else
    case OutServerType_DEPRECATED_DO_NOT_USE:
    case OutServerCommand_DEPRECATED_DO_NOT_USE:
    case OutServerTLS_DEPRECATED_DO_NOT_USE:
        break;
#endif
    case InServer: {
        return cg.readEntry("IncomingServer");
    }
    case InServerLogin: {
        return cg.readEntry("IncomingUserName");
    }
    case InServerPass: {
        return cg.readEntry("IncomingPassword");
    }
#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
    case InServerType: {
        return cg.readEntry("IncomingServerType");
    }
    case InServerMBXType: {
        return cg.readEntry("IncomingServerMBXType");
    }
    case InServerTLS: {
        return cg.readEntry("IncomingServerTLS", QVariant(false)).toString();
    }
#endif
    };
    return QString();
}
void KEMailSettings::setSetting(KEMailSettings::Setting s, const QString &v)
{
    KConfigGroup cg(p->m_pConfig, QLatin1String("PROFILE_") + p->m_sCurrentProfile);
    switch (s) {
    case ClientProgram: {
        cg.writePathEntry("EmailClient", v);
        break;
    }
    case ClientTerminal: {
        cg.writeEntry("TerminalClient", (v == QLatin1String("true")));
        break;
    }
    case RealName: {
        cg.writeEntry("FullName", v);
        break;
    }
    case EmailAddress: {
        cg.writeEntry("EmailAddress", v);
        break;
    }
    case ReplyToAddress: {
        cg.writeEntry("ReplyAddr", v);
        break;
    }
    case Organization: {
        cg.writeEntry("Organization", v);
        break;
    }
    case OutServer: {
        cg.writeEntry("OutgoingServer", v);
        break;
    }
    case OutServerLogin: {
        cg.writeEntry("OutgoingUserName", v);
        break;
    }
    case OutServerPass: {
        cg.writeEntry("OutgoingPassword", v);
        break;
    }
#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
    case OutServerType: {
        cg.writeEntry("OutgoingServerType", v);
        break;
    }
    case OutServerCommand: {
        cg.writeEntry("OutgoingCommand", v);
        break;
    }
    case OutServerTLS: {
        cg.writeEntry("OutgoingServerTLS", (v == QLatin1String("true")));
        break;
    }
#else
    case OutServerType_DEPRECATED_DO_NOT_USE:
    case OutServerCommand_DEPRECATED_DO_NOT_USE:
    case OutServerTLS_DEPRECATED_DO_NOT_USE:
        break;
#endif
    case InServer: {
        cg.writeEntry("IncomingServer", v);
        break;
    }
    case InServerLogin: {
        cg.writeEntry("IncomingUserName", v);
        break;
    }
    case InServerPass: {
        cg.writeEntry("IncomingPassword", v);
        break;
    }
#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
    case InServerType: {
        cg.writeEntry("IncomingServerType", v);
        break;
    }
    case InServerMBXType: {
        cg.writeEntry("IncomingServerMBXType", v);
        break;
    }
    case InServerTLS: {
        cg.writeEntry("IncomingServerTLS", (v == QLatin1String("true")));
        break;
    }
#endif
    };
    cg.sync();
}

void KEMailSettings::setDefault(const QString &s)
{
    p->m_pConfig->group("Defaults").writeEntry("Profile", s);
    p->m_pConfig->sync();
    p->m_sDefaultProfile = s;
}

void KEMailSettings::setProfile(const QString &s)
{
    const QString groupname = QLatin1String("PROFILE_") + s;
    p->m_sCurrentProfile = s;
    if (!p->m_pConfig->hasGroup(groupname)) { // Create a group if it doesn't exist
        KConfigGroup cg(p->m_pConfig, groupname);
        cg.writeEntry("ServerType", QString());
        p->profiles += s;
    }
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(5, 0)
QString KEMailSettings::currentProfileName() const
{
    return p->m_sCurrentProfile;
}
#endif

QStringList KEMailSettings::profiles() const
{
    return p->profiles;
}

KEMailSettings::KEMailSettings()
    : p(new KEMailSettingsPrivate())
{
    p->m_sCurrentProfile.clear();

    p->m_pConfig = new KConfig(QStringLiteral("emaildefaults"));

    const QStringList groups = p->m_pConfig->groupList();
    for (const auto &grp : groups) {
        if (grp.startsWith(QLatin1String("PROFILE_"))) {
            p->profiles += grp.mid(8 /* length of "PROFILE_" */);
        }
    }

    KConfigGroup cg(p->m_pConfig, "Defaults");
    p->m_sDefaultProfile = cg.readEntry("Profile", tr("Default"));
    if (!p->m_sDefaultProfile.isNull()) {
        if (!p->m_pConfig->hasGroup(QLatin1String("PROFILE_") + p->m_sDefaultProfile)) {
            setDefault(tr("Default"));
        } else {
            setDefault(p->m_sDefaultProfile);
        }
    } else {
        if (!p->profiles.isEmpty()) {
            setDefault(p->profiles[0]);
        } else {
            setDefault(tr("Default"));
        }
    }
    setProfile(defaultProfileName());
}

KEMailSettings::~KEMailSettings()
{
    delete p;
}
