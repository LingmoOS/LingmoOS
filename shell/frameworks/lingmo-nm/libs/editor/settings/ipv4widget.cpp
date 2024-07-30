/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipv4widget.h"
#include "ipv4delegate.h"
#include "ui_ipv4.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHostInfo>
#include <QItemSelection>
#include <QNetworkAddressEntry>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QUrl>

#include <KEditListWidget>
#include <KLocalizedString>

quint32 suggestNetmask(quint32 ip)
{
    /*
        A   0       0.0.0.0 <-->127.255.255.255  255.0.0.0 <--->/8
        B   10      128.0.0.0 <>191.255.255.255  255.255.0.0 <->/16
        C   110     192.0.0.0 <>223.255.255.255  255.255.255.0 >/24
        D   1110    224.0.0.0 <>239.255.255.255  not defined <->not defined
        E   1111    240.0.0.0 <>255.255.255.254  not defined <->not defined
    */
    quint32 netmask = 0;

    if (!(ip & 0x80000000)) {
        // test 0 leading bit
        netmask = 0xFF000000;
    } else if (!(ip & 0x40000000)) {
        // test 10 leading bits
        netmask = 0xFFFF0000;
    } else if (!(ip & 0x20000000)) {
        // test 110 leading bits
        netmask = 0xFFFFFF00;
    }

    return netmask;
}

class IPv4Widget::Private
{
public:
    Private()
        : model(0, 3)
    {
        auto headerItem = new QStandardItem(i18nc("Header text for IPv4 address", "Address"));
        model.setHorizontalHeaderItem(0, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 netmask", "Netmask"));
        model.setHorizontalHeaderItem(1, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 gateway", "Gateway"));
        model.setHorizontalHeaderItem(2, headerItem);
    }
    QStandardItemModel model;
};

IPv4Widget::IPv4Widget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::IPv4Widget)
    , d(new IPv4Widget::Private())
{
    m_ui->setupUi(this);

    m_ui->tableViewAddresses->setModel(&d->model);
    m_ui->tableViewAddresses->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_ui->tableViewAddresses->horizontalHeader()->setStretchLastSection(true);

    auto ipDelegate = new IpV4Delegate(this);
    m_ui->tableViewAddresses->setItemDelegateForColumn(0, ipDelegate);
    m_ui->tableViewAddresses->setItemDelegateForColumn(1, ipDelegate);
    m_ui->tableViewAddresses->setItemDelegateForColumn(2, ipDelegate);

    connect(m_ui->btnAdd, &QPushButton::clicked, this, &IPv4Widget::slotAddIPAddress);
    connect(m_ui->btnRemove, &QPushButton::clicked, this, &IPv4Widget::slotRemoveIPAddress);

    connect(m_ui->dnsMorePushButton, &QPushButton::clicked, this, &IPv4Widget::slotDnsServers);
    connect(m_ui->dnsSearchMorePushButton, &QPushButton::clicked, this, &IPv4Widget::slotDnsDomains);

    connect(m_ui->tableViewAddresses->selectionModel(), &QItemSelectionModel::selectionChanged, this, &IPv4Widget::selectionChanged);

    connect(&d->model, &QStandardItemModel::itemChanged, this, &IPv4Widget::tableViewItemChanged);

    if (setting) {
        loadConfig(setting);
    }

    connect(m_ui->method, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &IPv4Widget::slotModeComboChanged);
    slotModeComboChanged(m_ui->method->currentIndex());

    connect(m_ui->btnRoutes, &QPushButton::clicked, this, &IPv4Widget::slotRoutesDialog);
    connect(m_ui->btnAdvanced, &QPushButton::clicked, this, &IPv4Widget::slotAdvancedDialog);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->dns, &KLineEdit::textChanged, this, &IPv4Widget::slotWidgetChanged);
    connect(m_ui->method, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &IPv4Widget::slotWidgetChanged);
    connect(&d->model, &QStandardItemModel::dataChanged, this, &IPv4Widget::slotWidgetChanged);
    connect(&d->model, &QStandardItemModel::rowsRemoved, this, &IPv4Widget::slotWidgetChanged);

