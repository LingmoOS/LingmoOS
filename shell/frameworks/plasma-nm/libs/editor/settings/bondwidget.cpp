/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bondwidget.h"
#include "connectioneditordialog.h"
#include "plasma_nm_editor.h"
#include "ui_bond.h"

#include <QDBusPendingReply>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/Settings>

#include <KLocalizedString>
#include <KMessageBox>
#include <kwidgetsaddons_version.h>

#define NM_SETTING_BOND_OPTION_MII_MONITOR "mii"
#define NM_SETTING_BOND_OPTION_ARP_MONITOR "arp"

BondWidget::BondWidget(const QString &masterUuid, const QString &masterId, const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_uuid(masterUuid)
    , m_id(masterId)
    , m_ui(new Ui::BondWidget)
    , m_menu(new QMenu(this))
{
    m_ui->setupUi(this);

    // Action buttons and menu
    auto action = new QAction(i18n("Ethernet"), this);
    action->setData(NetworkManager::ConnectionSettings::Wired);
    m_menu->addAction(action);
    action = new QAction(i18n("InfiniBand"), this);
    action->setData(NetworkManager::ConnectionSettings::Infiniband);
    m_menu->addAction(action);
    m_ui->btnAdd->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &BondWidget::addBond);
    connect(m_ui->btnEdit, &QPushButton::clicked, this, &BondWidget::editBond);
    connect(m_ui->btnDelete, &QPushButton::clicked, this, &BondWidget::deleteBond);

    // mode
    m_ui->mode->addItem(i18nc("bond mode", "Round-robin"), QLatin1String("balance-rr"));
    m_ui->mode->addItem(i18nc("bond mode", "Active backup"), QLatin1String("active-backup"));
    m_ui->mode->addItem(i18nc("bond mode", "Broadcast"), QLatin1String("broadcast"));
    m_ui->mode->addItem(i18nc("bond mode", "802.3ad"), QLatin1String("802.3ad"));
    m_ui->mode->addItem(i18nc("bond mode", "Adaptive transmit load balancing"), QLatin1String("balance-tlb"));
    m_ui->mode->addItem(i18nc("bond mode", "Adaptive load balancing"), QLatin1String("balance-alb"));

    // link monitor
    m_ui->linkMonitoring->addItem(i18nc("bond link monitoring", "MII (recommended)"), NM_SETTING_BOND_OPTION_MII_MONITOR);
    m_ui->linkMonitoring->addItem(i18nc("bond link monitoring", "ARP"), NM_SETTING_BOND_OPTION_ARP_MONITOR);

    // bonds
    populateBonds();
    connect(m_ui->bonds, &QListWidget::currentItemChanged, this, &BondWidget::currentBondChanged);
    connect(m_ui->bonds, &QListWidget::itemDoubleClicked, this, &BondWidget::editBond);

    connect(m_ui->ifaceName, &KLineEdit::textChanged, this, &BondWidget::slotWidgetChanged);
    connect(m_ui->arpTargets, &KLineEdit::textChanged, this, &BondWidget::slotWidgetChanged);
    connect(m_ui->linkMonitoring, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &BondWidget::slotWidgetChanged);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);
    KAcceleratorManager::manage(m_menu);

    if (setting) {
        loadConfig(setting);
    }
}

BondWidget::~BondWidget()
{
    delete m_ui;
}

void BondWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::BondSetting::Ptr bondSetting = setting.staticCast<NetworkManager::BondSetting>();

    m_ui->ifaceName->setText(bondSetting->interfaceName());

    const NMStringMap options = bondSetting->options();

    // mode
    int modeIndex = m_ui->mode->findData(options.value(NM_SETTING_BOND_OPTION_MODE));
    if (modeIndex == -1)
        modeIndex = 0;
    m_ui->mode->setCurrentIndex(modeIndex);

    const QString arpTargets = options.value(NM_SETTING_BOND_OPTION_ARP_IP_TARGET);
    if (!arpTargets.isEmpty()) { // ARP
        m_ui->linkMonitoring->setCurrentIndex(m_ui->linkMonitoring->findData(NM_SETTING_BOND_OPTION_ARP_MONITOR));

        bool ok = false;
        const int arpMonFreq = options.value(NM_SETTING_BOND_OPTION_ARP_INTERVAL).toInt(&ok);
        if (ok && arpMonFreq > 0)
            m_ui->monitorFreq->setValue(arpMonFreq);

        m_ui->arpTargets->setText(arpTargets);
    } else { // MII
        m_ui->linkMonitoring->setCurrentIndex(m_ui->linkMonitoring->findData(NM_SETTING_BOND_OPTION_MII_MONITOR));

        bool ok = false;
        const int miiMonFreq = options.value(NM_SETTING_BOND_OPTION_MIIMON).toInt(&ok);
        if (ok && miiMonFreq > 0)
            m_ui->monitorFreq->setValue(miiMonFreq);

        ok = false;
        const int upDelay = options.value(NM_SETTING_BOND_OPTION_UPDELAY).toInt(&ok);
        if (ok && upDelay > 0)
            m_ui->upDelay->setValue(upDelay);

        ok = false;
        const int downDelay = options.value(NM_SETTING_BOND_OPTION_DOWNDELAY).toInt(&ok);
        if (ok && downDelay > 0)
            m_ui->upDelay->setValue(downDelay);
    }
}

QVariantMap BondWidget::setting() const
{
    NetworkManager::BondSetting setting;
    setting.setInterfaceName(m_ui->ifaceName->text());

    NMStringMap options;
    options.insert(NM_SETTING_BOND_OPTION_MODE, m_ui->mode->itemData(m_ui->mode->currentIndex()).toString());

    if (m_ui->linkMonitoring->itemData(m_ui->linkMonitoring->currentIndex()).toString() == NM_SETTING_BOND_OPTION_MII_MONITOR) { // MII
        options.insert(NM_SETTING_BOND_OPTION_MIIMON, QString::number(m_ui->monitorFreq->value()));
        const int upDelay = m_ui->upDelay->value();
        if (upDelay)
            options.insert(NM_SETTING_BOND_OPTION_UPDELAY, QString::number(upDelay));
        const int downDelay = m_ui->downDelay->value();
        if (downDelay)
            options.insert(NM_SETTING_BOND_OPTION_DOWNDELAY, QString::number(downDelay));
    } else { // ARP
        options.insert(NM_SETTING_BOND_OPTION_ARP_INTERVAL, QString::number(m_ui->monitorFreq->value()));
        const QString arpTargets = m_ui->arpTargets->text();
        if (!arpTargets.isEmpty())
            options.insert(NM_SETTING_BOND_OPTION_ARP_IP_TARGET, arpTargets);
    }

    setting.setOptions(options);
    return setting.toMap();
}

void BondWidget::addBond(QAction *action)
{
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Adding bonded connection:" << action->data();
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Master UUID:" << m_uuid;
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Slave type:" << type();

    NetworkManager::ConnectionSettings::ConnectionType connectionType = static_cast<NetworkManager::ConnectionSettings::ConnectionType>(action->data().toInt());
    NetworkManager::ConnectionSettings::Ptr connectionSettings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(connectionType));
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    connectionSettings->setMaster(m_uuid);
    connectionSettings->setSlaveType(type());
    connectionSettings->setAutoconnect(false);

    QPointer<ConnectionEditorDialog> bondEditor = new ConnectionEditorDialog(connectionSettings);
    bondEditor->setAttribute(Qt::WA_DeleteOnClose);
    connect(bondEditor.data(), &ConnectionEditorDialog::accepted, [bondEditor, this]() {
        qCDebug(PLASMA_NM_EDITOR_LOG) << "Saving slave connection";
        // qCDebug(PLASMA_NM_EDITOR_LOG) << bondEditor->setting();
        QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(bondEditor->setting());
        auto watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &BondWidget::bondAddComplete);
    });
    bondEditor->setModal(true);
    bondEditor->show();
}

void BondWidget::currentBondChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    m_ui->btnEdit->setEnabled(current);
    m_ui->btnDelete->setEnabled(current);
}

