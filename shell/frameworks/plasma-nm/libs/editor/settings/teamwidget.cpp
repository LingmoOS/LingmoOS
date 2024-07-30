/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "teamwidget.h"
#include "connectioneditordialog.h"
#include "plasma_nm_editor.h"
#include "ui_team.h"

#include <QDBusPendingReply>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStandardPaths>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/Settings>

#include <KLocalizedString>
#include <KMessageBox>
#include <kwidgetsaddons_version.h>

TeamWidget::TeamWidget(const QString &masterUuid, const QString &masterId, const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_uuid(masterUuid)
    , m_id(masterId)
    , m_ui(new Ui::TeamWidget)
    , m_menu(new QMenu(this))
{
    m_ui->setupUi(this);

    // Action buttons and menu
    auto action = new QAction(i18n("Ethernet"), this);
    action->setData(NetworkManager::ConnectionSettings::Wired);
    m_menu->addAction(action);
    action = new QAction(i18n("Infiniband"), this);
    action->setData(NetworkManager::ConnectionSettings::Infiniband);
    m_menu->addAction(action);
    action = new QAction(i18n("Wi-Fi"), this);
    action->setData(NetworkManager::ConnectionSettings::Wireless);
    m_menu->addAction(action);
    action = new QAction(i18n("VLAN"), this);
    action->setData(NetworkManager::ConnectionSettings::Vlan);
    m_menu->addAction(action);
    m_ui->btnAdd->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &TeamWidget::addTeam);
    connect(m_ui->btnEdit, &QPushButton::clicked, this, &TeamWidget::editTeam);
    connect(m_ui->btnDelete, &QPushButton::clicked, this, &TeamWidget::deleteTeam);

    connect(m_ui->btnImport, &QPushButton::clicked, this, &TeamWidget::importConfig);

    // teams
    populateTeams();
    connect(m_ui->teams, &QListWidget::currentItemChanged, this, &TeamWidget::currentTeamChanged);
    connect(m_ui->teams, &QListWidget::itemDoubleClicked, this, &TeamWidget::editTeam);

    connect(m_ui->ifaceName, &KLineEdit::textChanged, this, &TeamWidget::slotWidgetChanged);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);
    KAcceleratorManager::manage(m_menu);

    if (setting) {
        loadConfig(setting);
    }
}

TeamWidget::~TeamWidget()
{
    delete m_ui;
}

void TeamWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::TeamSetting::Ptr teamSetting = setting.staticCast<NetworkManager::TeamSetting>();

    m_ui->ifaceName->setText(teamSetting->interfaceName());
    m_ui->config->setPlainText(teamSetting->config());
}

QVariantMap TeamWidget::setting() const
{
    NetworkManager::TeamSetting setting;
    setting.setInterfaceName(m_ui->ifaceName->text());
    setting.setConfig(m_ui->config->toPlainText());

    return setting.toMap();
}

void TeamWidget::addTeam(QAction *action)
{
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Adding teamed connection:" << action->data();
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Master UUID:" << m_uuid;
    qCDebug(PLASMA_NM_EDITOR_LOG) << "Slave type:" << type();

    NetworkManager::ConnectionSettings::ConnectionType connectionType = static_cast<NetworkManager::ConnectionSettings::ConnectionType>(action->data().toInt());
    NetworkManager::ConnectionSettings::Ptr connectionSettings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(connectionType));
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    connectionSettings->setMaster(m_uuid);
    connectionSettings->setSlaveType(type());
    connectionSettings->setAutoconnect(false);

    QPointer<ConnectionEditorDialog> teamEditor = new ConnectionEditorDialog(connectionSettings);
    teamEditor->setAttribute(Qt::WA_DeleteOnClose);
    connect(teamEditor.data(), &ConnectionEditorDialog::accepted, [teamEditor, this]() {
        qCDebug(PLASMA_NM_EDITOR_LOG) << "Saving slave connection";
        // qCDebug(PLASMA_NM_EDITOR_LOG) << teamEditor->setting();
        QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(teamEditor->setting());
        auto watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &TeamWidget::teamAddComplete);
    });
    teamEditor->setModal(true);
    teamEditor->show();
}

void TeamWidget::currentTeamChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    m_ui->btnEdit->setEnabled(current);
    m_ui->btnDelete->setEnabled(current);
}

void TeamWidget::teamAddComplete(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;

    if (reply.isValid()) {
        // find the slave connection with matching UUID
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(reply.value().path());
        if (connection && connection->settings()->master() == m_uuid) {
            const QString label = QString("%1 (%2)").arg(connection->name(), connection->settings()->typeAsString(connection->settings()->connectionType()));
            auto slaveItem = new QListWidgetItem(label, m_ui->teams);
            slaveItem->setData(Qt::UserRole, connection->uuid());
            slotWidgetChanged();
        }
    } else {
        qCWarning(PLASMA_NM_EDITOR_LOG) << "Teamed connection not added:" << reply.error().message();
    }
}

void TeamWidget::editTeam()
{
    QListWidgetItem *currentItem = m_ui->teams->currentItem();
    if (!currentItem)
        return;

    const QString uuid = currentItem->data(Qt::UserRole).toString();
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(uuid);

    if (connection) {
        qCDebug(PLASMA_NM_EDITOR_LOG) << "Editing teamed connection" << currentItem->text() << uuid;
        QPointer<ConnectionEditorDialog> teamEditor = new ConnectionEditorDialog(connection->settings());
        teamEditor->setAttribute(Qt::WA_DeleteOnClose);
        connect(teamEditor.data(), &ConnectionEditorDialog::accepted, [connection, teamEditor, this]() {
            connection->update(teamEditor->setting());
            connect(connection.data(), &NetworkManager::Connection::updated, this, &TeamWidget::populateTeams);
        });
        teamEditor->setModal(true);
        teamEditor->show();
    }
}

void TeamWidget::deleteTeam()
{
    QListWidgetItem *currentItem = m_ui->teams->currentItem();
    if (!currentItem)
        return;

    const QString uuid = currentItem->data(Qt::UserRole).toString();
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(uuid);

    if (connection) {
        qCDebug(PLASMA_NM_EDITOR_LOG) << "About to delete teamed connection" << currentItem->text() << uuid;
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

void TeamWidget::populateTeams()
{
    m_ui->teams->clear();

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
            auto slaveItem = new QListWidgetItem(label, m_ui->teams);
            slaveItem->setData(Qt::UserRole, connection->uuid());
        }
    }
}

void TeamWidget::importConfig()
{
    const QString filename =
        QFileDialog::getOpenFileName(this, i18n("Select file to import"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "text/plain");
    if (!filename.isEmpty()) {
        // qCDebug(PLASMA_NM_EDITOR_LOG) << "Importing" << filename;
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            m_ui->config->setPlainText(stream.readAll());
            file.close();
        }
    }
}

bool TeamWidget::isValid() const
{
    return !m_ui->ifaceName->text().isEmpty() && m_ui->teams->count() > 0;
}

#include "moc_teamwidget.cpp"
