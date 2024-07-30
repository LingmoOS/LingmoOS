/**
 * SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "printermanager.h"
#include "pmkcm_log.h"

#ifdef SCP_INSTALL
#include "scpinstaller.h"
#endif

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QStringDecoder>

#include <KLocalizedString>
#include <KOSRelease>

#include <KCupsRequest.h>
#include <PPDModel.h>
#include <cups/adminutil.h>
#include <cups/ppd.h>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(PrinterManager, "kcm_printer_manager.json")

QDBusArgument &operator<<(QDBusArgument &argument, const DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument << driverMatch.ppd << driverMatch.match;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument >> driverMatch.ppd >> driverMatch.match;
    argument.endStructure();
    return argument;
}

PrinterManager::PrinterManager(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData)
    , m_serverSettings({{QLatin1String(CUPS_SERVER_USER_CANCEL_ANY), false},
                        {QLatin1String(CUPS_SERVER_SHARE_PRINTERS), false},
                        {QLatin1String(CUPS_SERVER_REMOTE_ANY), false},
                        {QLatin1String(CUPS_SERVER_REMOTE_ADMIN), false}})
{
    setButtons(KQuickConfigModule::NoAdditionalButton);
    initOSRelease();

    // Make sure we update our server settings if the user changes anything on
    // another interface
    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, [](const QString &msg) {
        qCDebug(PMKCM) << "CUPS SERVER AUDIT" << msg;
    });
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, [this](const QString &msg) {
        qCDebug(PMKCM) << "CUPS SERVER STARTED" << msg;
        Q_EMIT serverStarted();
    });
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, [this](const QString &msg) {
        qCDebug(PMKCM) << "CUPS SERVER STOPPED" << msg;
        m_serverSettingsLoaded = false;
        Q_EMIT serverStopped();
    });
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, [this](const QString &msg) {
        qCDebug(PMKCM) << "CUPS SERVER RE-STARTED" << msg;
        Q_EMIT serverStarted();
    });

    qmlRegisterUncreatableMetaObject(PMTypes::staticMetaObject,
                                     "org.kde.plasma.printmanager", // use same namespace as kcupslib
                                     1,
                                     0,
                                     "PPDType", // QML qualifier
                                     u"Error: for only enums"_s);

    qDBusRegisterMetaType<DriverMatch>();
    qDBusRegisterMetaType<DriverMatchList>();

#ifdef SCP_INSTALL
    qmlRegisterType<SCPInstaller>("org.kde.plasma.printmanager", 1, 0, "SCPInstaller");
#endif
}

void PrinterManager::initOSRelease()
{
    KOSRelease os;
    m_osName = os.name();
    m_osBugReportUrl = os.bugReportUrl();
}

QString PrinterManager::osName() const
{
    return m_osName;
}

QString PrinterManager::osBugReportUrl() const
{
    return m_osBugReportUrl;
}

void PrinterManager::getRemotePrinters(const QString &uri, const QString &uriScheme)
{
    QUrl url(QUrl::fromUserInput(uri));
    if (url.host().isEmpty() && !uri.contains(u"://"_s)) {
        url = QUrl();
        // URI might be scsi, network or anything that didn't match before
        if (uriScheme != u"other"_s) {
            url.setScheme(uriScheme);
        }
        url.setAuthority(uri);
    }

    qCDebug(PMKCM) << "Finding Printers for URL:" << url.toDisplayString() << uriScheme;

    m_remotePrinters.clear();
    auto conn = new KCupsConnection(url, this);
    auto request = new KCupsRequest(conn);

    request->getPrinters({KCUPS_DEVICE_URI,
                          KCUPS_PRINTER_NAME,
                          KCUPS_PRINTER_STATE,
                          KCUPS_PRINTER_IS_SHARED,
                          KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                          KCUPS_PRINTER_TYPE,
                          KCUPS_PRINTER_LOCATION,
                          KCUPS_PRINTER_INFO,
                          KCUPS_PRINTER_MAKE_AND_MODEL});

    request->waitTillFinished();

    if (request) {
        const auto printers = request->printers();
        if (request->hasError()) {
            Q_EMIT requestError(request->errorMsg());
        } else {
            for (const auto &p : printers) {
                const auto mm = p.makeAndModel();
                const auto make = mm.split(u" "_s).at(0);
                m_remotePrinters.append(QVariantMap({{KCUPS_DEVICE_URI, p.deviceUri()},
                                                     {u"printer-is-class"_s, p.isClass()},
                                                     {u"iconName"_s, p.iconName()},
                                                     {u"remote"_s, QVariant::fromValue<bool>(p.type() & CUPS_PRINTER_REMOTE)},
                                                     {KCUPS_PRINTER_NAME, p.name()},
                                                     {KCUPS_PRINTER_STATE, p.state()},
                                                     {KCUPS_PRINTER_IS_SHARED, p.isShared()},
                                                     {KCUPS_PRINTER_IS_ACCEPTING_JOBS, p.isAcceptingJobs()},
                                                     {KCUPS_PRINTER_TYPE, p.type()},
                                                     {KCUPS_PRINTER_LOCATION, p.location()},
                                                     {KCUPS_PRINTER_INFO, p.info()},
                                                     {u"printer-make"_s, make},
                                                     {KCUPS_PRINTER_MAKE_AND_MODEL, mm}}));
            }
            Q_EMIT remotePrintersLoaded();
        }

        request->deleteLater();
    }

    conn->deleteLater();
}

void PrinterManager::clearRemotePrinters()
{
    m_remotePrinters.clear();
}

void PrinterManager::clearRecommendedDrivers()
{
    m_recommendedDrivers.clear();
}

bool PrinterManager::isSCPAvailable()
{
    // The service is activatable, so check the list
    const auto conn = QDBusConnection::sessionBus().interface();
    const auto list = conn->activatableServiceNames();
    return list.value().contains("org.fedoraproject.Config.Printing"_L1);
}

void PrinterManager::savePrinter(const QString &name, const QVariantMap &saveArgs, bool isClass)
{
    QVariantMap args = saveArgs;
    QString fileName;

    if (args.contains(u"ppd-type"_s)) {
        const auto ppdType = args.take(u"ppd-type"_s).toInt();
        if (ppdType == PMTypes::Manual) {
            // local file, remove key, use filename
            fileName = args.take(u"ppd-name"_s).toString();
        }
    }

    const bool addMode = args.take(u"add"_s).toBool();
    // Will only be set if default is changed to true
    const bool isDefault = args.take(u"isDefault"_s).toBool();

    if (addMode) {
        args[KCUPS_PRINTER_STATE] = IPP_PRINTER_IDLE;
    }

    qCDebug(PMKCM) << (addMode ? "New Printer:" : "Change Printer:") << name << "isClass?" << isClass << "Changing Default?" << isDefault << "filename"
                   << fileName << Qt::endl
                   << "args" << args;

    QPointer<KCupsRequest> request = new KCupsRequest;

    /** Default printer is handled by CUPS independently of the other printer
     * attributes. If changing an existing printer and "default" is set
     * then save immediately
     */
    if (!addMode && isDefault) {
        request->setDefaultPrinter(name);
        request->waitTillFinished();
    }

    if (isClass) {
        // Member list is a QVariantList, kcupslib wants to see
        // a QStringList
        const auto list = args.take(KCUPS_MEMBER_URIS);
        if (!list.value<QVariantList>().empty()) {
            args.insert(KCUPS_MEMBER_URIS, list.toStringList());
        }
        if (!args.isEmpty()) {
            request->addOrModifyClass(name, args);
            request->waitTillFinished();
        }
    } else {
        if (!args.isEmpty() || !fileName.isEmpty()) {
            request->addOrModifyPrinter(name, args, fileName);
            request->waitTillFinished();
        }
    }

    // If adding a new printer and it's set as default, save explicitly
    if (addMode && isDefault) {
        request->setDefaultPrinter(name);
        request->waitTillFinished();
    }

    if (request) {
        if (!request->hasError()) {
            Q_EMIT saveDone();
        } else {
            Q_EMIT requestError((isClass ? i18nc("@info", "Failed to configure class: ") : i18nc("@info", "Failed to configure printer: "))
                                + request->errorMsg());
            qCWarning(PMKCM) << "Failed to save printer/class" << name << request->errorMsg();
        }
        request->deleteLater();
    }
}

