/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connectionwidget.h"
#include "advancedpermissionswidget.h"
#include "ui_connectionwidget.h"

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Settings>

#include <KAcceleratorManager>
#include <KLocalizedString>
#include <KUser>
#include <QDBusConnection>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

ConnectionWidget::ConnectionWidget(const NetworkManager::ConnectionSettings::Ptr &settings, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_widget(new Ui::ConnectionWidget)
    , m_type(settings->connectionType())
    , m_masterUuid(settings->master())
    , m_slaveType(settings->slaveType())
{
    m_widget->setupUi(this);

    m_widget->firewallZone->addItems(firewallZones());

    // VPN combo
    populateVpnConnections();
    if (settings->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
        m_widget->autoconnectVpn->setEnabled(false);
        m_widget->vpnCombobox->setEnabled(false);
        m_widget->autoconnect->setEnabled(false);
    } else {
        m_widget->autoconnectVpn->setEnabled(true);
        m_widget->autoconnect->setEnabled(true);
    }

    connect(m_widget->autoconnectVpn, &QCheckBox::toggled, this, &ConnectionWidget::autoVpnToggled);

    if (settings) {
        loadConfig(settings);
    }

    m_tmpSetting.setPermissions(settings->permissions());

    KAcceleratorManager::manage(this);

    connect(m_widget->autoconnect, &QCheckBox::stateChanged, this, &ConnectionWidget::settingChanged);
    connect(m_widget->allUsers, &QCheckBox::stateChanged, this, &ConnectionWidget::settingChanged);
    connect(m_widget->allUsers, &QCheckBox::stateChanged, this, &ConnectionWidget::allUsersChanged);
    connect(m_widget->autoconnectVpn, &QCheckBox::stateChanged, this, &ConnectionWidget::settingChanged);
    connect(m_widget->pushButtonPermissions, &QPushButton::clicked, this, &ConnectionWidget::settingChanged);
    connect(m_widget->firewallZone, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionWidget::settingChanged);
    connect(m_widget->firewallZone, &QComboBox::currentTextChanged, this, &ConnectionWidget::settingChanged);
    connect(m_widget->vpnCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionWidget::settingChanged);
    connect(m_widget->vpnCombobox, &QComboBox::currentTextChanged, this, &ConnectionWidget::settingChanged);
    connect(m_widget->prioritySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConnectionWidget::settingChanged);
    connect(m_widget->metered, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionWidget::settingChanged);

    connect(m_widget->pushButtonPermissions, &QPushButton::clicked, this, &ConnectionWidget::openAdvancedPermissions);
}

ConnectionWidget::~ConnectionWidget()
{
    delete m_widget;
}

void ConnectionWidget::loadConfig(const NetworkManager::ConnectionSettings::Ptr &settings)
{
    if (settings->permissions().isEmpty()) {
        m_widget->allUsers->setChecked(true);
    } else {
        m_widget->allUsers->setChecked(false);
    }

    const QString zone = settings->zone();
    m_widget->firewallZone->setCurrentIndex(m_widget->firewallZone->findText(zone));

    const QStringList secondaries = settings->secondaries();
    const QStringList vpnKeys = vpnConnections().keys();
    if (!secondaries.isEmpty() && !vpnKeys.isEmpty()) {
        for (const QString &vpnKey : vpnKeys) {
            if (secondaries.contains(vpnKey)) {
                m_widget->vpnCombobox->setCurrentIndex(m_widget->vpnCombobox->findData(vpnKey));
                m_widget->autoconnectVpn->setChecked(true);
                break;
            }
        }
    } else {
        m_widget->autoconnectVpn->setChecked(false);
    }

    m_widget->autoconnect->setChecked(settings->autoconnect());

    if (m_widget->prioritySpin->isEnabled()) {
        m_widget->prioritySpin->setValue(settings->autoconnectPriority());
    }

    m_widget->metered->setCurrentIndex(settings->metered());
}

NMVariantMapMap ConnectionWidget::setting() const
{
    NetworkManager::ConnectionSettings settings;

    settings.setConnectionType(m_type);
    settings.setAutoconnect(m_widget->autoconnect->isChecked());
    settings.setMaster(m_masterUuid);
    settings.setSlaveType(m_slaveType);

    if (m_widget->allUsers->isChecked()) {
        settings.setPermissions(QHash<QString, QString>());
    } else {
        if (m_tmpSetting.permissions().isEmpty()) {
            settings.addToPermissions(KUser().loginName(), QString());
        } else {
            settings.setPermissions(m_tmpSetting.permissions());
        }
    }

    if (m_widget->autoconnectVpn->isChecked() && m_widget->vpnCombobox->count() > 0) {
        settings.setSecondaries(QStringList() << m_widget->vpnCombobox->itemData(m_widget->vpnCombobox->currentIndex()).toString());
    }

    const QString zone = m_widget->firewallZone->currentText();
    if (!zone.isEmpty()) {
        settings.setZone(zone);
    }

    if (m_widget->prioritySpin->isEnabled()) {
        settings.setAutoconnectPriority(m_widget->prioritySpin->value());
    }

    settings.setMetered(static_cast<NetworkManager::ConnectionSettings::Metered>(m_widget->metered->currentIndex()));

    return settings.toMap();
}

bool ConnectionWidget::allUsers() const
{
    return m_widget->allUsers->isChecked();
}

void ConnectionWidget::autoVpnToggled(bool on)
{
    m_widget->vpnCombobox->setEnabled(on);
}

void ConnectionWidget::openAdvancedPermissions()
{
    QPointer<AdvancedPermissionsWidget> dialog = new AdvancedPermissionsWidget(m_tmpSetting.permissions(), this);
    dialog->setWindowTitle(i18nc("@title:window advanced permissions editor", "Advanced Permissions Editor"));
    if (dialog->exec() == QDialog::Accepted) {
        m_tmpSetting.setPermissions(dialog->currentUsers());
    }
    delete dialog;
}

NMStringMap ConnectionWidget::vpnConnections() const
{
    NetworkManager::Connection::List list = NetworkManager::listConnections();
    NMStringMap result;

    for (const NetworkManager::Connection::Ptr &conn : list) {
        NetworkManager::ConnectionSettings::Ptr conSet = conn->settings();
        if (conSet->connectionType() == NetworkManager::ConnectionSettings::Vpn //
            || conSet->connectionType() == NetworkManager::ConnectionSettings::WireGuard) {
            // qCDebug(PLASMA_NM_EDITOR_LOG) << "Found VPN" << conSet->id() << conSet->uuid();
            result.insert(conSet->uuid(), conSet->id());
        }
    }

    return result;
}

QStringList ConnectionWidget::firewallZones() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.fedoraproject.FirewallD1"), //
                                                      QStringLiteral("/org/fedoraproject/FirewallD1"),
                                                      QStringLiteral("org.fedoraproject.FirewallD1.zone"),
                                                      QStringLiteral("getZones"));
    QDBusPendingReply<QStringList> reply = QDBusConnection::systemBus().asyncCall(msg);
    reply.waitForFinished();
    if (reply.isValid())
        return reply.value();

    return {};
}

void ConnectionWidget::populateVpnConnections()
{
    QMapIterator<QString, QString> it(vpnConnections());
    while (it.hasNext()) {
        it.next();
        m_widget->vpnCombobox->addItem(it.value(), it.key());
    }
}

#include "moc_connectionwidget.cpp"
