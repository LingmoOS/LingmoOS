/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wireguardsetting.h"
#include "wireguardsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 16, 0)
#define NM_SETTING_WIREGUARD_SETTING_NAME "wireguard"

#define NM_SETTING_WIREGUARD_FWMARK "fwmark"
#define NM_SETTING_WIREGUARD_LISTEN_PORT "listen-port"
#define NM_SETTING_WIREGUARD_PRIVATE_KEY "private-key"
#define NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS "private-key-flags"
#define NM_SETTING_WIREGUARD_PEERS "peers"
#define NM_SETTING_WIREGUARD_MTU "mtu"
#define NM_SETTING_WIREGUARD_PEER_ROUTES "peer-routes"

#define NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY "preshared-key"
#define NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS "preshared-key-flags"
#define NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY "public-key"
#endif

NetworkManager::WireGuardSettingPrivate::WireGuardSettingPrivate()
    : name(NM_SETTING_WIREGUARD_SETTING_NAME)
    , fwmark(0)
    , listenPort(0)
    , mtu(0)
    , peerRoutes(true)
    , privateKeyFlags(NetworkManager::Setting::None)
{
}

NetworkManager::WireGuardSetting::WireGuardSetting()
    : Setting(Setting::WireGuard)
    , d_ptr(new WireGuardSettingPrivate())
{
}

NetworkManager::WireGuardSetting::WireGuardSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new WireGuardSettingPrivate())
{
    setFwmark(other->fwmark());
    setListenPort(other->listenPort());
    setMtu(other->mtu());
    setPeerRoutes(other->peerRoutes());
    setPeers(other->peers());
    setPrivateKey(other->privateKey());
    setPrivateKeyFlags(other->privateKeyFlags());
}

NetworkManager::WireGuardSetting::~WireGuardSetting()
{
    delete d_ptr;
}

QString NetworkManager::WireGuardSetting::name() const
{
    Q_D(const WireGuardSetting);

    return d->name;
}

quint32 NetworkManager::WireGuardSetting::fwmark() const
{
    Q_D(const WireGuardSetting);

    return d->fwmark;
}

void NetworkManager::WireGuardSetting::setFwmark(quint32 fwmark)
{
    Q_D(WireGuardSetting);

    d->fwmark = fwmark;
}

quint32 NetworkManager::WireGuardSetting::listenPort() const
{
    Q_D(const WireGuardSetting);

    return d->listenPort;
}

void NetworkManager::WireGuardSetting::setListenPort(quint32 port)
{
    Q_D(WireGuardSetting);

    d->listenPort = port;
}

quint32 NetworkManager::WireGuardSetting::mtu() const
{
    Q_D(const WireGuardSetting);

    return d->mtu;
}

void NetworkManager::WireGuardSetting::setMtu(quint32 mtu)
{
    Q_D(WireGuardSetting);

    d->mtu = mtu;
}

bool NetworkManager::WireGuardSetting::peerRoutes() const
{
    Q_D(const WireGuardSetting);

    return d->peerRoutes;
}

void NetworkManager::WireGuardSetting::setPeerRoutes(bool peerRoutes)
{
    Q_D(WireGuardSetting);

    d->peerRoutes = peerRoutes;
}

NMVariantMapList NetworkManager::WireGuardSetting::peers() const
{
    Q_D(const WireGuardSetting);

    return d->peers;
}

void NetworkManager::WireGuardSetting::setPeers(const NMVariantMapList &peers)
{
    Q_D(WireGuardSetting);

    d->peers = peers;
}

QString NetworkManager::WireGuardSetting::privateKey() const
{
    Q_D(const WireGuardSetting);

    return d->privateKey;
}

void NetworkManager::WireGuardSetting::setPrivateKey(const QString &key)
{
    Q_D(WireGuardSetting);

    d->privateKey = key;
}

NetworkManager::Setting::SecretFlags NetworkManager::WireGuardSetting::privateKeyFlags() const
{
    Q_D(const WireGuardSetting);

    return d->privateKeyFlags;
}

void NetworkManager::WireGuardSetting::setPrivateKeyFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(WireGuardSetting);

    d->privateKeyFlags = flags;
}

void NetworkManager::WireGuardSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY))) {
        setPrivateKey(secrets.value(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIREGUARD_PEERS))) {
        NMVariantMapList listOfPeers = qdbus_cast<NMVariantMapList>(secrets.value(QLatin1String(NM_SETTING_WIREGUARD_PEERS)));
        NMVariantMapList origPeers = peers();

        for (const QVariantMap &peer : listOfPeers) {
            if (peer.contains(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY))) {
                QString presharedKey = peer.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY)).toString();
                QString publicKey = peer.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)).toString();
                for (int i = 0; i < origPeers.size(); i++) {
                    if (origPeers[i][QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)].toString() == publicKey) {
                        origPeers[i].insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY), presharedKey);
                    }
                }
            }
        }
        setPeers(origPeers);
    }
}

QVariantMap NetworkManager::WireGuardSetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!privateKey().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY), privateKey());
    }

    NMVariantMapList peersSecrets;

    for (const QVariantMap &map : peers()) {
        if (map.contains(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)) && map.contains(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY))) {
            QVariantMap newMap;
            newMap.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY), map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)));
            newMap.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY), map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY)));

            peersSecrets << newMap;
        }
    }

    if (!peersSecrets.isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIREGUARD_PEERS), QVariant::fromValue(peersSecrets));
    }

    return secrets;
}

