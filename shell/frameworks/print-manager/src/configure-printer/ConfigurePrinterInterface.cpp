/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConfigurePrinterInterface.h"
#include "configureprinteradaptor.h"

#include "ConfigureDialog.h"
#include "Debug.h"

#include <KCupsPrinter.h>
#include <KCupsRequest.h>

#include <KX11Extras>
#include <KWindowSystem>
#include <QDBusConnection>
#include <QDialog>
#include <QPointer>
#include <QTimer>

ConfigurePrinterInterface::ConfigurePrinterInterface(QObject *parent)
    : QObject(parent)
{
    qCDebug(PM_CONFIGURE_PRINTER) << "Creating Helper";
    (void)new ConfigurePrinterAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.ConfigurePrinter"))) {
        qCDebug(PM_CONFIGURE_PRINTER) << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject(QLatin1String("/"), this)) {
        qCDebug(PM_CONFIGURE_PRINTER) << "unable to register service interface to dbus";
        return;
    }

    // setup the timer that updates the UIs
    m_updateUi = new QTimer(this);
    m_updateUi->setInterval(1000);
    m_updateUi->start();
}

ConfigurePrinterInterface::~ConfigurePrinterInterface()
{
}

void ConfigurePrinterInterface::ConfigurePrinter(const QString &destName)
{
    if (!m_uis.contains(destName)) {
        // Reserve this since the CUPS call might take a long time
        m_uis[destName] = nullptr;

        // Get destinations with these attributes
        QPointer<KCupsRequest> request = new KCupsRequest;
        request->getPrinters({KCUPS_PRINTER_NAME, KCUPS_PRINTER_TYPE});
        request->waitTillFinished();
        if (!request) {
            return;
        }

        bool found = false;
        KCupsPrinter printer;
        const KCupsPrinters printers = request->printers();
        for (const KCupsPrinter &p : printers) {
            if (p.name() == destName) {
                printer = p;
                found = true;
                break;
            }
        }
        request->deleteLater();

        if (found) {
            auto ui = new ConfigureDialog(printer.name(), printer.isClass());
            connect(m_updateUi, &QTimer::timeout, ui, static_cast<void (ConfigureDialog::*)()>(&ConfigureDialog::update));
            connect(ui, &ConfigureDialog::finished, this, &ConfigurePrinterInterface::RemovePrinter);
            ui->show();
            m_uis[printer.name()] = ui;
        } else {
            // Remove the reservation
            m_uis.remove(destName);

            // if no destination was found and we aren't showing
            // a queue quit the app
            if (m_uis.isEmpty()) {
                Q_EMIT quit();
            }
            return;
        }
    }

    // Check if it's not reserved
    if (m_uis.value(destName)) {
        if (KWindowSystem::isPlatformX11()) {
            KX11Extras::forceActiveWindow(m_uis.value(destName)->winId());
        }
    }
}

void ConfigurePrinterInterface::RemovePrinter()
{
    auto ui = qobject_cast<QWidget *>(sender());
    if (ui) {
        m_uis.remove(m_uis.key(ui));
    }

    // if no destination was found and we aren't showing
    // a queue quit the app
    if (m_uis.isEmpty()) {
        Q_EMIT quit();
    }
}

#include "moc_ConfigurePrinterInterface.cpp"
