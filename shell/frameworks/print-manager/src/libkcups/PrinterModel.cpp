/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterModel.h"

#include "kcupslib_log.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDateTime>
#include <QMimeData>
#include <QPointer>

#include <KLocalizedString>
#include <KMessageBox>
#include <KUser>

#include <KCupsRequest.h>

#include <cups/cups.h>

PrinterModel::PrinterModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_attrs({KCUPS_PRINTER_NAME,
               KCUPS_PRINTER_STATE,
               KCUPS_PRINTER_STATE_MESSAGE,
               KCUPS_PRINTER_IS_SHARED,
               KCUPS_PRINTER_IS_ACCEPTING_JOBS,
               KCUPS_PRINTER_TYPE,
               KCUPS_PRINTER_LOCATION,
               KCUPS_PRINTER_INFO,
               KCUPS_PRINTER_MAKE_AND_MODEL,
               KCUPS_PRINTER_COMMANDS,
               KCUPS_MARKER_CHANGE_TIME,
               KCUPS_MARKER_COLORS,
               KCUPS_MARKER_LEVELS,
               KCUPS_MARKER_NAMES,
               KCUPS_MARKER_TYPES,
               KCUPS_DEVICE_URI,
               KCUPS_PRINTER_URI_SUPPORTED,
               KCUPS_MEMBER_NAMES})
{
    m_roles = QStandardItemModel::roleNames();
    m_roles[DestStatus] = "stateMessage";
    m_roles[DestName] = "printerName";
    m_roles[DestState] = "printerState";
    m_roles[DestIsDefault] = "isDefault";
    m_roles[DestIsShared] = "isShared";
    m_roles[DestIsAcceptingJobs] = "isAcceptingJobs";
    m_roles[DestIsPaused] = "isPaused";
    m_roles[DestIsClass] = "isClass";
    m_roles[DestLocation] = "location";
    m_roles[DestDescription] = "info";
    m_roles[DestKind] = "kind";
    m_roles[DestType] = "type";
    m_roles[DestCommands] = "commands";
    m_roles[DestMarkerChangeTime] = "markerChangeTime";
    m_roles[DestMarkers] = "markers";
    m_roles[DestIconName] = "iconName";
    m_roles[DestRemote] = "remote";
    m_roles[DestUri] = "printerUri";
    m_roles[DestUriSupported] = "uriSupported";
    m_roles[DestMemberNames] = "memberNames";

    // This is emitted when a printer is added
    connect(KCupsConnection::global(), &KCupsConnection::printerAdded, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(), &KCupsConnection::printerModified, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(), &KCupsConnection::printerStateChanged, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(), &KCupsConnection::printerStopped, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(), &KCupsConnection::printerRestarted, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(), &KCupsConnection::printerShutdown, this, &PrinterModel::insertUpdatePrinter);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::printerDeleted, this, &PrinterModel::printerRemoved);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrinterModel::serverChanged);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrinterModel::serverChanged);

    // Deprecated stuff that works better than the above
    connect(KCupsConnection::global(), &KCupsConnection::rhPrinterAdded, this, &PrinterModel::insertUpdatePrinterName);
    connect(KCupsConnection::global(), &KCupsConnection::rhPrinterRemoved, this, &PrinterModel::printerRemovedName);
    connect(KCupsConnection::global(), &KCupsConnection::rhQueueChanged, this, &PrinterModel::insertUpdatePrinterName);

    connect(this, &PrinterModel::rowsInserted, this, &PrinterModel::slotCountChanged);
    connect(this, &PrinterModel::rowsRemoved, this, &PrinterModel::slotCountChanged);
    connect(this, &PrinterModel::modelReset, this, &PrinterModel::slotCountChanged);

    update();
}

