/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2012 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsConnection.h"

#include "KCupsPasswordDialog.h"
#include "KIppRequest.h"
#include "kcupslib_log.h"

#include <config.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QMutexLocker>

#include <KLocalizedString>

#define RENEW_INTERVAL 3500
#define SUBSCRIPTION_DURATION 3600

#define DBUS_SERVER_RESTARTED "server-restarted" // ServerRestarted
#define DBUS_SERVER_STARTED "server-started" // ServerStarted
#define DBUS_SERVER_STOPPED "server-stopped" // ServerStopped
#define DBUS_SERVER_AUDIT "server-audit" // ServerAudit

#define DBUS_PRINTER_RESTARTED "printer-restarted" // PrinterRestarted
#define DBUS_PRINTER_SHUTDOWN "printer-shutdown" // PrinterShutdown
#define DBUS_PRINTER_STOPPED "printer-stopped" // PrinterStopped
#define DBUS_PRINTER_STATE_CHANGED "printer-state-changed" // PrinterStateChanged
#define DBUS_PRINTER_FINISHINGS_CHANGED "printer-finishings-changed" // PrinterFinishingsChanged
#define DBUS_PRINTER_MEDIA_CHANGED "printer-media-changed" // PrinterMediaChanged
#define DBUS_PRINTER_ADDED "printer-added" // PrinterAdded
#define DBUS_PRINTER_DELETED "printer-deleted" // PrinterDeleted
#define DBUS_PRINTER_MODIFIED "printer-modified" // PrinterModified

#define DBUS_JOB_STATE_CHANGED "job-state-changed" // JobState
#define DBUS_JOB_CREATED "job-created" // JobCreated
#define DBUS_JOB_COMPLETED "job-completed" // JobCompleted
#define DBUS_JOB_STOPPED "job-stopped" // JobStopped
#define DBUS_JOB_CONFIG_CHANGED "job-config-changed" // JobConfigChanged
#define DBUS_JOB_PROGRESS "job-progress" // JobProgress

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)

KCupsConnection *KCupsConnection::m_instance = nullptr;
static int password_retries = 0;
static int total_retries = 0;
static int internalErrorCount = 0;
const char *password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data);

KCupsConnection *KCupsConnection::global()
{
    if (!m_instance) {
        m_instance = new KCupsConnection(qApp);
    }

    return m_instance;
}

KCupsConnection::KCupsConnection(QObject *parent)
    : QThread(parent)
{
    init();
}

KCupsConnection::KCupsConnection(const QUrl &server, QObject *parent)
    : QThread(parent)
    , m_serverUrl(server)
{
    qRegisterMetaType<KIppRequest>("KIppRequest");
    init();
}

KCupsConnection::~KCupsConnection()
{
    if (m_instance == this) {
        m_instance = nullptr;
    }
    m_passwordDialog->deleteLater();

    quit();
    wait();

    delete m_renewTimer;
    delete m_subscriptionTimer;
}

void KCupsConnection::setPasswordMainWindow(WId mainwindow)
{
    m_passwordDialog->setMainWindow(mainwindow);
}

