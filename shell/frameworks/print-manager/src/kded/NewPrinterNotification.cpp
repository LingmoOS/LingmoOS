/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NewPrinterNotification.h"
#include "newprinternotificationadaptor.h"

#include "pmkded_log.h"

#include <KLocalizedString>
#include <KNotification>

#include <KCupsRequest.h>
#include <ProcessRunner.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

#define STATUS_SUCCESS 0
#define STATUS_MODEL_MISMATCH 1
#define STATUS_GENERIC_DRIVER 2
#define STATUS_NO_DRIVER 3

#define PRINTER_NAME "PrinterName"

NewPrinterNotification::NewPrinterNotification(QObject *parent)
    : QObject(parent)
{
    // Creates our new adaptor
    (void)new NewPrinterNotificationAdaptor(this);

    // Register the com.redhat.NewPrinterNotification interface
    if (!registerService()) {
        // in case registration fails due to another user or application running
        // keep an eye on it so we can register when available
        auto watcher = new QDBusServiceWatcher(QLatin1String("com.redhat.NewPrinterNotification"),
                                               QDBusConnection::systemBus(),
                                               QDBusServiceWatcher::WatchForUnregistration,
                                               this);
        connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &NewPrinterNotification::registerService);
    }
}

NewPrinterNotification::~NewPrinterNotification()
{
}

void NewPrinterNotification::GetReady()
{
    qCDebug(PMKDED) << "GetReady";
    // This method is all about telling the user a new printer was detected
    auto notify = new KNotification(QLatin1String("GetReady"));
    notify->setComponentName(QLatin1String("printmanager"));
    notify->setIconName(QLatin1String("printer"));
    notify->setTitle(i18n("A New Printer was detected"));
    notify->setText(i18n("Configuring new printer..."));
    notify->sendEvent();
}

// status: 0
// name: PSC_1400_series
// mfg: HP
// mdl: PSC 1400 series
// des:
// cmd: LDL,MLC,PML,DYN
void NewPrinterNotification::NewPrinter(int status,
                                        const QString &name,
                                        const QString &make,
                                        const QString &model,
                                        const QString &description,
                                        const QString &cmd)
{
    qCDebug(PMKDED) << status << name << make << model << description << cmd;

    // 1
    // "usb://Samsung/SCX-3400%20Series?serial=Z6Y1BQAC500079K&interface=1"
    // mfg "Samsung"
    // mdl "SCX-3400 Series" "" "SPL,FWV,PIC,BDN,EXT"
    // This method is all about telling the user a new printer was detected
    auto notify = new KNotification(QLatin1String("NewPrinterNotification"));
    notify->setComponentName(QLatin1String("printmanager"));
    notify->setIconName(QLatin1String("printer"));
    notify->setFlags(KNotification::Persistent);

    if (name.contains(QLatin1Char('/'))) {
        const QString devid = QString::fromLatin1("MFG:%1;MDL:%2;DES:%3;CMD:%4;").arg(make, model, description, cmd);
        setupPrinterNotification(notify, make, model, description, name + QLatin1Char('/') + devid);
    } else {
        notify->setProperty(PRINTER_NAME, name);
        // name is the name of the queue which hal_lpadmin has set up
        // automatically.

        if (status < STATUS_GENERIC_DRIVER) {
            notify->setTitle(i18n("The New Printer was Added"));
        } else {
            notify->setTitle(i18n("The New Printer is Missing Drivers"));
        }

        auto request = new KCupsRequest;
        connect(request, &KCupsRequest::finished, this, [this, notify, status, name](KCupsRequest *request) {
            const QString ppdFileName = request->printerPPD();
            // Get a list of missing executables
            getMissingExecutables(notify, status, name, ppdFileName);
            request->deleteLater();
        });
        request->getPrinterPPD(name);
    }
}

bool NewPrinterNotification::registerService()
{
    if (!QDBusConnection::systemBus().registerService(QLatin1String("com.redhat.NewPrinterNotification"))) {
        qCWarning(PMKDED) << "unable to register service to dbus";
        return false;
    }

    if (!QDBusConnection::systemBus().registerObject(QLatin1String("/com/redhat/NewPrinterNotification"), this)) {
        qCWarning(PMKDED) << "unable to register object to dbus";
        return false;
    }
    return true;
}

void NewPrinterNotification::configurePrinter()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PMKDED) << "configure printer tool" << printerName;
    ProcessRunner::configurePrinter(printerName);
}

