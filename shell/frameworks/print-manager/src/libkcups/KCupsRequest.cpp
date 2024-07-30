/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsRequest.h"

#include "kcupslib_log.h"

#include <KLocalizedString>

#include <cups/adminutil.h>
#include <cups/ppd.h>

#define CUPS_DATADIR QLatin1String("/usr/share/cups")

KCupsRequest::KCupsRequest(KCupsConnection *connection)
    : m_connection(connection)
{
    // If no connection was specified use default one
    if (m_connection == nullptr) {
        m_connection = KCupsConnection::global();
    }
    connect(this, &KCupsRequest::finished, &m_loop, &QEventLoop::quit);
}

QString KCupsRequest::serverError() const
{
    switch (error()) {
    case IPP_SERVICE_UNAVAILABLE:
        return i18n("Print service is unavailable");
    case IPP_NOT_FOUND:
        return i18n("Not found");
    default: // In this case we don't want to map all enums
        qCWarning(LIBKCUPS) << "status unrecognised: " << error();
        return QString::fromUtf8(ippErrorString(error()));
    }
}

void KCupsRequest::getPPDS(const QString &make)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(CUPS_GET_PPDS, QLatin1String("/"));
        if (!make.isEmpty()) {
            request.addString(IPP_TAG_PRINTER, IPP_TAG_TEXT, KCUPS_PPD_MAKE_AND_MODEL, make);
        }

        m_ppds = m_connection->request(request, IPP_TAG_PRINTER);

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPPDS", make);
    }
}

static void choose_device_cb(const char *device_class, /* I - Class */
                             const char *device_id, /* I - 1284 device ID */
                             const char *device_info, /* I - Description */
                             const char *device_make_and_model, /* I - Make and model */
                             const char *device_uri, /* I - Device URI */
                             const char *device_location, /* I - Location */
                             void *user_data) /* I - Result object */
{
    /*
     * Add the device to the array...
     */
    auto request = static_cast<KCupsRequest *>(user_data);
    QMetaObject::invokeMethod(request,
                              "device",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromUtf8(device_class)),
                              Q_ARG(QString, QString::fromUtf8(device_id)),
                              Q_ARG(QString, QString::fromUtf8(device_info)),
                              Q_ARG(QString, QString::fromUtf8(device_make_and_model)),
                              Q_ARG(QString, QString::fromUtf8(device_uri)),
                              Q_ARG(QString, QString::fromUtf8(device_location)));
}

void KCupsRequest::getDevices(int timeout)
{
    getDevices(timeout, QStringList(), QStringList());
}

void KCupsRequest::getDevices(int timeout, QStringList includeSchemes, QStringList excludeSchemes)
{
    if (m_connection->readyToStart()) {
        do {
            const char *include;
            if (includeSchemes.isEmpty()) {
                include = CUPS_INCLUDE_ALL;
            } else {
                include = qUtf8Printable(includeSchemes.join(QLatin1String(",")));
            }

            const char *exclude;
            if (excludeSchemes.isEmpty()) {
                exclude = CUPS_EXCLUDE_NONE;
            } else {
                exclude = qUtf8Printable(excludeSchemes.join(QLatin1String(",")));
            }

            // Scan for devices for "timeout" seconds
            cupsGetDevices(CUPS_HTTP_DEFAULT, timeout, include, exclude, (cups_device_cb_t)choose_device_cb, this);
        } while (m_connection->retry("/admin/", CUPS_GET_DEVICES));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished(true);
    } else {
        invokeMethod("getDevices", timeout, includeSchemes, excludeSchemes);
    }
}

// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details

void KCupsRequest::getPrinters(QStringList attributes, int mask)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(CUPS_GET_PRINTERS, QLatin1String("/"));
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL);
        if (!attributes.isEmpty()) {
            request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);
        }
        if (mask != -1) {
            request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE_MASK, mask);
        }

        const ReturnArguments ret = m_connection->request(request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            m_printers << KCupsPrinter(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", QVariant::fromValue(attributes), mask);
    }
}