void KCupsConnection::init()
{
    // Creating the dialog before start() will make it run on the gui thread
    m_passwordDialog = new KCupsPasswordDialog;

    // setup the DBus subscriptions

    // Server related signals
    // ServerStarted
    notifierConnect(QLatin1String("ServerStarted"), this, SIGNAL(serverStarted(QString)));

    // ServerStopped
    notifierConnect(QLatin1String("ServerStopped"), this, SIGNAL(serverStopped(QString)));

    // ServerRestarted
    notifierConnect(QLatin1String("ServerRestarted"), this, SIGNAL(serverRestarted(QString)));

    // ServerAudit
    notifierConnect(QLatin1String("ServerAudit"), this, SIGNAL(serverAudit(QString)));

    // Printer related signals
    // PrinterAdded
    notifierConnect(QLatin1String("PrinterAdded"), this, SIGNAL(printerAdded(QString, QString, QString, uint, QString, bool)));

    // PrinterModified
    notifierConnect(QLatin1String("PrinterModified"), this, SIGNAL(printerModified(QString, QString, QString, uint, QString, bool)));

    // PrinterDeleted
    notifierConnect(QLatin1String("PrinterDeleted"), this, SIGNAL(printerDeleted(QString, QString, QString, uint, QString, bool)));

    // PrinterStateChanged
    notifierConnect(QLatin1String("PrinterStateChanged"), this, SIGNAL(printerStateChanged(QString, QString, QString, uint, QString, bool)));

    // PrinterStopped
    notifierConnect(QLatin1String("PrinterStopped"), this, SIGNAL(printerStopped(QString, QString, QString, uint, QString, bool)));

    // PrinterShutdown
    notifierConnect(QLatin1String("PrinterShutdown"), this, SIGNAL(printerShutdown(QString, QString, QString, uint, QString, bool)));

    // PrinterRestarted
    notifierConnect(QLatin1String("PrinterRestarted"), this, SIGNAL(printerRestarted(QString, QString, QString, uint, QString, bool)));

    // PrinterMediaChanged
    notifierConnect(QLatin1String("PrinterMediaChanged"), this, SIGNAL(printerMediaChanged(QString, QString, QString, uint, QString, bool)));

    // PrinterFinishingsChanged
    notifierConnect(QLatin1String("PrinterFinishingsChanged"), this, SIGNAL(PrinterFinishingsChanged(QString, QString, QString, uint, QString, bool)));

    // Job related signals
    // JobState
    notifierConnect(QLatin1String("JobState"), this, SIGNAL(jobState(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // JobCreated
    notifierConnect(QLatin1String("JobCreated"), this, SIGNAL(jobCreated(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // JobStopped
    notifierConnect(QLatin1String("JobStopped"), this, SIGNAL(jobStopped(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // JobConfigChanged
    notifierConnect(QLatin1String("JobConfigChanged"),
                    this,
                    SIGNAL(jobConfigChanged(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // JobProgress
    notifierConnect(QLatin1String("JobProgress"),
                    this,
                    SIGNAL(jobProgress(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // JobCompleted
    notifierConnect(QLatin1String("JobCompleted"),
                    this,
                    SIGNAL(jobCompleted(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)));

    // This signal is needed since the cups registration thing
    // doesn't Q_EMIT printerAdded when we add a printer class
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterAdded"),
                                         this,
                                         SIGNAL(rhPrinterAdded(QString)));

    // This signal is needed since the cups registration thing
    // sometimes simple stops working... don't ask me why
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("QueueChanged"),
                                         this,
                                         SIGNAL(rhQueueChanged(QString)));

    // This signal is needed since the cups registration thing
    // doesn't Q_EMIT printerRemoved when we add a printer class
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterRemoved"),
                                         this,
                                         SIGNAL(rhPrinterRemoved(QString)));

    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("JobQueuedLocal"),
                                         this,
                                         SIGNAL(rhJobQueuedLocal(QString, uint, QString)));

    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("JobStartedLocal"),
                                         this,
                                         SIGNAL(rhJobStartedLocal(QString, uint, QString)));

    // Creates the timer that will renew the DBus subscription
    m_renewTimer = new QTimer;
    m_renewTimer->setInterval(RENEW_INTERVAL * 1000);
    m_renewTimer->moveToThread(this);
    connect(m_renewTimer, &QTimer::timeout, this, static_cast<void (KCupsConnection::*)()>(&KCupsConnection::renewDBusSubscription), Qt::DirectConnection);

    // Creates the timer to merge updates on the DBus subscription
    m_subscriptionTimer = new QTimer;
    m_subscriptionTimer->setInterval(0);
    m_subscriptionTimer->setSingleShot(true);
    m_subscriptionTimer->moveToThread(this);
    connect(m_subscriptionTimer, &QTimer::timeout, this, &KCupsConnection::updateSubscription, Qt::DirectConnection);

    // Starts this thread
    start();
}

void KCupsConnection::run()
{
    // Check if we need an special connection
    if (!m_serverUrl.isEmpty()) {
        if (m_serverUrl.port() < 0) {
            // TODO find out if there's a better way of hardcoding
            // the CUPS port
            m_serverUrl.setPort(631);
        }

        cupsSetServer(qUtf8Printable(m_serverUrl.authority()));
    }

    // This is dead cool, cups will call the thread_password_cb()
    // function when a password set is needed, as we passed the
    // password dialog pointer the functions just need to call
    // it on a blocking mode.
    cupsSetPasswordCB2(password_cb, m_passwordDialog);

    m_inited = true;
    exec();

    // Event loop quit so cancelDBusSubscription()
    if (m_subscriptionId != -1) {
        cancelDBusSubscription();
    }
}

bool KCupsConnection::readyToStart()
{
    if (QThread::currentThread() == this) {
        password_retries = 0;
        total_retries = 0;
        internalErrorCount = 0;
        return true;
    }
    return false;
}

ReturnArguments KCupsConnection::request(const KIppRequest &request, ipp_tag_t groupTag) const
{
    ReturnArguments ret;
    ipp_t *response = nullptr;
    do {
        ippDelete(response);
        response = nullptr;

        response = request.sendIppRequest();
    } while (retry(qUtf8Printable(request.resource()), request.operation()));

    if (response && groupTag != IPP_TAG_ZERO) {
        ret = parseIPPVars(response, groupTag);
    }
    ippDelete(response);

    return ret;
}

int KCupsConnection::renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events)
{
    int ret = -1;

    ipp_op_t operation;

    // check if we have a valid subscription ID
    if (subscriptionId >= 0) {
        // Add the "notify-events" values to the request
        operation = IPP_RENEW_SUBSCRIPTION;
    } else {
        operation = IPP_CREATE_PRINTER_SUBSCRIPTION;
    }

    KIppRequest request(operation, QLatin1String("/"));
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, QLatin1String("/"));
    request.addInteger(IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, KCUPS_NOTIFY_LEASE_DURATION, leaseDuration);

    if (operation == IPP_CREATE_PRINTER_SUBSCRIPTION) {
        // Add the "notify-events" values to the request
        request.addStringList(IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, KCUPS_NOTIFY_EVENTS, events);
        request.addString(IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, KCUPS_NOTIFY_PULL_METHOD, QLatin1String("ippget"));
        request.addString(IPP_TAG_SUBSCRIPTION, IPP_TAG_URI, KCUPS_NOTIFY_RECIPIENT_URI, QLatin1String("dbus://"));
    } else {
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_NOTIFY_SUBSCRIPTION_ID, subscriptionId);
    }

    ipp_t *response = nullptr;
    do {
        // Do the request
        response = request.sendIppRequest();
    } while (retry("/", operation));

#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    if (response && ippGetStatusCode(response) == IPP_OK) {
#else
    if (response && response->request.status.status_code == IPP_OK) {
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
        ipp_attribute_t *attr;
        if (subscriptionId >= 0) {
            // Request was ok, just return the current subscription
            ret = subscriptionId;
        } else if ((attr = ippFindAttribute(response, "notify-subscription-id", IPP_TAG_INTEGER)) == nullptr) {
            qCWarning(LIBKCUPS) << "No notify-subscription-id in response!";
            ret = -1;
        } else {
#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
            ret = ippGetInteger(attr, 0);
        }
    } else if (subscriptionId >= 0 && response && ippGetStatusCode(response) == IPP_NOT_FOUND) {
        qCDebug(LIBKCUPS) << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#else
            ret = attr->values[0].integer;
        }
    } else if (subscriptionId >= 0 && response && response->request.status.status_code == IPP_NOT_FOUND) {
        qCDebug(LIBKCUPS) << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    } else {
        qCDebug(LIBKCUPS) << "Request failed" << cupsLastError() << httpGetStatus(CUPS_HTTP_DEFAULT);
        // When the server stops/restarts we will have some error so ignore it
        ret = subscriptionId;
    }

    ippDelete(response);

    return ret;
}

void KCupsConnection::notifierConnect(const QString &signal, QObject *receiver, const char *slot)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    systemBus.connect(QString(), QStringLiteral("/org/cups/cupsd/Notifier"), QStringLiteral("org.cups.cupsd.Notifier"), signal, receiver, slot);
}

void KCupsConnection::connectNotify(const QMetaMethod &signal)
{
    QMutexLocker locker(&m_mutex);
    QString event = eventForSignal(signal);
    if (!event.isNull()) {
        m_connectedEvents << event;
        QMetaObject::invokeMethod(m_subscriptionTimer, "start", Qt::QueuedConnection);
    }
}

void KCupsConnection::disconnectNotify(const QMetaMethod &signal)
{
    QMutexLocker locker(&m_mutex);
    QString event = eventForSignal(signal);
    if (!event.isNull()) {
        m_connectedEvents.removeOne(event);
        QMetaObject::invokeMethod(m_subscriptionTimer, "start", Qt::QueuedConnection);
    }
}

QString KCupsConnection::eventForSignal(const QMetaMethod &signal) const
{
    // Server signals
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::serverAudit)) {
        return QStringLiteral(DBUS_SERVER_AUDIT);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::serverStarted)) {
        return QStringLiteral(DBUS_SERVER_STARTED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::serverStopped)) {
        return QStringLiteral(DBUS_SERVER_STOPPED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::serverRestarted)) {
        return QStringLiteral(DBUS_SERVER_RESTARTED);
    }

    // Printer signals
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerAdded)) {
        return QStringLiteral(DBUS_PRINTER_ADDED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerDeleted)) {
        return QStringLiteral(DBUS_PRINTER_DELETED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerFinishingsChanged)) {
        return QStringLiteral(DBUS_PRINTER_FINISHINGS_CHANGED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerMediaChanged)) {
        return QStringLiteral(DBUS_PRINTER_MEDIA_CHANGED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerModified)) {
        return QStringLiteral(DBUS_PRINTER_MODIFIED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerRestarted)) {
        return QStringLiteral(DBUS_PRINTER_RESTARTED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerShutdown)) {
        return QStringLiteral(DBUS_PRINTER_SHUTDOWN);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerStateChanged)) {
        return QStringLiteral(DBUS_PRINTER_STATE_CHANGED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::printerStopped)) {
        return QStringLiteral(DBUS_PRINTER_STOPPED);
    }

    // job signals
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobCompleted)) {
        return QStringLiteral(DBUS_JOB_COMPLETED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobConfigChanged)) {
        return QStringLiteral(DBUS_JOB_CONFIG_CHANGED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobCreated)) {
        return QStringLiteral(DBUS_JOB_CREATED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobProgress)) {
        return QStringLiteral(DBUS_JOB_PROGRESS);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobState)) {
        return QStringLiteral(DBUS_JOB_STATE_CHANGED);
    }
    if (signal == QMetaMethod::fromSignal(&KCupsConnection::jobStopped)) {
        return QStringLiteral(DBUS_JOB_STOPPED);
    }

    // No registered event signal matched
    return QString();
}