QVariantMap PrinterManager::getPrinterPPD(const QString &name)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->getPrinterPPD(name);
    request->waitTillFinished();
    if (!request) {
        return {};
    }

    const auto filename = request->printerPPD();
    const auto err = request->errorMsg();
    request->deleteLater();

    ppd_file_t *ppd = nullptr;
    if (!filename.isEmpty()) {
        ppd = ppdOpenFile(qUtf8Printable(filename));
        unlink(qUtf8Printable(filename));
    }

    if (ppd == nullptr) {
        qCWarning(PMKCM) << "Could not open ppd file:" << filename << err;
        return {};
    }

    ppdLocalize(ppd);
    // select the default options on the ppd file
    ppdMarkDefaults(ppd);

    const char *lang_encoding;
    lang_encoding = ppd->lang_encoding;
    QStringDecoder codec;

    if (lang_encoding && !strcasecmp(lang_encoding, "UTF-8")) {
        codec = QStringDecoder(QStringDecoder::Utf8);
    } else if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin1")) {
        codec = QStringDecoder(QStringDecoder::Latin1);
    } else if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin2")) {
        codec = QStringDecoder("ISO-8859-2");
    } else if (lang_encoding && !strcasecmp(lang_encoding, "ISOLatin5")) {
        codec = QStringDecoder("ISO-8859-5");
    } else if (lang_encoding && !strcasecmp(lang_encoding, "JIS83-RKSJ")) {
        codec = QStringDecoder("SHIFT-JIS");
    } else if (lang_encoding && !strcasecmp(lang_encoding, "MacStandard")) {
        codec = QStringDecoder("MACINTOSH");
    } else if (lang_encoding && !strcasecmp(lang_encoding, "WindowsANSI")) {
        codec = QStringDecoder("WINDOWS-1252");
    } else {
        qCWarning(PMKCM) << "Unknown ENCODING:" << lang_encoding;
        codec = QStringDecoder(lang_encoding);
    }
    // Fallback
    if (!codec.isValid()) {
        codec = QStringDecoder(QStringDecoder::Utf8);
    }

    qCDebug(PMKCM) << codec(ppd->pcfilename) << codec(ppd->modelname) << codec(ppd->shortnickname);

    QString make, makeAndModel, file;
    if (ppd->manufacturer) {
        make = codec(ppd->manufacturer);
    }

    if (ppd->nickname) {
        makeAndModel = codec(ppd->nickname);
    }

    if (ppd->pcfilename) {
        file = codec(ppd->pcfilename);
    }

    ppd_attr_t *ppdattr;
    bool autoConfig = false;
    if (ppd->num_filters == 0
        || ((ppdattr = ppdFindAttr(ppd, "cupsCommands", nullptr)) != nullptr && ppdattr->value && strstr(ppdattr->value, "AutoConfigure"))) {
        autoConfig = true;
    } else {
        for (int i = 0; i < ppd->num_filters; ++i) {
            if (!strncmp(ppd->filters[i], "application/vnd.cups-postscript", 31)) {
                autoConfig = true;
                break;
            }
        }
    }

    return {{u"autoConfig"_s, autoConfig},
            {u"file"_s, QString()},
            {u"pcfile"_s, file},
            {u"type"_s, QVariant(PMTypes::PPDType::Custom)},
            {u"make"_s, make},
            {u"makeModel"_s, makeAndModel}};
}