    KAcceleratorManager::manage(this);
}

IPv4Widget::~IPv4Widget()
{
    delete d;
    delete m_ui;
}

void IPv4Widget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::Ipv4Setting::Ptr ipv4Setting = setting.staticCast<NetworkManager::Ipv4Setting>();

    // BUG:406118
    // We don't have route-metric in the UI, maybe even won't have for now, but that doesn't mean we
    // want to loose it when it's configured manually in a config file
    m_tmpIpv4Setting.setRouteMetric(ipv4Setting->routeMetric());

    m_tmpIpv4Setting.setRoutes(ipv4Setting->routes());
    m_tmpIpv4Setting.setNeverDefault(ipv4Setting->neverDefault());
    m_tmpIpv4Setting.setIgnoreAutoRoutes(ipv4Setting->ignoreAutoRoutes());

    m_tmpIpv4Setting.setDhcpHostname(ipv4Setting->dhcpHostname());
    m_tmpIpv4Setting.setDhcpSendHostname(ipv4Setting->dhcpSendHostname());
    m_tmpIpv4Setting.setDadTimeout(ipv4Setting->dadTimeout());

    // method
    switch (ipv4Setting->method()) {
    case NetworkManager::Ipv4Setting::Automatic:
        if (ipv4Setting->ignoreAutoDns()) {
            m_ui->method->setCurrentIndex(AutomaticOnlyIP);
        } else {
            m_ui->method->setCurrentIndex(Automatic);
        }
        break;
    case NetworkManager::Ipv4Setting::Manual:
        m_ui->method->setCurrentIndex(Manual);
        break;
    case NetworkManager::Ipv4Setting::LinkLocal:
        m_ui->method->setCurrentIndex(LinkLocal);
        break;
    case NetworkManager::Ipv4Setting::Shared:
        m_ui->method->setCurrentIndex(Shared);
        break;
    case NetworkManager::Ipv4Setting::Disabled:
        m_ui->method->setCurrentIndex(Disabled);
        break;
    }

    // dns
    QStringList tmp;
    for (const QHostAddress &addr : ipv4Setting->dns()) {
        tmp.append(addr.toString());
    }
    m_ui->dns->setText(tmp.join(QStringLiteral(",")));
    m_ui->dnsSearch->setText(ipv4Setting->dnsSearch().join(QStringLiteral(",")));

    m_ui->dhcpClientId->setText(ipv4Setting->dhcpClientId());

    // addresses
    for (const NetworkManager::IpAddress &addr : ipv4Setting->addresses()) {
        QList<QStandardItem *> item{
            new QStandardItem(addr.ip().toString()),
            new QStandardItem(addr.netmask().toString()),
            new QStandardItem(addr.gateway().toString()),
        };

        d->model.appendRow(item);
    }

    // may-fail
    m_ui->ipv4RequiredCB->setChecked(!ipv4Setting->mayFail());
}