void KCupsConnection::updateSubscription()
{
    QMutexLocker locker(&m_mutex);
    // Build the current list
    QStringList currentEvents = m_connectedEvents;
    currentEvents.sort();
    currentEvents.removeDuplicates();

    // Check if the requested events are already being asked
    if (m_requestedDBusEvents != currentEvents) {
        m_requestedDBusEvents = currentEvents;

        // If we already have a subscription lets cancel
        // and create a new one
        if (m_subscriptionId >= 0) {
            cancelDBusSubscription();
        }

        // Calculates the new events
        renewDBusSubscription();
    }
}

void KCupsConnection::renewDBusSubscription()
{
    // check if we have a valid subscription ID
    if (m_subscriptionId >= 0) {
        m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION);
    }

    // The above request might fail if the subscription was cancelled
    if (m_subscriptionId < 0) {
        if (m_requestedDBusEvents.isEmpty()) {
            m_renewTimer->stop();
        } else {
            m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION, m_requestedDBusEvents);
            m_renewTimer->start();
        }
    }
}

void KCupsConnection::cancelDBusSubscription()
{
    KIppRequest request(IPP_CANCEL_SUBSCRIPTION, QLatin1String("/"));
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, QLatin1String("/"));
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_NOTIFY_SUBSCRIPTION_ID, m_subscriptionId);

    do {
        // Do the request
        ippDelete(request.sendIppRequest());
    } while (retry(qUtf8Printable(request.resource()), request.operation()));

    // Reset the subscription id
    m_subscriptionId = -1;
}