void BondWidget::bondAddComplete(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;

    if (reply.isValid()) {
        // find the slave connection with matching UUID
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(reply.value().path());
        if (connection && connection->settings()->master() == m_uuid) {
            const QString label =
                QStringLiteral("%1 (%2)").arg(connection->name(), connection->settings()->typeAsString(connection->settings()->connectionType()));
            auto slaveItem = new QListWidgetItem(label, m_ui->bonds);
            slaveItem->setData(Qt::UserRole, connection->uuid());
            slotWidgetChanged();
        }
    } else {
        qCWarning(PLASMA_NM_EDITOR_LOG) << "Bonded connection not added:" << reply.error().message();
    }
}

void BondWidget::editBond()
{
    QListWidgetItem *currentItem = m_ui->bonds->currentItem();
    if (!currentItem)
        return;

    const QString uuid = currentItem->data(Qt::UserRole).toString();
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(uuid);

    if (connection) {
        // qCDebug(PLASMA_NM_EDITOR_LOG) << "Editing bonded connection" << currentItem->text() << uuid;
        QPointer<ConnectionEditorDialog> bondEditor = new ConnectionEditorDialog(connection->settings());
        bondEditor->setAttribute(Qt::WA_DeleteOnClose);
        connect(bondEditor.data(), &ConnectionEditorDialog::accepted, [connection, bondEditor, this]() {
            connection->update(bondEditor->setting());
            connect(connection.data(), &NetworkManager::Connection::updated, this, &BondWidget::populateBonds);
        });
        bondEditor->setModal(true);
        bondEditor->show();
    }
}

void BondWidget::deleteBond()
{
    QListWidgetItem *currentItem = m_ui->bonds->currentItem();
    if (!currentItem)
        return;

    const QString uuid = currentItem->data(Qt::UserRole).toString();
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(uuid);

    if (connection) {
        // qCDebug(PLASMA_NM_EDITOR_LOG) << "About to delete bonded connection" << currentItem->text() << uuid;
        if (KMessageBox::questionTwoActions(this,
                                            i18n("Do you want to remove the connection '%1'?", connection->name()),
                                            i18n("Remove Connection"),
                                            KStandardGuiItem::remove(),
                                            KStandardGuiItem::cancel(),
                                            QString(),
                                            KMessageBox::Dangerous)
            == KMessageBox::ButtonCode::PrimaryAction) {
            connection->remove();
            delete currentItem;
            slotWidgetChanged();
        }
    }
}

void BondWidget::populateBonds()
{
    m_ui->bonds->clear();

    for (const NetworkManager::Connection::Ptr &connection : NetworkManager::listConnections()) {
        NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
        // The mapping from slave to master may be by uuid or name, try our best to
        // figure out if we are master to the slave.
        const QString master = settings->master();
        bool isSlave = ((master == m_uuid) || // by-uuid
                        (!m_id.isEmpty() && master == m_id)); // by-name
        if (isSlave && (settings->slaveType() == type())) {
            const QString label =
                QStringLiteral("%1 (%2)").arg(connection->name(), connection->settings()->typeAsString(connection->settings()->connectionType()));
            auto slaveItem = new QListWidgetItem(label, m_ui->bonds);
            slaveItem->setData(Qt::UserRole, connection->uuid());
        }
    }
}

bool BondWidget::isValid() const
{
    if (m_ui->linkMonitoring->itemData(m_ui->linkMonitoring->currentIndex()).toString() == NM_SETTING_BOND_OPTION_ARP_MONITOR) {
        const QStringList ipAddresses = m_ui->arpTargets->text().split(QLatin1Char(','));
        if (ipAddresses.isEmpty()) {
            return false;
        }

        for (const QString &ip : ipAddresses) {
            QHostAddress ipAddress(ip);
            if (ipAddress.isNull()) {
                return false;
            }
        }
    }

    return !m_ui->ifaceName->text().isEmpty() && m_ui->bonds->count() > 0;
}

#include "moc_bondwidget.cpp"