QVariantMap IPv4Widget::setting() const
{
    NetworkManager::Ipv4Setting ipv4Setting;

    // BUG:406118
    // We don't have route-metric in the UI, maybe even won't have for now, but that doesn't mean we
    // want to loose it when it's configured manually in a config file
    ipv4Setting.setRouteMetric(m_tmpIpv4Setting.routeMetric());

    ipv4Setting.setRoutes(m_tmpIpv4Setting.routes());
    ipv4Setting.setNeverDefault(m_tmpIpv4Setting.neverDefault());
    ipv4Setting.setIgnoreAutoRoutes(m_tmpIpv4Setting.ignoreAutoRoutes());

    ipv4Setting.setDhcpHostname(m_tmpIpv4Setting.dhcpHostname());
    ipv4Setting.setDhcpSendHostname(m_tmpIpv4Setting.dhcpSendHostname());
    ipv4Setting.setDadTimeout(m_tmpIpv4Setting.dadTimeout());

    // method
    switch ((MethodIndex)m_ui->method->currentIndex()) {
    case Automatic:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Automatic);
        break;
    case IPv4Widget::AutomaticOnlyIP:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Automatic);
        ipv4Setting.setIgnoreAutoDns(true);
        break;
    case Manual:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Manual);
        break;
    case LinkLocal:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::LinkLocal);
        break;
    case Shared:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Shared);
        break;
    case Disabled:
        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Disabled);
        break;
    }

    // dns
    if (m_ui->dns->isEnabled() && !m_ui->dns->text().isEmpty()) {
        QStringList tmp = m_ui->dns->text().split(QLatin1Char(','));
        QList<QHostAddress> tmpAddrList;
        for (const QString &str : tmp) {
            QHostAddress addr(str);
            if (!addr.isNull())
                tmpAddrList.append(addr);
        }
        ipv4Setting.setDns(tmpAddrList);
    }
    if (m_ui->dnsSearch->isEnabled() && !m_ui->dnsSearch->text().isEmpty()) {
        ipv4Setting.setDnsSearch(m_ui->dnsSearch->text().split(QLatin1Char(',')));
    }

    // dhcp id
    if (m_ui->dhcpClientId->isEnabled() && !m_ui->dhcpClientId->text().isEmpty()) {
        ipv4Setting.setDhcpClientId(m_ui->dhcpClientId->text());
    }

    // addresses
    if (m_ui->tableViewAddresses->isEnabled()) {
        QList<NetworkManager::IpAddress> list;
        for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
            NetworkManager::IpAddress address;
            address.setIp(QHostAddress(d->model.item(i, 0)->text()));
            address.setNetmask(QHostAddress(d->model.item(i, 1)->text()));
            address.setGateway(QHostAddress(d->model.item(i, 2)->text()));
            list << address;
        }
        if (!list.isEmpty()) {
            ipv4Setting.setAddresses(list);
        }
    }

    // may-fail
    if (m_ui->ipv4RequiredCB->isEnabled()) {
        ipv4Setting.setMayFail(!m_ui->ipv4RequiredCB->isChecked());
    }

    return ipv4Setting.toMap();
}

void IPv4Widget::slotModeComboChanged(int index)
{
    if (index == Automatic) { // Automatic
        m_ui->dnsLabel->setText(i18n("Other DNS Servers:"));
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(true);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);
        m_ui->tableViewAddresses->setEnabled(false);
        m_ui->btnAdd->setEnabled(false);
        m_ui->btnRemove->setEnabled(false);
    } else if (index == AutomaticOnlyIP) {
        m_ui->dnsLabel->setText(i18n("DNS Servers:"));
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(true);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);
        m_ui->tableViewAddresses->setEnabled(false);
        m_ui->btnAdd->setEnabled(false);
        m_ui->btnRemove->setEnabled(false);
    } else if (index == Manual) { // Manual
        m_ui->dnsLabel->setText(i18n("DNS Servers:"));
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);
        m_ui->tableViewAddresses->setEnabled(true);
        m_ui->btnAdd->setEnabled(true);
        m_ui->btnRemove->setEnabled(true);
    } else if (index == LinkLocal || index == Shared) { // Link-local or Shared
        m_ui->dnsLabel->setText(i18n("DNS Servers:"));
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(false);
        m_ui->tableViewAddresses->setEnabled(false);
        m_ui->btnAdd->setEnabled(false);
        m_ui->btnRemove->setEnabled(false);
    } else if (index == Disabled) { // Disabled
        m_ui->dnsLabel->setText(i18n("DNS Servers:"));
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(false);
        m_ui->btnRoutes->setEnabled(false);
        m_ui->tableViewAddresses->setEnabled(false);
        m_ui->btnAdd->setEnabled(false);
        m_ui->btnRemove->setEnabled(false);
    }
}