void PrinterManager::pausePrinter(const QString &name)
{
    const auto request = setupRequest();
    request->pausePrinter(name);
}

void PrinterManager::resumePrinter(const QString &name)
{
    const auto request = setupRequest();
    request->resumePrinter(name);
}

bool PrinterManager::isIPPCapable(const QString &uri)
{
    /**
     * Per CUPS
     * "dnssd:" URIs with "._ipp._tcp" or "._ipps._tcp"
     *  or "ipp:" or "ipps:" URIs can be of a driverless printer
    */
    if (uri.startsWith(u"dnssd:"_s, Qt::CaseInsensitive)
        && (uri.contains(u"._ipp._tcp"_s, Qt::CaseInsensitive)
            || uri.contains(u"._ipps._tcp"_s, Qt::CaseInsensitive))) {
        return true;
    }

    if (uri.startsWith(u"ipp:"_s, Qt::CaseInsensitive)
            || uri.startsWith(u"ipps:"_s, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

void PrinterManager::getRecommendedDrivers(const QString &deviceId, const QString &makeAndModel, const QString &deviceUri)
{
    qCDebug(PMKCM) << "getRecommendedDrivers for:" << makeAndModel << deviceUri << deviceId;

    m_recommendedDrivers.clear();

    auto call = QDBusMessage::createMethodCall(u"org.fedoraproject.Config.Printing"_s, u"/org/fedoraproject/Config/Printing"_s, u"org.fedoraproject.Config.Printing"_s, u"GetBestDrivers"_s);
    call.setArguments({deviceId, makeAndModel, deviceUri});
    const auto pending = QDBusConnection::sessionBus().asyncCall(call);
    const auto watcher = new QDBusPendingCallWatcher(pending, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<DriverMatchList> reply(*w);
        if (reply.isError()) {
            qCWarning(PMKCM) << "Failed to get best drivers" << reply.error().message();
        } else {
            const auto dml = reply.value();
            for (const auto &driverMatch : dml) {
                if (driverMatch.match == u"none"_s) {
                    continue;
                }
                m_recommendedDrivers.append(QVariantMap({{u"match"_s, driverMatch.match}, {u"ppd-name"_s, driverMatch.ppd}, {u"ppd-type"_s, PMTypes::Auto}}));
            }
        }
        Q_EMIT recommendedDriversLoaded();
        w->deleteLater();
    });
}

KCupsRequest *PrinterManager::setupRequest(std::function<void()> finished)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, finished](KCupsRequest *r) {
        if (r->hasError()) {
            Q_EMIT requestError(i18n("Failed to perform request: %1", r->errorMsg()));
        } else {
            finished();
        }
        r->deleteLater();
    });

    return request;
}