void NetworkManager::WireGuardSetting::secretsFromStringMap(const NMStringMap &map)
{
    QVariantMap secretsMap;
    NMVariantMapList peers;

    auto it = map.constBegin();
    while (it != map.constEnd()) {
        if (it.key() == QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY)) {
            secretsMap.insert(it.key(), it.value());
        }

        if (it.key().startsWith(QLatin1String(NM_SETTING_WIREGUARD_PEERS)) && it.key().endsWith(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY))) {
            QStringList peerStrList = it.key().split(QLatin1Char('.'));

            QVariantMap peer;
            peer.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY), peerStrList.at(1));
            peer.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY), it.value());

            peers << peer;
        }
        ++it;
    }

    if (!peers.isEmpty()) {
        secretsMap.insert(QLatin1String(NM_SETTING_WIREGUARD_PEERS), QVariant::fromValue(peers));
    }

    secretsFromMap(secretsMap);
}

NMStringMap NetworkManager::WireGuardSetting::secretsToStringMap() const
{
    NMStringMap ret;
    QVariantMap secretsMap = secretsToMap();

    auto it = secretsMap.constBegin();
    while (it != secretsMap.constEnd()) {
        if (it.key() == QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY)) {
            ret.insert(it.key(), it.value().toString());
        }

        if (it.key() == QLatin1String(NM_SETTING_WIREGUARD_PEERS)) {
            NMVariantMapList listOfPeers = qdbus_cast<NMVariantMapList>(it.value());

            for (const QVariantMap &map : listOfPeers) {
                const QString str = QStringLiteral("%1.%2.%3")
                                        .arg(QLatin1String(NM_SETTING_WIREGUARD_PEERS))
                                        .arg(map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)).toString())
                                        .arg(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY));
                ret.insert(str, map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY)).toString());
            }
        }
        ++it;
    }

    return ret;
}

QStringList NetworkManager::WireGuardSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if (!privateKeyFlags().testFlag(Setting::NotRequired)) {
        if (privateKey().isEmpty() || requestNew) {
            secrets << QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY);
        }
    }

    for (const QVariantMap &map : peers()) {
        const QString presharedKey = map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY)).toString();
        SecretFlags preSharedKeyFlags = (SecretFlags)map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS)).toInt();

        if (!presharedKey.isEmpty()) {
            continue;
        }

        if (preSharedKeyFlags.testFlag(Setting::NotRequired)) {
            continue;
        }

        const QString str = QStringLiteral("%1.%2.%3")
                                .arg(QLatin1String(NM_SETTING_WIREGUARD_PEERS))
                                .arg(map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY)).toString())
                                .arg(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY));
        secrets << str;
    }

    return secrets;
}

void NetworkManager::WireGuardSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_FWMARK))) {
        setFwmark(setting.value(QLatin1String(NM_SETTING_WIREGUARD_FWMARK)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_LISTEN_PORT))) {
        setListenPort(setting.value(QLatin1String(NM_SETTING_WIREGUARD_LISTEN_PORT)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_MTU))) {
        setMtu(setting.value(QLatin1String(NM_SETTING_WIREGUARD_MTU)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_PEER_ROUTES))) {
        setPeerRoutes(setting.value(QLatin1String(NM_SETTING_WIREGUARD_PEER_ROUTES)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_PEERS))) {
        setPeers(qdbus_cast<NMVariantMapList>(setting.value(QLatin1String(NM_SETTING_WIREGUARD_PEERS))));
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY))) {
        setPrivateKey(setting.value(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS))) {
        setPrivateKeyFlags((SecretFlags)setting.value(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS)).toInt());
    }
}

QVariantMap NetworkManager::WireGuardSetting::toMap() const
{
    QVariantMap setting;

    setting.insert(QLatin1String(NM_SETTING_WIREGUARD_FWMARK), fwmark());
    setting.insert(QLatin1String(NM_SETTING_WIREGUARD_LISTEN_PORT), listenPort());
    setting.insert(QLatin1String(NM_SETTING_WIREGUARD_MTU), mtu());
    setting.insert(QLatin1String(NM_SETTING_WIREGUARD_PEER_ROUTES), peerRoutes());

    if (!peers().isEmpty()) {
        // FIXME we seem to have SecretFlags as an int, but NM expects an uint, while this is not
        // problem for rest of *-flags properties, it's problem for "preshared-key" which NM handless
        // as GVariant and asks for "u" when getting it's value
        NMVariantMapList fixedPeers = peers();
        for (QVariantMap &map : fixedPeers) {
            if (map.contains(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS))) {
                map.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS),
                           map.value(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS)).toUInt());
            }
        }

        setting.insert(QLatin1String(NM_SETTING_WIREGUARD_PEERS), QVariant::fromValue(fixedPeers));
    }

    if (!privateKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY), privateKey());
    }
    setting.insert(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS), (int)privateKeyFlags());

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::WireGuardSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_WIREGUARD_FWMARK << ": " << setting.fwmark() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_LISTEN_PORT << ": " << setting.listenPort() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_MTU << ": " << setting.mtu() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_PEER_ROUTES << ": " << setting.peerRoutes() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_PEERS << ": " << setting.peers() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_PRIVATE_KEY << ": " << setting.privateKey() << '\n';
    dbg.nospace() << NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS << ": " << setting.privateKeyFlags() << '\n';

    return dbg.maybeSpace();
}