void NewPrinterNotification::printTestPage()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PMKDED) << "printing test page for" << printerName;

    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, request, &KCupsRequest::deleteLater);
    request->printTestPage(printerName, false);
}

void NewPrinterNotification::findDriver()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PMKDED) << "find driver for" << printerName;

    // This function will show the PPD browser dialog
    // to choose a better PPD to the already added printer
    ProcessRunner::changePrinterPPD(printerName);
}

void NewPrinterNotification::setupPrinterNotification(KNotification *notify,
                                                      const QString &make,
                                                      const QString &model,
                                                      const QString &description,
                                                      const QString &arg)
{
    // name is a URI, no queue was generated, because no suitable
    // driver was found
    notify->setTitle(i18n("Missing printer driver"));
    if (!make.isEmpty() && !model.isEmpty()) {
        notify->setText(i18n("No printer driver for %1 %2.", make, model));
    } else if (!description.isEmpty()) {
        notify->setText(i18n("No printer driver for %1.", description));
    } else {
        notify->setText(i18n("No driver for this printer."));
    }
    auto searchAction = notify->addAction(i18n("Search"));
    connect(searchAction, &KNotificationAction::activated, this, [arg]() {
        qCDebug(PMKDED);
        // This function will show the PPD browser dialog
        // to choose a better PPD, queue name, location
        // in this case the printer was not added
        ProcessRunner::addPrinterFromDevice(arg);
    });

    notify->sendEvent();
}

void NewPrinterNotification::getMissingExecutables(KNotification *notify, int status, const QString &name, const QString &ppdFileName)
{
    qCDebug(PMKDED) << "get missing executables" << ppdFileName;
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                                          QLatin1String("/org/fedoraproject/Config/Printing"),
                                                          QLatin1String("org.fedoraproject.Config.Printing"),
                                                          QLatin1String("MissingExecutables"));
    message << ppdFileName;

    QDBusPendingReply<QStringList> reply = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, notify, status, name]() {
        watcher->deleteLater();
        QDBusPendingReply<QStringList> reply = *watcher;
        if (!reply.isValid()) {
            qCWarning(PMKDED) << "Invalid reply" << reply.error();
            notify->deleteLater();
            return;
        }

        const QStringList missingExecutables = reply;
        if (!missingExecutables.isEmpty()) {
            // TODO check with PackageKit about missing drivers
            qCWarning(PMKDED) << "Missing executables:" << missingExecutables;
            notify->deleteLater();
            return;
        } else if (status == STATUS_SUCCESS) {
            printerReadyNotification(notify, name);
        } else {
            // Model mismatch
            checkPrinterCurrentDriver(notify, name);
        }
    });
}

void NewPrinterNotification::checkPrinterCurrentDriver(KNotification *notify, const QString &name)
{
    // Get the new printer attributes
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, notify, name](KCupsRequest *request) {
        request->deleteLater();

        QString driver;
        // Get the new printer driver
        const auto printers = request->printers();
        if (!printers.isEmpty()) {
            driver = printers.first().makeAndModel();
        }

        // The cups request might have failed
        if (driver.isEmpty()) {
            notify->setText(i18n("'%1' has been added, please check its driver.", name));
            auto configAction = notify->addAction(i18n("Configure"));
            connect(configAction, &KNotificationAction::activated, this, &NewPrinterNotification::configurePrinter);
        } else {
            notify->setText(i18n("'%1' has been added, using the '%2' driver.", name, driver));
            auto testAction = notify->addAction(i18n("Print test page"));
            connect(testAction, &KNotificationAction::activated, this, &NewPrinterNotification::printTestPage);
            auto findAction = notify->addAction(i18n("Find driver"));
            connect(findAction, &KNotificationAction::activated, this, &NewPrinterNotification::findDriver);
        }
        notify->sendEvent();
    });
    request->getPrinterAttributes(name, false, {KCUPS_PRINTER_MAKE_AND_MODEL});
}

void NewPrinterNotification::printerReadyNotification(KNotification *notify, const QString &name)
{
    notify->setText(i18n("'%1' is ready for printing.", name));

    auto testAction = notify->addAction(i18n("Print test page"));
    connect(testAction, &KNotificationAction::activated, this, &NewPrinterNotification::printTestPage);

    auto configAction = notify->addAction(i18n("Configure"));
    connect(configAction, &KNotificationAction::activated, this, &NewPrinterNotification::configurePrinter);

    notify->sendEvent();
}

#include "moc_NewPrinterNotification.cpp"
