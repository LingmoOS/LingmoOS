/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PageDestinations.h"
#include "ui_PageDestinations.h"

#include <kde-add-printer_log.h>

#include "DevicesModel.h"

#include "ChooseLpd.h"
#include "ChooseSamba.h"
#include "ChooseSerial.h"
#include "ChooseSocket.h"
#include "ChooseUri.h"

#include <KCupsRequest.h>
#include <KLocalizedString>
#include <NoSelectionRectDelegate.h>

#include <QDebug>
#include <QItemSelectionModel>

// system-config-printer --setup-printer='file:/tmp/printout' --devid='MFG:Ricoh;MDL:Aficio SP C820DN'
PageDestinations::PageDestinations(const QVariantMap &args, QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::PageDestinations)
    , m_chooseLpd(new ChooseLpd(this))
    , m_chooseSamba(new ChooseSamba(this))
    , m_chooseSerial(new ChooseSerial(this))
    , m_chooseSocket(new ChooseSocket(this))
    , m_chooseUri(new ChooseUri(this))
    , m_chooseLabel(new QLabel(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->stackedWidget->addWidget(m_chooseLpd);
    connect(m_chooseLpd, &ChooseLpd::allowProceed, this, &PageDestinations::allowProceed);
    connect(m_chooseLpd, &ChooseLpd::startWorking, this, &PageDestinations::working);
    connect(m_chooseLpd, &ChooseLpd::stopWorking, this, &PageDestinations::notWorking);

    ui->stackedWidget->addWidget(m_chooseSamba);
    connect(m_chooseSamba, &ChooseSamba::allowProceed, this, &PageDestinations::allowProceed);
    connect(m_chooseSamba, &ChooseSamba::startWorking, this, &PageDestinations::working);
    connect(m_chooseSamba, &ChooseSamba::stopWorking, this, &PageDestinations::notWorking);

    ui->stackedWidget->addWidget(m_chooseSerial);
    connect(m_chooseSerial, &ChooseSerial::allowProceed, this, &PageDestinations::allowProceed);
    connect(m_chooseSerial, &ChooseSerial::startWorking, this, &PageDestinations::working);
    connect(m_chooseSerial, &ChooseSerial::stopWorking, this, &PageDestinations::notWorking);

    ui->stackedWidget->addWidget(m_chooseSocket);
    connect(m_chooseSocket, &ChooseSocket::allowProceed, this, &PageDestinations::allowProceed);
    connect(m_chooseSocket, &ChooseSocket::startWorking, this, &PageDestinations::working);
    connect(m_chooseSocket, &ChooseSocket::stopWorking, this, &PageDestinations::notWorking);

    ui->stackedWidget->addWidget(m_chooseUri);
    connect(m_chooseUri, &ChooseUri::allowProceed, this, &PageDestinations::allowProceed);
    connect(m_chooseUri, &ChooseUri::startWorking, this, &PageDestinations::working);
    connect(m_chooseUri, &ChooseUri::stopWorking, this, &PageDestinations::notWorking);
    ui->messageWidget->setWordWrap(true);
    connect(m_chooseUri, &ChooseUri::errorMessage, ui->messageWidget, &KMessageWidget::setText);
    connect(m_chooseUri, &ChooseUri::errorMessage, ui->messageWidget, &KMessageWidget::animatedShow);
    connect(m_chooseUri, &ChooseUri::insertDevice, this, &PageDestinations::insertDevice);

    m_chooseLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->stackedWidget->addWidget(m_chooseLabel);

    // Hide the message widget
    ui->messageWidget->setWordWrap(true);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->hide();

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    m_model = new DevicesModel(this);
    ui->devicesTV->setModel(m_model);
    ui->devicesTV->setItemDelegate(new NoSelectionRectDelegate(this));
    connect(ui->devicesTV->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PageDestinations::deviceChanged);
    connect(m_model, &DevicesModel::errorMessage, ui->messageWidget, &KMessageWidget::setText);
    connect(m_model, &DevicesModel::errorMessage, ui->messageWidget, &KMessageWidget::animatedShow);

    // Expand when a parent is added
    connect(m_model, &DevicesModel::parentAdded, ui->devicesTV, &QTreeView::expand);

    // Update the view when the device URI combo box changed
    connect(ui->connectionsCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PageDestinations::deviceUriChanged);
    ui->connectionsGB->setVisible(false);

    // Setup the busy cursor
    working();
    connect(m_model, &DevicesModel::loaded, this, &PageDestinations::notWorking);

    if (!args.isEmpty()) {
        // set our args
        setValues(args);
    }
}

PageDestinations::~PageDestinations()
{
    delete ui;
}

void PageDestinations::setValues(const QVariantMap &args)
{
    m_args = args;
    if (args[ADDING_PRINTER].toBool()) {
        //        m_isValid = true;
        m_model->update();
        //        m_busySeq->start();
    } else {
        //        m_isValid = false;
    }
}

bool PageDestinations::isValid() const
{
    return true;
}

QVariantMap PageDestinations::values() const
{
    QVariantMap ret = m_args;
    auto page = qobject_cast<GenericPage *>(ui->stackedWidget->currentWidget());
    if (page) {
        ret = page->values();
    } else if (canProceed()) {
        ret = selectedItemValues();
    }
    return ret;
}

bool PageDestinations::canProceed() const
{
    bool ret = ui->stackedWidget->currentIndex() != 0;

    auto page = qobject_cast<GenericPage *>(ui->stackedWidget->currentWidget());
    if (page) {
        ret = page->canProceed();
    }

    return ret;
}

void PageDestinations::deviceChanged()
{
    QItemSelectionModel *selection = ui->devicesTV->selectionModel();
    if (!selection->selectedIndexes().isEmpty() && selection->selectedIndexes().size() == 1) {
        QModelIndex index = selection->selectedIndexes().first();
        QVariant uris = index.data(DevicesModel::DeviceUris);
        if (uris.isNull()) {
            ui->connectionsGB->setVisible(false);
        } else if (uris.type() == QVariant::StringList) {
            ui->connectionsCB->clear();
            for (const QString &uri : uris.toStringList()) {
                ui->connectionsCB->addItem(m_model->uriDevice(uri), uri);
            }
            ui->connectionsGB->setVisible(true);
        } else {
            ui->connectionsCB->clear();
            const auto printers = uris.value<KCupsPrinters>();
            for (const KCupsPrinter &printer : printers) {
                ui->connectionsCB->addItem(printer.name(), QVariant::fromValue(printer));
            }
            ui->connectionsGB->setVisible(true);
        }
    } else {
        ui->connectionsGB->setVisible(false);
        setCurrentPage(nullptr, selectedItemValues());
        return;
    }

    deviceUriChanged();
}

void PageDestinations::deviceUriChanged()
{
    // Get the selected values
    QVariantMap args = selectedItemValues();

    // "beh" is excluded from the list
    QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    qCDebug(PM_ADD_PRINTER) << deviceUri;
    if (deviceUri.startsWith(QLatin1String("parallel"))) {
        m_chooseLabel->setText(i18n("A printer connected to the parallel port."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("usb"))) {
        m_chooseLabel->setText(i18n("A printer connected to a USB port."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("bluetooth"))) {
        m_chooseLabel->setText(i18n("A printer connected via Bluetooth."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hal"))) {
        m_chooseLabel->setText(
            i18n("Local printer detected by the "
                 "Hardware Abstraction Layer (HAL)."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hp"))) {
        m_chooseLabel->setText(
            i18n("HPLIP software driving a printer, "
                 "or the printer function of a multi-function device."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hpfax"))) {
        m_chooseLabel->setText(
            i18n("HPLIP software driving a fax machine, "
                 "or the fax function of a multi-function device."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("dnssd")) || deviceUri.startsWith(QLatin1String("mdns"))) {
        // TODO this needs testing...
        QString text;
        if (deviceUri.contains(QLatin1String("cups"))) {
            text = i18n("Remote CUPS printer via DNS-SD");
        } else {
            QString protocol;
            if (deviceUri.contains(QLatin1String("._ipp"))) {
                protocol = QLatin1String("IPP");
            } else if (deviceUri.contains(QLatin1String("._printer"))) {
                protocol = QLatin1String("LPD");
            } else if (deviceUri.contains(QLatin1String("._pdl-datastream"))) {
                protocol = QLatin1String("AppSocket/JetDirect");
            }

            if (protocol.isNull()) {
                text = i18n("Network printer via DNS-SD");
            } else {
                text = i18n("%1 network printer via DNS-SD", protocol);
            }
        }
        m_chooseLabel->setText(text);
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("socket"))) {
        qCDebug(PM_ADD_PRINTER) << "SOCKET";
        setCurrentPage(m_chooseSocket, args);
    } else if (deviceUri.startsWith(QLatin1String("ipp")) || deviceUri.startsWith(QLatin1String("ipps")) || deviceUri.startsWith(QLatin1String("http"))
               || deviceUri.startsWith(QLatin1String("https"))) {
        setCurrentPage(m_chooseUri, args);
    } else if (deviceUri.startsWith(QLatin1String("lpd"))) {
        setCurrentPage(m_chooseLpd, args);
    } else if (deviceUri.startsWith(QLatin1String("scsi"))) {
        // TODO
        setCurrentPage(m_chooseUri, args);
    } else if (deviceUri.startsWith(QLatin1String("serial"))) {
        setCurrentPage(m_chooseSerial, args);
    } else if (deviceUri.startsWith(QLatin1String("smb"))) {
        setCurrentPage(m_chooseSamba, args);
    } else if (deviceUri.startsWith(QLatin1String("network"))) {
        setCurrentPage(m_chooseUri, args);
    } else {
        setCurrentPage(m_chooseUri, args);
    }

    Q_EMIT allowProceed(canProceed());
}

void PageDestinations::insertDevice(const QString &device_class,
                                    const QString &device_id,
                                    const QString &device_info,
                                    const QString &device_make_and_model,
                                    const QString &device_uri,
                                    const QString &device_location,
                                    const KCupsPrinters &grouped_printers)
{
    m_model->insertDevice(device_class, device_id, device_info, device_make_and_model, device_uri, device_location, grouped_printers);
}

QVariantMap PageDestinations::selectedItemValues() const
{
    QVariantMap ret = m_args;
    if (!ui->devicesTV->selectionModel()->selectedIndexes().isEmpty() && ui->devicesTV->selectionModel()->selectedIndexes().size() == 1) {
        QModelIndex index = ui->devicesTV->selectionModel()->selectedIndexes().first();
        QVariant uri = index.data(DevicesModel::DeviceUri);
        QVariant uris = index.data(DevicesModel::DeviceUris);
        // if the devicesTV holds an item with grouped URIs
        // get the selected value from the connections combo box
        if (uris.isNull() || uris.type() == QVariant::StringList) {
            if (uris.type() == QVariant::StringList) {
                uri = ui->connectionsCB->itemData(ui->connectionsCB->currentIndex());
            }
            ret[KCUPS_DEVICE_URI] = uri;
            ret[KCUPS_DEVICE_ID] = index.data(DevicesModel::DeviceId);
            ret[KCUPS_DEVICE_MAKE_AND_MODEL] = index.data(DevicesModel::DeviceMakeAndModel);
            ret[KCUPS_DEVICE_INFO] = index.data(DevicesModel::DeviceInfo);
            ret[KCUPS_DEVICE_LOCATION] = index.data(DevicesModel::DeviceLocation);
        } else {
            QVariant aux = ui->connectionsCB->itemData(ui->connectionsCB->currentIndex());
            KCupsPrinter printer = aux.value<KCupsPrinter>();
            QUrl url(uri.toString());
            url.setPath(QLatin1String("printers/") + printer.name());
            ret[KCUPS_DEVICE_URI] = url.url();
            ret[KCUPS_DEVICE_ID] = index.data(DevicesModel::DeviceId);
            ret[KCUPS_PRINTER_INFO] = printer.info();
            qCDebug(PM_ADD_PRINTER) << KCUPS_PRINTER_INFO << printer.info();
            ret[KCUPS_PRINTER_NAME] = printer.name();
            ret[KCUPS_DEVICE_LOCATION] = printer.location();
        }
        qCDebug(PM_ADD_PRINTER) << uri << ret;
    }
    return ret;
}

void PageDestinations::setCurrentPage(QWidget *widget, const QVariantMap &args)
{
    auto page = qobject_cast<GenericPage *>(widget);
    if (page) {
        page->setValues(args);
        if (ui->stackedWidget->currentWidget() != page) {
            ui->stackedWidget->setCurrentWidget(page);
        }
    } else if (qobject_cast<QLabel *>(widget)) {
        if (ui->connectionsGB->isVisible() && ui->connectionsCB->currentText() == m_chooseLabel->text()) {
            // Don't show duplicated text for the user
            m_chooseLabel->clear();
        }

        if (ui->stackedWidget->currentWidget() != widget) {
            ui->stackedWidget->setCurrentWidget(widget);
        }
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

#include "moc_PageDestinations.cpp"
