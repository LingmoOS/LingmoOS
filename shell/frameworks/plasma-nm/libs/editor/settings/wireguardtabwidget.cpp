/*
    SPDX-FileCopyrightText: 2019 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "wireguardtabwidget.h"
#include "ui_wireguardpeerwidget.h"
#include "ui_wireguardtabwidget.h"
#include "wireguardpeerwidget.h"

#include <QStandardItemModel>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/Utils>

// Keys for the NetworkManager configuration
#define PNM_SETTING_WIREGUARD_SETTING_NAME "wireguard"

#define PNM_WG_KEY_PEERS "peers"
#define PNM_WG_KEY_MTU "mtu"
#define PNM_WG_KEY_PEER_ROUTES "peer-routes"
#define PNM_WG_PEER_KEY_ALLOWED_IPS "allowed-ips"
#define PNM_WG_PEER_KEY_ENDPOINT "endpoint"
#define PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE "persistent-keepalive"
#define PNM_WG_PEER_KEY_PRESHARED_KEY "preshared-key"
#define PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS "preshared-key-flags"
#define PNM_WG_PEER_KEY_PUBLIC_KEY "public-key"

class WireGuardTabWidget::Private
{
public:
    Private();
    ~Private();

    Ui_WireGuardTabWidget ui;
    NetworkManager::WireGuardSetting::Ptr setting;
    KSharedConfigPtr config;
    NMVariantMapList peers;
    int currentIndex;
    bool currentPeerValid;
    bool otherPeersValid;
};

WireGuardTabWidget::Private::Private() = default;

WireGuardTabWidget::Private::~Private() = default;

WireGuardTabWidget::WireGuardTabWidget(const NMVariantMapList &peerData, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , d(new Private)
{
    d->ui.setupUi(this);

    d->config = KSharedConfig::openConfig();
    setWindowTitle(i18nc("@title: window wireguard peers properties", "WireGuard peers properties"));
    connect(d->ui.btnAdd, &QPushButton::clicked, this, &WireGuardTabWidget::slotAddPeer);
    connect(d->ui.btnRemove, &QPushButton::clicked, this, &WireGuardTabWidget::slotRemovePeer);
    connect(d->ui.buttonBox, &QDialogButtonBox::accepted, this, &WireGuardTabWidget::accept);
    connect(d->ui.buttonBox, &QDialogButtonBox::rejected, this, &WireGuardTabWidget::reject);

    KAcceleratorManager::manage(this);

    loadConfig(peerData);

    if (peerData.isEmpty())
        slotAddPeer();
}

WireGuardTabWidget::~WireGuardTabWidget()
{
    delete d;
}

void WireGuardTabWidget::loadConfig(const NMVariantMapList &peerData)
{
    d->peers = peerData;
    int numIncomingPeers = d->peers.size();

    // If there weren't any peers in the incoming setting, create
    // the required first element
    if (d->peers.isEmpty())
        d->peers.append(*(new QVariantMap()));

    for (int i = 0; i < numIncomingPeers; i++) {
        slotAddPeerWithData(peerData[i]);
    }
    d->ui.tabWidget->setCurrentIndex(0);
}

NMVariantMapList WireGuardTabWidget::setting() const
{
    d->peers.clear();
    for (int i = 0; i < d->ui.tabWidget->count(); i++)
        d->peers.append(static_cast<WireGuardPeerWidget *>(d->ui.tabWidget->widget(i))->setting());
    return d->peers;
}

void WireGuardTabWidget::slotAddPeer()
{
    auto newItem = new QVariantMap;
    int numPeers = d->ui.tabWidget->count() + 1;
    auto newTab = new WireGuardPeerWidget(*newItem);
    connect(newTab, &WireGuardPeerWidget::notifyValid, this, &WireGuardTabWidget::slotWidgetChanged);
    d->ui.tabWidget->addTab(newTab, QString("Peer %1").arg(QString::number(numPeers)));
    d->peers.append(*newItem);
    d->ui.tabWidget->setCurrentIndex(numPeers - 1);
    slotWidgetChanged();
}

void WireGuardTabWidget::slotAddPeerWithData(const QVariantMap &peerData)
{
    int numPeers = d->ui.tabWidget->count() + 1;
    auto newTab = new WireGuardPeerWidget(peerData);
    d->ui.tabWidget->addTab(newTab, QString("Peer %1").arg(QString::number(numPeers)));
    connect(newTab, &WireGuardPeerWidget::notifyValid, this, &WireGuardTabWidget::slotWidgetChanged);
    d->peers.append(peerData);
    d->ui.tabWidget->setCurrentIndex(numPeers - 1);
    slotWidgetChanged();
}

void WireGuardTabWidget::slotRemovePeer()
{
    int numPeers = d->ui.tabWidget->count() - 1;
    d->ui.tabWidget->removeTab(d->ui.tabWidget->currentIndex());
    if (numPeers == 0) {
        slotAddPeer();
        numPeers = 1;
    }

    // Retitle the tabs to reflect the current numbers
    for (int i = 0; i < numPeers; i++)
        d->ui.tabWidget->setTabText(i, QString("Peer %1").arg(QString::number(i + 1)));
}

void WireGuardTabWidget::slotWidgetChanged()
{
    bool valid = true;
    for (int i = 0; i < d->ui.tabWidget->count(); i++) {
        if (!static_cast<WireGuardPeerWidget *>(d->ui.tabWidget->widget(i))->isValid()) {
            valid = false;
            break;
        }
    }
    d->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

#include "moc_wireguardtabwidget.cpp"
