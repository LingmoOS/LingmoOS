// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "kcm.h"

#include <KAboutData>
#include <KJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "core/ipvalidator.h"
#include "core/loglistmodel.h"
#include "core/rulelistmodel.h"
#include "version.h"

#include "backends/netstat/connectionsmodel.h"
#include "backends/netstat/netstatclient.h"

K_PLUGIN_CLASS_WITH_JSON(KCMFirewall, "kcm_firewall.json")

KCMFirewall::KCMFirewall(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData)
    , m_client(new FirewallClient(this))
{
    qmlRegisterAnonymousType<KJob>("org.kcm.firewall", 1);
    qmlRegisterType<RuleListModel>("org.kcm.firewall", 1, 0, "RuleListModel");
    qmlRegisterType<Rule>("org.kcm.firewall", 1, 0, "Rule");
    qmlRegisterUncreatableType<FirewallClient>("org.kcm.firewall", 1, 0, "FirewallClient", "FirewallClient is created by the KCM.");
    qmlRegisterUncreatableType<LogListModel>("org.kcm.firewall", 1, 0, "LogListModel", "Only created from the UfwClient.");
    qmlRegisterType<NetstatClient>("org.kcm.firewall", 1, 0, "NetstatClient");
    qmlRegisterUncreatableType<ConnectionsModel>("org.kcm.firewall", 1, 0, "ConnectionsModel", "Use the NetstatClient");
    qmlRegisterType<IPValidator>("org.kcm.firewall", 1, 0, "IPValidator");

    // TODO: Make this configurable.
    m_client->setBackend({"ufw", "firewalld"});

    if (m_client->capabilities() & FirewallClient::SaveCapability) {
        setButtons(Help | Apply);
    } else {
        setButtons(Help);
    }
}

KCMFirewall::~KCMFirewall()
{
}

void KCMFirewall::save()
{
    m_client->save();
}

FirewallClient *KCMFirewall::client() const
{
    return m_client;
}

#include "kcm.moc"