QVariantList PrinterManager::remotePrinters() const
{
    return m_remotePrinters;
}

QVariantList PrinterManager::recommendedDrivers() const
{
    return m_recommendedDrivers;
}

QVariantMap PrinterManager::serverSettings() const
{
    return m_serverSettings;
}

bool PrinterManager::serverSettingsLoaded() const
{
    return m_serverSettingsLoaded;
}

void PrinterManager::removePrinter(const QString &name)
{
    const auto request = setupRequest([this]() -> void {
        Q_EMIT removeDone();
    });
    request->deletePrinter(name);
}

void PrinterManager::makePrinterDefault(const QString &name)
{
    const auto request = setupRequest();
    request->setDefaultPrinter(name);
}

void PrinterManager::getServerSettings()
{
    const auto request = new KCupsRequest();
    connect(request, &KCupsRequest::finished, this, [this](KCupsRequest *r) {
        // When we don't have any destinations, error is set to IPP_NOT_FOUND
        // we can safely ignore the error since it DOES bring the server
        // settings
        if (r->hasError() && r->error() != IPP_NOT_FOUND) {
            Q_EMIT requestError(i18nc("@info", "Failed to get server settings: %1", r->errorMsg()));
            m_serverSettingsLoaded = false;
        } else {
            KCupsServer server = r->serverSettings();
            m_serverSettings[QLatin1String(CUPS_SERVER_USER_CANCEL_ANY)] = server.allowUserCancelAnyJobs();
            m_serverSettings[QLatin1String(CUPS_SERVER_SHARE_PRINTERS)] = server.sharePrinters();
            m_serverSettings[QLatin1String(CUPS_SERVER_REMOTE_ANY)] = server.allowPrintingFromInternet();
            m_serverSettings[QLatin1String(CUPS_SERVER_REMOTE_ADMIN)] = server.allowRemoteAdmin();

            m_serverSettingsLoaded = true;
            Q_EMIT serverSettingsChanged();
        }
        r->deleteLater();
    });

    request->getServerSettings();
}