void PrinterModel::getDestsFinished(KCupsRequest *request)
{
    // When there is no printer IPP_NOT_FOUND is returned
    if (request->hasError() && request->error() != IPP_NOT_FOUND) {
        // clear the model after so that the proper widget can be shown
        clear();

        Q_EMIT error(request->error(), request->serverError(), request->errorMsg());
        if (request->error() == IPP_SERVICE_UNAVAILABLE && !m_unavailable) {
            m_unavailable = true;
            Q_EMIT serverUnavailableChanged(m_unavailable);
        }
    } else {
        if (m_unavailable) {
            m_unavailable = false;
            Q_EMIT serverUnavailableChanged(m_unavailable);
        }

        const KCupsPrinters printers = request->printers();
        for (int i = 0; i < printers.size(); ++i) {
            // If there is a printer and it's not the current one add it
            // as a new destination
            int dest_row = destRow(printers.at(i).name());
            if (dest_row == -1) {
                // not found, insert new one
                insertDest(i, printers.at(i));
            } else if (dest_row == i) {
                // update the printer
                updateDest(item(i), printers.at(i));
            } else {
                // found at wrong position
                // take it and insert on the right position
                QList<QStandardItem *> row = takeRow(dest_row);
                insertRow(i, row);
                updateDest(item(i), printers.at(i));
            }
        }

        // remove old printers
        // The above code starts from 0 and make sure
        // dest == modelIndex(x) and if it's not the
        // case it either inserts or moves it.
        // so any item > num_jobs can be safely deleted
        while (rowCount() > printers.size()) {
            removeRow(rowCount() - 1);
        }

        setDisplayLocationHint();

        Q_EMIT error(IPP_OK, QString(), QString());
    }
    request->deleteLater();
}

void PrinterModel::setDisplayLocationHint()
{
    QStringList locList;

    // get location list
    for (int i = 0; i < rowCount(); i++) {
        const auto val = item(i)->data(DestLocation).toString();
        if (!val.isEmpty()) {
            locList.append(val);
        }
    }
    // only show the location if there is more than one printer
    // and at least two distinct locations exist
    locList.removeDuplicates();
    m_displayLocationHint = rowCount() > 1 && locList.count() > 1;
    Q_EMIT displayLocationHintChanged();
}

bool PrinterModel::printersOnly() const
{
    for (int i = 0; i < rowCount(); i++) {
        if (item(i)->data(DestIsClass).toBool()) {
            return false;
        }
    }
    return true;
}

bool PrinterModel::displayLocationHint() const
{
    return m_displayLocationHint;
}

void PrinterModel::slotCountChanged()
{
    Q_EMIT countChanged(rowCount());
}

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

int PrinterModel::count() const
{
    return rowCount();
}

bool PrinterModel::serverUnavailable() const
{
    return m_unavailable;
}

QHash<int, QByteArray> PrinterModel::roleNames() const
{
    return m_roles;
}