void IPv4Widget::slotAddIPAddress()
{
    QList<QStandardItem *> item{new QStandardItem, new QStandardItem, new QStandardItem};
    d->model.appendRow(item);

    const int rowCount = d->model.rowCount();
    if (rowCount > 0) {
        m_ui->tableViewAddresses->selectRow(rowCount - 1);

        QItemSelectionModel *selectionModel = m_ui->tableViewAddresses->selectionModel();
        QModelIndexList list = selectionModel->selectedIndexes();
        if (!list.isEmpty()) {
            // QTableView is configured to select only rows.
            // So, list[0] - IP address.
            m_ui->tableViewAddresses->edit(list[0]);
        }
    }
}

void IPv4Widget::slotRemoveIPAddress()
{
    QItemSelectionModel *selectionModel = m_ui->tableViewAddresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    m_ui->btnRemove->setEnabled(m_ui->tableViewAddresses->selectionModel()->hasSelection());
}

void IPv4Widget::selectionChanged(const QItemSelection &selected)
{
    m_ui->btnRemove->setEnabled(!selected.isEmpty());
}

void IPv4Widget::tableViewItemChanged(QStandardItem *item)
{
    if (item->text().isEmpty()) {
        return;
    }

    const int column = item->column();
    if (column == 0) { // ip
        int row = item->row();

        QStandardItem *netmaskItem = d->model.item(row, column + 1); // netmask
        if (netmaskItem && netmaskItem->text().isEmpty()) {
            QHostAddress addr(item->text());
            const quint32 netmask = suggestNetmask(addr.toIPv4Address());
            if (netmask) {
                QHostAddress v(netmask);
                netmaskItem->setText(v.toString());
            }
        }
    }
}