ReturnArguments KCupsConnection::parseIPPVars(ipp_t *response, ipp_tag_t group_tag)
{
    ipp_attribute_t *attr;
    ReturnArguments ret;

#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    QVariantMap destAttributes;
    for (attr = ippFirstAttribute(response); attr != nullptr; attr = ippNextAttribute(response)) {
        // We hit an attribute separator
        if (ippGetName(attr) == nullptr) {
            ret << destAttributes;
            destAttributes.clear();
            continue;
        }

        // Skip leading attributes until we hit a group which can be a printer, job...
        if (ippGetGroupTag(attr) != group_tag
            || (ippGetValueTag(attr) != IPP_TAG_INTEGER && ippGetValueTag(attr) != IPP_TAG_ENUM && ippGetValueTag(attr) != IPP_TAG_BOOLEAN
                && ippGetValueTag(attr) != IPP_TAG_TEXT && ippGetValueTag(attr) != IPP_TAG_TEXTLANG && ippGetValueTag(attr) != IPP_TAG_LANGUAGE
                && ippGetValueTag(attr) != IPP_TAG_NAME && ippGetValueTag(attr) != IPP_TAG_NAMELANG && ippGetValueTag(attr) != IPP_TAG_KEYWORD
                && ippGetValueTag(attr) != IPP_TAG_RANGE && ippGetValueTag(attr) != IPP_TAG_URI)) {
            continue;
        }

        // Add a printer description attribute...
        destAttributes[QString::fromUtf8(ippGetName(attr))] = ippAttrToVariant(attr);
    }

    if (!destAttributes.isEmpty()) {
        ret << destAttributes;
    }
#else
    for (attr = response->attrs; attr != nullptr; attr = attr->next) {
        /*
         * Skip leading attributes until we hit a group which can be a printer, job...
         */
        while (attr && attr->group_tag != group_tag) {
            attr = attr->next;
        }

        if (attr == nullptr) {
            break;
        }

        /*
         * Pull the needed attributes from this printer...
         */
        QVariantMap destAttributes;
        for (; attr && attr->group_tag == group_tag; attr = attr->next) {
            if (attr->value_tag != IPP_TAG_INTEGER && attr->value_tag != IPP_TAG_ENUM && attr->value_tag != IPP_TAG_BOOLEAN && attr->value_tag != IPP_TAG_TEXT
                && attr->value_tag != IPP_TAG_TEXTLANG && attr->value_tag != IPP_TAG_LANGUAGE && attr->value_tag != IPP_TAG_NAME
                && attr->value_tag != IPP_TAG_NAMELANG && attr->value_tag != IPP_TAG_KEYWORD && attr->value_tag != IPP_TAG_RANGE
                && attr->value_tag != IPP_TAG_URI) {
                continue;
            }

            /*
             * Add a printer description attribute...
             */
            destAttributes[QString::fromUtf8(attr->name)] = ippAttrToVariant(attr);
        }

        ret << destAttributes;

        if (attr == nullptr) {
            break;
        }
    }
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)

    return ret;
}