void PrinterModel::pausePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->pausePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::resumePrinter(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->resumePrinter(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::rejectJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->rejectJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::acceptJobs(const QString &printerName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->acceptJobs(printerName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterModel::update()
{
    //                 kcmshell(6331) PrinterModel::update: (QHash(("printer-type", QVariant(int, 75534348) ) ( "marker-names" ,  QVariant(QStringList, ("Cyan",
    //                 "Yellow", "Magenta", "Black") ) ) ( "printer-name" ,  QVariant(QString, "EPSON_Stylus_TX105") ) ( "marker-colors" , QVariant(QStringList,
    //                 ("#00ffff", "#ffff00", "#ff00ff", "#000000") ) ) ( "printer-location" ,  QVariant(QString, "Luiz Vitor’s MacBook Pro") ) (
    //                 "marker-levels" ,  QVariant(QList<int>, ) ) ( "marker-types" ,  QVariant(QStringList, ("inkCartridge", "inkCartridge", "inkCartridge",
    //                 "inkCartridge") ) ) ( "printer-is-shared" ,  QVariant(bool, true) ) ( "printer-state-message" ,  QVariant(QString, "") ) (
    //                 "printer-commands" ,  QVariant(QStringList, ("Clean", "PrintSelfTestPage", "ReportLevels") ) ) ( "marker-change-time" ,  QVariant(int,
    //                 1267903160) ) ( "printer-state" ,  QVariant(int, 3) ) ( "printer-info" ,  QVariant(QString, "EPSON Stylus TX105") ) (
    //                 "printer-make-and-model" ,  QVariant(QString, "EPSON TX105 Series") ) )  )
    // Get destinations with these attributes
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterModel::getDestsFinished);
    request->getPrinters(m_attrs);
}

void PrinterModel::insertDest(int pos, const KCupsPrinter &printer)
{
    // Create the printer item
    auto stdItem = new QStandardItem(printer.name());
    stdItem->setData(printer.name(), DestName);
    stdItem->setIcon(printer.icon());
    // update the item
    updateDest(stdItem, printer);

    // insert the printer Item
    insertRow(pos, stdItem);
}

void PrinterModel::updateDest(QStandardItem *destItem, const KCupsPrinter &printer)
{
    // store if the printer is the network default
    bool isDefault = printer.isDefault();
    if (destItem->data(DestIsDefault).isNull() || isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    KCupsPrinter::Status state = printer.state();
    if (state != destItem->data(DestState)) {
        destItem->setData(state, DestState);
    }
    qCDebug(LIBKCUPS) << state << printer.name();

    // store if the printer is accepting jobs
    bool accepting = printer.isAcceptingJobs();
    if (accepting != destItem->data(DestIsAcceptingJobs)) {
        destItem->setData(accepting, DestIsAcceptingJobs);
    }

    // store the printer status message
    QString status = destStatus(state, printer.stateMsg(), accepting);
    if (status != destItem->data(DestStatus)) {
        destItem->setData(status, DestStatus);
    }

    bool paused = (state == KCupsPrinter::Stopped || !accepting);
    if (paused != destItem->data(DestIsPaused)) {
        destItem->setData(paused, DestIsPaused);
    }

    // store if the printer is shared
    bool shared = printer.isShared();
    if (shared != destItem->data(DestIsShared)) {
        destItem->setData(shared, DestIsShared);
    }

    // store if the printer is a class
    // the printer-type param is a flag
    bool isClass = printer.isClass();
    if (isClass != destItem->data(DestIsClass)) {
        destItem->setData(isClass, DestIsClass);
    }

    // store if the printer type
    // the printer-type param is a flag
    uint printerType = printer.type();
    if (printerType != destItem->data(DestType)) {
        destItem->setData(printerType, DestType);
        destItem->setData(printerType & CUPS_PRINTER_REMOTE, DestRemote);
    }

    // store the printer location
    QString location = printer.location();
    if (location.isEmpty() || location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    // store the printer icon name
    QString iconName = printer.iconName();
    if (iconName != destItem->data(DestIconName).toString()) {
        destItem->setData(iconName, DestIconName);
    }

    if (destItem->data(DestName).toString() != destItem->text()) {
        destItem->setText(destItem->data(DestName).toString());
    }

    // store the printer description
    QString description = printer.info();
    if (description.isEmpty() || description != destItem->data(DestDescription).toString()) {
        destItem->setData(description, DestDescription);
    }

    // store the printer kind
    QString kind = printer.makeAndModel();
    if (kind != destItem->data(DestKind)) {
        destItem->setData(kind, DestKind);
    }

    // store the printer commands
    QStringList commands = printer.commands();
    if (commands != destItem->data(DestCommands)) {
        destItem->setData(commands, DestCommands);
    }

    // store the printer URI
    QString uri = printer.deviceUri();
    if (uri != destItem->data(DestUri).toString()) {
        destItem->setData(uri, DestUri);
    }

    QString us = printer.uriSupported();
    if (us != destItem->data(DestUriSupported).toString()) {
        destItem->setData(us, DestUriSupported);
    }

    // printer member names for type=class
    const auto members = printer.memberNames();
    if (members != destItem->data(DestMemberNames)) {
        destItem->setData(members, DestMemberNames);
    }

    int markerChangeTime = printer.markerChangeTime();
    if (markerChangeTime != destItem->data(DestMarkerChangeTime)) {
        destItem->setData(printer.markerChangeTime(), DestMarkerChangeTime);
        const QVariantMap markers{{KCUPS_MARKER_CHANGE_TIME, printer.markerChangeTime()},
                                  {KCUPS_MARKER_COLORS, printer.argument(KCUPS_MARKER_COLORS)},
                                  {KCUPS_MARKER_LEVELS, printer.argument(KCUPS_MARKER_LEVELS)},
                                  {KCUPS_MARKER_NAMES, printer.argument(KCUPS_MARKER_NAMES)},
                                  {KCUPS_MARKER_TYPES, printer.argument(KCUPS_MARKER_TYPES)}};
        destItem->setData(markers, DestMarkers);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString()) {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrinterModel::destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const
{
    switch (state) {
    case KCupsPrinter::Idle:
        if (message.isEmpty()) {
            return isAcceptingJobs ? i18n("Idle") : i18n("Idle, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Idle - '%1'", message) : i18n("Idle, rejecting jobs - '%1'", message);
        }
    case KCupsPrinter::Printing:
        if (message.isEmpty()) {
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case KCupsPrinter::Stopped:
        if (message.isEmpty()) {
            return isAcceptingJobs ? i18n("Paused") : i18n("Paused, rejecting jobs");
        } else {
            return isAcceptingJobs ? i18n("Paused - '%1'", message) : i18n("Paused, rejecting jobs - '%1'", message);
        }
    default:
        if (message.isEmpty()) {
            return i18n("Unknown");
        } else {
            return i18n("Unknown - '%1'", message);
        }
    }
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PrinterModel::insertUpdatePrinterName(const QString &printerName)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterModel::insertUpdatePrinterFinished);
    // TODO how do we know if it's a class if this DBus signal
    // does not tell us
    request->getPrinterAttributes(printerName, false, m_attrs);
}

void PrinterModel::insertUpdatePrinter(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)

    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    insertUpdatePrinterName(printerName);
}

void PrinterModel::insertUpdatePrinterFinished(KCupsRequest *request)
{
    if (!request->hasError()) {
        const KCupsPrinters printers = request->printers();
        for (const KCupsPrinter &printer : printers) {
            // If there is a printer and it's not the current one add it
            // as a new destination
            int dest_row = destRow(printer.name());
            if (dest_row == -1) {
                // not found, insert new one
                insertDest(0, printer);
            } else {
                // update the printer
                updateDest(item(dest_row), printer);
            }
        }
    }
    setDisplayLocationHint();
    request->deleteLater();
}

void PrinterModel::printerRemovedName(const QString &printerName)
{
    qCDebug(LIBKCUPS) << printerName;

    // Look for the removed printer
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        removeRows(dest_row, 1);
    }
    setDisplayLocationHint();
}

void PrinterModel::printerRemoved(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    // REALLY? all these parameters just to say foo was deleted??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;

    // Look for the removed printer
    int dest_row = destRow(printerName);
    if (dest_row != -1) {
        removeRows(dest_row, 1);
    }
    setDisplayLocationHint();
}

void PrinterModel::printerStateChanged(const QString &text,
                                       const QString &printerUri,
                                       const QString &printerName,
                                       uint printerState,
                                       const QString &printerStateReasons,
                                       bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}
void PrinterModel::printerStopped(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerRestarted(const QString &text,
                                    const QString &printerUri,
                                    const QString &printerName,
                                    uint printerState,
                                    const QString &printerStateReasons,
                                    bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerShutdown(const QString &text,
                                   const QString &printerUri,
                                   const QString &printerName,
                                   uint printerState,
                                   const QString &printerStateReasons,
                                   bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
}

void PrinterModel::printerModified(const QString &text,
                                   const QString &printerUri,
                                   const QString &printerName,
                                   uint printerState,
                                   const QString &printerStateReasons,
                                   bool printerIsAcceptingJobs)
{
    qCDebug(LIBKCUPS) << text << printerUri << printerName << printerState << printerStateReasons << printerIsAcceptingJobs;
    setDisplayLocationHint();
}

void PrinterModel::serverChanged(const QString &text)
{
    qCDebug(LIBKCUPS) << text;
    update();
}

#include "moc_PrinterModel.cpp"