void KCupsRequest::getPrinterAttributes(const QString &printerName, bool isClass, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_GET_PRINTER_ATTRIBUTES, QLatin1String("/"));

        request.addPrinterUri(printerName, isClass);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, QLatin1String(KCUPS_PRINTER_TYPE), CUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, QLatin1String(KCUPS_REQUESTED_ATTRIBUTES), attributes);

        const ReturnArguments ret = m_connection->request(request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            // Inject the printer name back to the arguments hash
            QVariantMap args = arguments;
            args[KCUPS_PRINTER_NAME] = printerName;
            m_printers << KCupsPrinter(args);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterAttributes", printerName, isClass, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getJobs(const QString &printerName, bool myJobs, int whichJobs, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_GET_JOBS, QLatin1String("/"));

        // printer-uri makes the Name of the Job and owner came blank lol
        request.addPrinterUri(printerName, false);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);

        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_MY_JOBS, myJobs);

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request.addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_WHICH_JOBS, QLatin1String("completed"));
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request.addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_WHICH_JOBS, QLatin1String("all"));
        }

        const ReturnArguments ret = m_connection->request(request, IPP_TAG_JOB);

        for (const QVariantMap &arguments : ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printerName, myJobs, whichJobs, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getJobAttributes(int jobId, const QString &printerUri, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_GET_JOB_ATTRIBUTES, QLatin1String("/"));

        request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, printerUri);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, CUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);

        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

        const ReturnArguments ret = m_connection->request(request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobAttributes", jobId, printerUri, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getServerSettings()
{
    if (m_connection->readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            QVariantMap arguments;
            int ret = cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
            for (int i = 0; i < num_settings; ++i) {
                QString name = QString::fromUtf8(settings[i].name);
                QString value = QString::fromUtf8(settings[i].value);
                arguments[name] = value;
            }
            cupsFreeOptions(num_settings, settings);
            if (ret) {
                setError(HTTP_OK, IPP_OK, QString());
            } else {
                setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            }

            m_server = KCupsServer(arguments);
        } while (m_connection->retry("/admin/", -1));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}

void KCupsRequest::getPrinterPPD(const QString &printerName)
{
    if (m_connection->readyToStart()) {
        do {
            const char *filename;
            filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName));
            qCDebug(LIBKCUPS) << filename;
            m_ppdFile = QString::fromUtf8(filename);
            qCDebug(LIBKCUPS) << m_ppdFile;
        } while (m_connection->retry("/", CUPS_GET_PPD));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterPPD", printerName);
    }
}

void KCupsRequest::setServerSettings(const KCupsServer &server)
{
    if (m_connection->readyToStart()) {
        do {
            QVariantMap args = server.arguments();
            int num_settings = 0;
            cups_option_t *settings;

            QVariantMap::const_iterator i = args.constBegin();
            while (i != args.constEnd()) {
                num_settings = cupsAddOption(qUtf8Printable(i.key()), qUtf8Printable(i.value().toString()), num_settings, &settings);
                ++i;
            }

            cupsAdminSetServerSettings(CUPS_HTTP_DEFAULT, num_settings, settings);
            cupsFreeOptions(num_settings, settings);
        } while (m_connection->retry("/admin/", -1));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", QVariant::fromValue(server));
    }
}

void KCupsRequest::addOrModifyPrinter(const QString &printerName, const QVariantMap &attributes, const QString &filename)
{
    KIppRequest request(CUPS_ADD_MODIFY_PRINTER, QLatin1String("/admin/"), filename);
    request.addPrinterUri(printerName);
    request.addVariantValues(attributes);

    process(request);
}

void KCupsRequest::addOrModifyClass(const QString &printerName, const QVariantMap &attributes)
{
    KIppRequest request(CUPS_ADD_MODIFY_CLASS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName, true);
    request.addVariantValues(attributes);

    process(request);
}

void KCupsRequest::setShared(const QString &printerName, bool isClass, bool shared)
{
    KIppRequest request(isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName, isClass);
    request.addBoolean(IPP_TAG_OPERATION, KCUPS_PRINTER_IS_SHARED, shared);

    process(request);
}