QVariant KCupsConnection::ippAttrToVariant(ipp_attribute_t *attr)
{
    QVariant ret;
#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    switch (ippGetValueTag(attr)) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        if (ippGetCount(attr) == 1) {
            ret = ippGetInteger(attr, 0);
        } else {
            QList<int> values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << ippGetInteger(attr, i);
            }
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_BOOLEAN:
        if (ippGetCount(attr) == 1) {
            ret = ippGetBoolean(attr, 0);
        } else {
            QList<bool> values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << ippGetBoolean(attr, i);
            }
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_RANGE: {
        QVariantList values;
        for (int i = 0; i < ippGetCount(attr); ++i) {
            int rangeUpper;
            values << ippGetRange(attr, i, &rangeUpper);
            values << rangeUpper;
        }
        ret = values;
    } break;
    default:
        if (ippGetCount(attr) == 1) {
            ret = QString::fromUtf8(ippGetString(attr, 0, nullptr));
        } else {
            QStringList values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << QString::fromUtf8(ippGetString(attr, i, nullptr));
            }
            ret = values;
        }
    }
#else
    switch (attr->value_tag) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        if (attr->num_values == 1) {
            ret = attr->values[0].integer;
        } else {
            QList<int> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << attr->values[i].integer;
            }
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_BOOLEAN:
        if (attr->num_values == 1) {
            ret = static_cast<bool>(attr->values[0].integer);
        } else {
            QList<bool> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << static_cast<bool>(attr->values[i].integer);
            }
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_RANGE: {
        QVariantList values;
        for (int i = 0; i < attr->num_values; ++i) {
            values << attr->values[i].range.lower;
            values << attr->values[i].range.upper;
        }
        ret = values;
    } break;
    default:
        if (attr->num_values == 1) {
            ret = QString::fromUtf8(attr->values[0].string.text);
        } else {
            QStringList values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << QString::fromUtf8(attr->values[i].string.text);
            }
            ret = values;
        }
    }
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    return ret;
}