void PrinterManager::saveServerSettings(const QVariantMap &settings)
{
    KCupsServer server;
    server.setSharePrinters(settings.value(QLatin1String(CUPS_SERVER_SHARE_PRINTERS), false).toBool());
    server.setAllowUserCancelAnyJobs(settings.value(QLatin1String(CUPS_SERVER_USER_CANCEL_ANY), false).toBool());
    server.setAllowRemoteAdmin(settings.value(QLatin1String(CUPS_SERVER_REMOTE_ADMIN), false).toBool());
    server.setAllowPrintingFromInternet(settings.value(QLatin1String(CUPS_SERVER_REMOTE_ANY), false).toBool());

    /**
     * The CUPS server will stop/start when settings change.  This will either be a stop signal
     * from CUPS and/or the non-fatal error below
    */
    auto request = new KCupsRequest;
    request->setServerSettings(server);
    request->waitTillFinished();

    if (request->error() == IPP_AUTHENTICATION_CANCELED) {
        // If the user has cancelled auth
        Q_EMIT requestError(i18nc("@info", "Server Settings Not Saved: (%1): %2", request->serverError(), request->errorMsg()));
    } else if (request->error() == IPP_SERVICE_UNAVAILABLE || request->error() == IPP_INTERNAL_ERROR) {
        // this is the expected "error" after settings have changed as the server is restarting
        m_serverSettings = settings;
        Q_EMIT serverSettingsChanged();
        qCDebug(PMKCM) << "CUPS SETTINGS SAVED!" << settings;
    } else {
        Q_EMIT requestError(i18nc("@info", "Fatal Save Server Settings: (%1): %2", request->serverError(), request->errorMsg()));
    }

    request->deleteLater();
}

bool PrinterManager::shareConnectedPrinters() const
{
    return m_serverSettings.value(QLatin1String(CUPS_SERVER_SHARE_PRINTERS), false).toBool();
}

bool PrinterManager::allowPrintingFromInternet() const
{
    return m_serverSettings.value(QLatin1String(CUPS_SERVER_REMOTE_ANY), false).toBool();
}

bool PrinterManager::allowRemoteAdmin() const
{
    return m_serverSettings.value(QLatin1String(CUPS_SERVER_REMOTE_ADMIN), false).toBool();
}

bool PrinterManager::allowUserCancelAnyJobs() const
{
    return m_serverSettings.value(QLatin1String(CUPS_SERVER_USER_CANCEL_ANY), false).toBool();
}

void PrinterManager::makePrinterShared(const QString &name, bool shared, bool isClass)
{
    const auto request = setupRequest();
    request->setShared(name, isClass, shared);
}

void PrinterManager::makePrinterRejectJobs(const QString &name, bool reject)
{
    const auto request = setupRequest();

    if (reject) {
        request->rejectJobs(name);
    } else {
        request->acceptJobs(name);
    }
}

void PrinterManager::printTestPage(const QString &name, bool isClass)
{
    const auto request = setupRequest();
    request->printTestPage(name, isClass);
}

void PrinterManager::printSelfTestPage(const QString &name)
{
    const auto request = setupRequest();
    request->printCommand(name, u"PrintSelfTestPage"_s, i18n("Print Self-Test Page"));
}

void PrinterManager::cleanPrintHeads(const QString &name)
{
    const auto request = setupRequest();
    request->printCommand(name, u"Clean all"_s, i18n("Clean Print Heads"));
}

#include "printermanager.moc"