void IPv4Widget::slotRoutesDialog()
{
    QPointer<IpV4RoutesWidget> dlg = new IpV4RoutesWidget(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    dlg->setRoutes(m_tmpIpv4Setting.routes());
    dlg->setNeverDefault(m_tmpIpv4Setting.neverDefault());
    if (m_ui->method->currentIndex() == 2) { // manual
        dlg->setIgnoreAutoRoutesCheckboxEnabled(false);
    } else {
        dlg->setIgnoreAutoRoutes(m_tmpIpv4Setting.ignoreAutoRoutes());
    }

    connect(dlg.data(), &QDialog::accepted, [dlg, this]() {
        m_tmpIpv4Setting.setRoutes(dlg->routes());
        m_tmpIpv4Setting.setNeverDefault(dlg->neverDefault());
        m_tmpIpv4Setting.setIgnoreAutoRoutes(dlg->ignoreautoroutes());
    });
    dlg->setModal(true);
    dlg->show();
}

void IPv4Widget::slotAdvancedDialog()
{
    auto dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    auto layout = new QFormLayout(dlg);
    dlg->setLayout(layout);

    auto label =
        new QLabel(i18n("<qt>You can find more information about these values here:<br/><a "
                        "href='https://developer.gnome.org/NetworkManager/stable/ch01.html'>https://developer.gnome.org/NetworkManager/stable/ch01.html"
                        "nm-settings-nmcli.html</a></qt>"));
    connect(label, &QLabel::linkActivated, this, [](const QString &link) {
        QDesktopServices::openUrl(QUrl(link));
    });
    layout->addRow(label);

    auto sendHostname = new QCheckBox(dlg);
    sendHostname->setChecked(m_tmpIpv4Setting.dhcpSendHostname());
    layout->addRow(i18n("Send hostname:"), sendHostname);

    auto dhcpHostname = new QLineEdit(dlg);
    dhcpHostname->setText(m_tmpIpv4Setting.dhcpHostname());
    dhcpHostname->setPlaceholderText(QHostInfo::localHostName());
    layout->addRow(i18n("DHCP hostname:"), dhcpHostname);

    connect(sendHostname, &QCheckBox::toggled, dhcpHostname, &QLineEdit::setEnabled);

    auto dadTimeout = new QSpinBox(dlg);
    dadTimeout->setSpecialValueText(i18n("Default"));
    dadTimeout->setSuffix(i18nc("Milliseconds", " ms"));
    dadTimeout->setMinimum(-1);
    dadTimeout->setValue(m_tmpIpv4Setting.dadTimeout());
    layout->addRow(i18n("DAD timeout:"), dadTimeout);

    auto box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    layout->addWidget(box);

    connect(dlg, &QDialog::accepted, this, [this, sendHostname, dhcpHostname, dadTimeout]() {
        m_tmpIpv4Setting.setDhcpSendHostname(sendHostname->isChecked());
        m_tmpIpv4Setting.setDhcpHostname(dhcpHostname->text());
        m_tmpIpv4Setting.setDadTimeout(dadTimeout->value());
    });

    dlg->setModal(true);
    dlg->show();
}

void IPv4Widget::slotDnsServers()
{
    QPointer<QDialog> dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(i18n("Edit DNS servers"));
    dialog->setLayout(new QVBoxLayout);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttons, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
    auto listWidget = new KEditListWidget(dialog);
    listWidget->setItems(m_ui->dns->text().split(QLatin1Char(',')).replaceInStrings(QStringLiteral(" "), QLatin1String("")));
    listWidget->lineEdit()->setFocus(Qt::OtherFocusReason);
    dialog->layout()->addWidget(listWidget);
    dialog->layout()->addWidget(buttons);
    connect(dialog.data(), &QDialog::accepted, [listWidget, this]() {
        QString text = listWidget->items().join(QStringLiteral(","));
        if (text.endsWith(',')) {
            text.chop(1);
        }
        m_ui->dns->setText(text);
    });
    dialog->setModal(true);
    dialog->show();
}

void IPv4Widget::slotDnsDomains()
{
    QPointer<QDialog> dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(i18n("Edit DNS search domains"));
    dialog->setLayout(new QVBoxLayout);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttons, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
    auto listWidget = new KEditListWidget(dialog);
    listWidget->setItems(m_ui->dnsSearch->text().split(QLatin1Char(',')).replaceInStrings(QStringLiteral(" "), QLatin1String("")));
    listWidget->lineEdit()->setFocus(Qt::OtherFocusReason);
    dialog->layout()->addWidget(listWidget);
    dialog->layout()->addWidget(buttons);
    connect(dialog.data(), &QDialog::accepted, [listWidget, this]() {
        QString text = listWidget->items().join(QStringLiteral(","));
        if (text.endsWith(',')) {
            text.chop(1);
        }
        m_ui->dnsSearch->setText(text);
    });
    dialog->setModal(true);
    dialog->show();
}

bool IPv4Widget::isValid() const
{
    if (m_ui->method->currentIndex() == Manual) {
        if (!d->model.rowCount()) {
            return false;
        }

        for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
            QHostAddress ip = QHostAddress(d->model.item(i, 0)->text());
            QHostAddress netmask = QHostAddress(d->model.item(i, 1)->text());
            QHostAddress gateway = QHostAddress(d->model.item(i, 2)->text());

            if (ip.isNull() || netmask.isNull() || (gateway.isNull() && !d->model.item(i, 2)->text().isEmpty())) {
                return false;
            }
        }
    }

    if (!m_ui->dns->text().isEmpty()
        && (m_ui->method->currentIndex() == Automatic || m_ui->method->currentIndex() == Manual || m_ui->method->currentIndex() == AutomaticOnlyIP)) {
        const QStringList tmp = m_ui->dns->text().split(QLatin1Char(','));
        for (const QString &str : tmp) {
            QHostAddress addr(str);
            if (addr.isNull()) {
                return false;
            }
        }
    }

    return true;
}

#include "moc_ipv4widget.cpp"