bool KCupsConnection::retry(const char *resource, int operation) const
{
    ipp_status_t status = cupsLastError();

    if (operation != -1) {
        qCDebug(LIBKCUPS) << ippOpString(static_cast<ipp_op_t>(operation)) << "last error:" << status << cupsLastErrorString();
    } else {
        qCDebug(LIBKCUPS) << operation << "last error:" << status << cupsLastErrorString();
    }

    // When CUPS process stops our connection
    // with it fails and has to be re-established
    if (status == IPP_INTERNAL_ERROR) {
        // Deleting this connection thread forces it
        // to create a new CUPS connection
        qCWarning(LIBKCUPS) << "IPP_INTERNAL_ERROR: clearing cookies and reconnecting";

        // Reconnect to CUPS
        int cancel = 0;
        if (httpReconnect2(CUPS_HTTP_DEFAULT, 10000, &cancel)) {
            qCWarning(LIBKCUPS) << "Failed to reconnect" << cupsLastErrorString();
        }

        // Try the request again
        return ++internalErrorCount < 3;
    }

    total_retries++;

    if (total_retries > (password_retries + 3)) {
        // Something is wrong.
        // This will happen if the password_cb function is not called,
        // which will for example be the case if the server has
        // an IP blacklist and thus always return 403.
        // In this case, there is nothing we can do.
        return false;
    }

    bool forceAuth = false;
    // If our user is forbidden to perform the
    // task we try again using the root user
    // ONLY if it was the first time
    if (status == IPP_FORBIDDEN && password_retries == 0) {
        // Pretend to be the root user
        // Sometimes setting this just works
        cupsSetUser("root");

        // force authentication
        forceAuth = true;
    }

    if (status == IPP_NOT_AUTHORIZED || status == IPP_NOT_AUTHENTICATED) {
        if (password_retries > 3 || password_retries == -1) {
            // the authentication failed 3 times
            // OR the dialog was canceled (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            total_retries = 0;
            return false;
        }

        // force authentication
        forceAuth = true;
    }

    if (forceAuth) {
        // force authentication
        int ret = cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", resource);
        qCDebug(LIBKCUPS) << "cupsDoAuthentication, return:" << ret << "password_retries:" << password_retries;

        // If the authentication was successful
        // sometimes just trying to be root works
        return ret != 0;
    }

    // the action was not forbidden
    return false;
}

const char *password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data)
{
    Q_UNUSED(http)
    Q_UNUSED(method)
    Q_UNUSED(resource)

    if (++password_retries > 3) {
        // cancel the authentication
        cupsSetUser(nullptr);
        return nullptr;
    }

    auto passwordDialog = static_cast<KCupsPasswordDialog *>(user_data);
    bool wrongPassword = password_retries > 1;

    // use prompt text from CUPS callback for dialog
    passwordDialog->setPromptText(i18n("A CUPS connection requires authentication: \"%1\"", QString::fromUtf8(prompt)));

    // This will block this thread until exec is not finished
    QMetaObject::invokeMethod(passwordDialog, "exec", Qt::BlockingQueuedConnection, Q_ARG(QString, QString::fromUtf8(cupsUser())), Q_ARG(bool, wrongPassword));

    // The password dialog has just returned check the result
    // method that returns QDialog enums
    if (passwordDialog->accepted()) {
        cupsSetUser(qUtf8Printable(passwordDialog->username()));
        return qUtf8Printable(passwordDialog->password());
    } else {
        // the dialog was canceled
        password_retries = -1;
        cupsSetUser(nullptr);
        return nullptr;
    }
}

#include "moc_KCupsConnection.cpp"
