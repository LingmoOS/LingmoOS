/*
    SPDX-FileCopyrightText: 2010 Alex Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "discover.h"
#include "../bluewizard.h"
#include "../wizardagent.h"
#include "bluedevil_wizard.h"

#include <QAction>
#include <QRegularExpressionValidator>
#include <QScroller>
#include <QSortFilterProxyModel>

#include <KLocalizedString>
#include <KMessageWidget>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/DevicesModel>
#include <BluezQt/Manager>

class DevicesProxyModel : public QSortFilterProxyModel
{
public:
    explicit DevicesProxyModel(QObject *parent);

    void setDevicesModel(BluezQt::DevicesModel *model);

    QVariant data(const QModelIndex &index, int role) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    BluezQt::DevicePtr device(const QModelIndex &index) const;

    QString searchString() const;
    void setSearchString(const QString &searchString);

private:
    BluezQt::DevicesModel *m_devicesModel = nullptr;
    QString m_searchString;
};

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

void DevicesProxyModel::setDevicesModel(BluezQt::DevicesModel *model)
{
    m_devicesModel = model;
    setSourceModel(model);
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        return QIcon::fromTheme(index.data(BluezQt::DevicesModel::IconRole).toString(), QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    qint16 leftRssi = left.data(BluezQt::DevicesModel::RssiRole).toInt();
    qint16 rightRssi = right.data(BluezQt::DevicesModel::RssiRole).toInt();

    return leftRssi < rightRssi;
}

bool DevicesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    bool devicePaired = index.data(BluezQt::DevicesModel::PairedRole).toBool();
    if (devicePaired) {
        return false;
    }

    bool adapterPowered = index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool();
    bool adapterPairable = index.data(BluezQt::DevicesModel::AdapterPairableRole).toBool();
    if (!adapterPowered || !adapterPairable) {
        return false;
    }

    if (!m_searchString.isEmpty()) {
        const QString displayString = index.data(Qt::DisplayRole).toString();
        if (!displayString.contains(m_searchString, Qt::CaseInsensitive)) {
            return false;
        }
    }

    return true;
}

BluezQt::DevicePtr DevicesProxyModel::device(const QModelIndex &index) const
{
    Q_ASSERT(m_devicesModel);
    return m_devicesModel->device(mapToSource(index));
}

QString DevicesProxyModel::searchString() const
{
    return m_searchString;
}

void DevicesProxyModel::setSearchString(const QString &searchString)
{
    if (m_searchString == searchString) {
        return;
    }

    m_searchString = searchString;
    invalidateFilter();
}

DiscoverPage::DiscoverPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_model(new DevicesProxyModel(this))
{
    setupUi(this);
    setTitle(i18n("Select a device"));

    deviceView->setModel(m_model);

    connect(deviceView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DiscoverPage::indexSelected);
    connect(deviceView, &QListView::doubleClicked, this, &DiscoverPage::itemDoubleClicked);

    QAction *findAction = new QAction(this);
    connect(findAction, &QAction::triggered, searchField, qOverload<>(&QWidget::setFocus));
    findAction->setShortcut(QKeySequence::Find);
    connect(searchField, &QLineEdit::textChanged, m_model, &DevicesProxyModel::setSearchString);
    addAction(findAction);
}

void DiscoverPage::initializePage()
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Initialize Discover Page";

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::NextButton;
    list << QWizard::CancelButton;
    m_wizard->setButtonLayout(list);

    QRegularExpression rx(QStringLiteral("[0-9]{1,16}"));
    pinText->setValidator(new QRegularExpressionValidator(rx, this));

    connect(manualPin, &QCheckBox::toggled, pinText, &QLineEdit::setEnabled);
    connect(manualPin, &QCheckBox::toggled, this, &DiscoverPage::completeChanged);
    connect(pinText, &QLineEdit::textChanged, this, &DiscoverPage::completeChanged);

    m_manager = m_wizard->manager();

    m_adapter = m_manager->usableAdapter();
    if (m_adapter && !m_adapter->isDiscovering()) {
        qCDebug(BLUEDEVIL_WIZARD_LOG) << "Starting scanning";
        m_adapter->startDiscovery();
    }

    if (!m_model->sourceModel()) {
        m_model->setDevicesModel(new BluezQt::DevicesModel(m_manager, this));
    }

    deviceView->setCurrentIndex(QModelIndex());
    manualPin->setChecked(false);
    pinText->clear();

    checkAdapters();
    connect(m_manager, &BluezQt::Manager::adapterAdded, this, &DiscoverPage::checkAdapters);
    connect(m_manager, &BluezQt::Manager::adapterChanged, this, &DiscoverPage::checkAdapters);
    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, &DiscoverPage::checkAdapters);
    connect(m_manager, &BluezQt::Manager::usableAdapterChanged, this, &DiscoverPage::usableAdapterChanged);

    QScroller::grabGesture(deviceView);
}

bool DiscoverPage::isComplete() const
{
    if (!m_wizard->device() || m_wizard->device()->isPaired()) {
        return false;
    }

    if (manualPin->isChecked() && pinText->text().isEmpty()) {
        return false;
    }

    return true;
}

int DiscoverPage::nextId() const
{
    if (!isComplete()) {
        return BlueWizard::Discover;
    }

    if (!m_wizard) {
        return BlueWizard::Discover;
    }

    if (!m_wizard->device()) {
        return BlueWizard::Discover;
    }

    BluezQt::DevicePtr device = m_wizard->device();

    if (device->isPaired()) {
        return BlueWizard::Discover;
    }

    if (m_adapter && m_adapter->isDiscovering()) {
        qCDebug(BLUEDEVIL_WIZARD_LOG) << "Stopping scanning";
        m_adapter->stopDiscovery();
    }

    QString pin = m_wizard->agent()->getPin(device);

    if (manualPin->isChecked()) {
        pin = pinText->text();
        m_wizard->agent()->setPin(pin);
    }

    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Device type: " << BluezQt::Device::typeToString(device->type());
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Legacy: " << device->hasLegacyPairing();
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "From DB: " << m_wizard->agent()->isFromDatabase();
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "PIN: " << pin;

    // NULL pin means that we should only connect to device
    if (pin == QLatin1String("NULL")) {
        return BlueWizard::Connect;
    }

    return BlueWizard::Pairing;
}

void DiscoverPage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    // Focus the device view by default, not the search field.
    deviceView->setFocus();
}

void DiscoverPage::indexSelected(const QModelIndex &index)
{
    if (m_wizard->currentId() != BlueWizard::Discover) {
        return;
    }

    BluezQt::DevicePtr device = m_model->device(index);
    m_wizard->setDevice(device);

    Q_EMIT completeChanged();
}

void DiscoverPage::itemDoubleClicked(const QModelIndex &index)
{
    indexSelected(index);
    m_wizard->next();
}

void DiscoverPage::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    m_adapter = adapter;

    if (m_adapter && !m_adapter->isDiscovering()) {
        m_adapter->startDiscovery();
    }

    checkAdapters();
}

void DiscoverPage::checkAdapters()
{
    bool error = false;

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        if (!adapter->isPowered() || !adapter->isPairable()) {
            error = true;
            break;
        }
    }

    delete m_warningWidget;
    m_warningWidget = nullptr;

    if (!error && !m_manager->isBluetoothBlocked()) {
        return;
    }

    QAction *fixAdapters = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_warningWidget);
    connect(fixAdapters, &QAction::triggered, this, &DiscoverPage::fixAdaptersError);

    m_warningWidget = new KMessageWidget(this);
    m_warningWidget->setMessageType(KMessageWidget::Warning);
    m_warningWidget->setCloseButtonVisible(false);
    if (m_manager->isBluetoothBlocked()) {
        m_warningWidget->setText(i18n("Bluetooth is disabled."));
        fixAdapters->setText(i18nc("Action to enable Bluetooth adapter", "Enable"));
    } else {
        m_warningWidget->setText(i18n("Your Bluetooth adapter is not pairable."));
        fixAdapters->setText(i18nc("Action to make Bluetooth adapter pairable", "Make Pairable"));
    }

    m_warningWidget->addAction(fixAdapters);
    verticalLayout->insertWidget(0, m_warningWidget);
}

void DiscoverPage::fixAdaptersError()
{
    m_manager->setBluetoothBlocked(false);

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        adapter->setPowered(true);
        adapter->setPairable(true);
    }
}

#include "moc_discover.cpp"
