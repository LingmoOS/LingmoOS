/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintQueue.h"
#include "PrintQueueUi.h"

#include <KCupsRequest.h>

#include <QDebug>
#include <QPointer>
#include <QTimer>

#include <KWindowSystem>
#include <KX11Extras>

PrintQueue::PrintQueue(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

PrintQueue::~PrintQueue()
{
}

void PrintQueue::showQueues(const QStringList &queues, const QString &cwd)
{
    Q_UNUSED(cwd)

    if (!queues.isEmpty()) {
        for (const QString &queue : queues) {
            showQueue(queue);
        }
    } else {
        qDebug() << "called with no args";
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, this, &PrintQueue::removeQueue);
    }
}

void PrintQueue::showQueue(const QString &destName)
{
    qDebug() << Q_FUNC_INFO << destName;
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
        for (const KCupsPrinter &printerItem : printers) {
            if (printerItem.name() == destName) {
                printer = printerItem;
                found = true;
                break;
            }
        }
        request->deleteLater();

        if (found) {
            auto ui = new PrintQueueUi(printer);
            connect(ui, &PrintQueueUi::finished, this, &PrintQueue::removeQueue);
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

void PrintQueue::removeQueue()
{
    auto ui = qobject_cast<QWidget *>(sender());
    if (ui) {
        m_uis.remove(m_uis.key(ui));
    }

    // if no destination was found and we aren't showing
    // a queue quit the app
    if (m_uis.isEmpty()) {
        quit();
    }
}

#include "moc_PrintQueue.cpp"