void KCupsRequest::pausePrinter(const QString &printerName)
{
    KIppRequest request(IPP_PAUSE_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::resumePrinter(const QString &printerName)
{
    KIppRequest request(IPP_RESUME_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::rejectJobs(const QString &printerName)
{
    KIppRequest request(CUPS_REJECT_JOBS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::acceptJobs(const QString &printerName)
{
    KIppRequest request(CUPS_ACCEPT_JOBS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::setDefaultPrinter(const QString &printerName)
{
    KIppRequest request(CUPS_SET_DEFAULT, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::deletePrinter(const QString &printerName)
{
    KIppRequest request(CUPS_DELETE_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::printTestPage(const QString &printerName, bool isClass)
{
    QString resource; /* POST resource path */
    QString filename; /* Test page filename */
    QString datadir; /* CUPS_DATADIR env var */

    /*
     * Locate the test page file...
     */
    datadir = QString::fromUtf8(qgetenv("CUPS_DATADIR"));
    if (datadir.isEmpty()) {
        datadir = CUPS_DATADIR;
    }
    filename = datadir % QLatin1String("/data/testprint");

    /*
     * Point to the printer/class...
     */
    if (isClass) {
        resource = QLatin1String("/classes/") + printerName;
    } else {
        resource = QLatin1String("/printers/") + printerName;
    }

    KIppRequest request(IPP_PRINT_JOB, resource, filename);
    request.addPrinterUri(printerName);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_NAME, KCUPS_JOB_NAME, i18n("Test Page"));

    process(request);
}

void KCupsRequest::printCommand(const QString &printerName, const QString &command, const QString &title)
{
    if (m_connection->readyToStart()) {
        do {
            int job_id; /* Command file job */
            char command_file[1024]; /* Command "file" */
            http_status_t status; /* Document status */
            cups_option_t hold_option; /* job-hold-until option */

            /*
             * Create the CUPS command file...
             */
            snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command.toUtf8().constData());

            /*
             * Send the command file job...
             */
            hold_option.name = const_cast<char *>("job-hold-until");
            hold_option.value = const_cast<char *>("no-hold");

            if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName), qUtf8Printable(title), 1, &hold_option)) < 1) {
                qCWarning(LIBKCUPS) << "Unable to send command to printer driver!";

                setError(HTTP_OK, IPP_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName), job_id, nullptr, CUPS_FORMAT_COMMAND, 1);
            if (status == HTTP_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file, strlen(command_file));
            }

            if (status == HTTP_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName));
            }

            setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            if (httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError() >= IPP_REDIRECTION_OTHER_SITE) {
                qCWarning(LIBKCUPS) << "Unable to send command to printer driver!";

                cupsCancelJob(qUtf8Printable(printerName), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (m_connection->retry("/", IPP_CREATE_JOB));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", printerName, command, title);
    }
}

void KCupsRequest::cancelJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_CANCEL_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::holdJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_HOLD_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::releaseJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_RELEASE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::restartJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_RESTART_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::moveJob(const QString &fromPrinterName, int jobId, const QString &toPrinterName)
{
    if (jobId < -1 || fromPrinterName.isEmpty() || toPrinterName.isEmpty() || jobId == 0) {
        qCWarning(LIBKCUPS) << "Internal error, invalid input data" << jobId << fromPrinterName << toPrinterName;
        setFinished();
        return;
    }

    KIppRequest request(CUPS_MOVE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(fromPrinterName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    QString toPrinterUri = KIppRequest::assembleUrif(toPrinterName, false);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_JOB_PRINTER_URI, toPrinterUri);

    process(request);
}

void KCupsRequest::authenticateJob(const QString &printerName, const QStringList authInfo, int jobId)
{
    KIppRequest request(IPP_OP_CUPS_AUTHENTICATE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);
    request.addStringList(IPP_TAG_OPERATION, IPP_TAG_TEXT, KCUPS_AUTH_INFO, authInfo);

    process(request);
}

void KCupsRequest::invokeMethod(const char *method,
                                const QVariant &arg1,
                                const QVariant &arg2,
                                const QVariant &arg3,
                                const QVariant &arg4,
                                const QVariant &arg5,
                                const QVariant &arg6,
                                const QVariant &arg7,
                                const QVariant &arg8)
{
    m_error = IPP_OK;
    m_errorMsg.clear();
    m_printers.clear();
    m_jobs.clear();
    m_ppds.clear();
    m_ppdFile.clear();

    // If this fails we get into a infinite loop
    // Do not use global()->thread() which point
    // to the KCupsConnection parent thread
    moveToThread(m_connection);

    m_finished = !QMetaObject::invokeMethod(this,
                                            method,
                                            Qt::QueuedConnection,
                                            QGenericArgument(arg1.typeName(), arg1.data()),
                                            QGenericArgument(arg2.typeName(), arg2.data()),
                                            QGenericArgument(arg3.typeName(), arg3.data()),
                                            QGenericArgument(arg4.typeName(), arg4.data()),
                                            QGenericArgument(arg5.typeName(), arg5.data()),
                                            QGenericArgument(arg6.typeName(), arg6.data()),
                                            QGenericArgument(arg7.typeName(), arg7.data()),
                                            QGenericArgument(arg8.typeName(), arg8.data()));
    if (m_finished) {
        setError(HTTP_ERROR, IPP_BAD_REQUEST, i18n("Failed to invoke method: %1", QLatin1String(method)));
        setFinished();
    }
}

void KCupsRequest::process(const KIppRequest &request)
{
    if (m_connection->readyToStart()) {
        m_connection->request(request);

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("process", QVariant::fromValue(request));
    }
}

ReturnArguments KCupsRequest::ppds() const
{
    return m_ppds;
}

KCupsServer KCupsRequest::serverSettings() const
{
    return m_server;
}

QString KCupsRequest::printerPPD() const
{
    return m_ppdFile;
}

KCupsPrinters KCupsRequest::printers() const
{
    return m_printers;
}

KCupsJobs KCupsRequest::jobs() const
{
    return m_jobs;
}

void KCupsRequest::waitTillFinished()
{
    if (m_finished) {
        return;
    }

    m_loop.exec();
}

bool KCupsRequest::hasError() const
{
    return m_error;
}

ipp_status_t KCupsRequest::error() const
{
    return m_error;
}

http_status_t KCupsRequest::httpStatus() const
{
    return m_httpStatus;
}

QString KCupsRequest::errorMsg() const
{
    return m_errorMsg;
}

KCupsConnection *KCupsRequest::connection() const
{
    return m_connection;
}

void KCupsRequest::setError(http_status_t httpStatus, ipp_status_t error, const QString &errorMsg)
{
    m_httpStatus = httpStatus;
    m_error = error;
    m_errorMsg = errorMsg;
}

void KCupsRequest::setFinished(bool delayed)
{
    m_finished = true;
    if (delayed) {
        QTimer::singleShot(0, this, [this]() {
            Q_EMIT finished(this);
        });
    } else {
        Q_EMIT finished(this);
    }
}

#include "moc_KCupsRequest.cpp"
