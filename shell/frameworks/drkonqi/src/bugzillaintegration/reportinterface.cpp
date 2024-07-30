/*******************************************************************
 * reportinterface.cpp
 * SPDX-FileCopyrightText: 2009, 2010, 2011 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "reportinterface.h"

#include <chrono>

#include <QGuiApplication>

#include <KIO/TransferJob>
#include <KLocalizedString>

#include "backtracegenerator.h"
#include "bugzillalib.h"
#include "config-drkonqi.h"
#include "crashedapplication.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "drkonqi_debug.h"
#include "parser/backtraceparser.h"
#include "productmapping.h"
#include "sentryconnection.h"
#include "settings.h"
#include "systeminformation.h"

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

// Max size a report may have. This is enforced in bugzilla, hardcoded, and
// cannot be queried through the API, so handle this client-side in a hardcoded
// fashion as well.
static const int s_maxReportSize = 65535;

ReportInterface::ReportInterface(QObject *parent)
    : QObject(parent)
    , m_sentryPostbox(DrKonqi::crashedApplication()->fakeExecutableBaseName(), std::make_shared<SentryNetworkConnection>())
{
    m_bugzillaManager = new BugzillaManager(KDE_BUGZILLA_URL, this);

    m_productMapping = new ProductMapping(DrKonqi::crashedApplication(), m_bugzillaManager, this);

    // Information the user can provide about the crash
    m_userRememberCrashSituation = false;
    m_reproducible = ReproducibleUnsure;
    m_provideActionsApplicationDesktop = false;
    m_provideUnusualBehavior = false;
    m_provideApplicationConfigurationDetails = false;

    // Do not attach the bug report to any other existent report (create a new one)
    m_attachToBugNumber = 0;

    connect(&m_sentryPostbox, &SentryPostbox::hasDeliveredChanged, this, [this] {
        maybeDone();
    });

    m_sentryStartTimer.setInterval(5s);
    m_sentryStartTimer.setSingleShot(true);
    connect(&m_sentryStartTimer, &QTimer::timeout, this, &ReportInterface::trySentry);
    connect(Settings::self(), &Settings::SentryChanged, &m_sentryStartTimer, QOverload<>::of(&QTimer::start));
    trySentry();
}

void ReportInterface::setBugAwarenessPageData(bool rememberSituation, Reproducible reproducible, bool actions, bool unusual, bool configuration)
{
    // Save the information the user can provide about the crash from the assistant page
    m_userRememberCrashSituation = rememberSituation;
    m_reproducible = reproducible;
    m_provideActionsApplicationDesktop = actions;
    m_provideUnusualBehavior = unusual;
    m_provideApplicationConfigurationDetails = configuration;
}

bool ReportInterface::isBugAwarenessPageDataUseful() const
{
    // Determine if the assistant should proceed, considering the amount of information
    // the user can provide
    int rating = selectedOptionsRating();

    // Minimum information required even for a good backtrace.
    bool useful = m_userRememberCrashSituation && (rating >= 2 || (m_reproducible == ReproducibleSometimes || m_reproducible == ReproducibleEverytime));
    return useful;
}

int ReportInterface::selectedOptionsRating() const
{
    // Check how many information the user can provide and generate a rating
    int rating = 0;
    if (m_provideActionsApplicationDesktop) {
        rating += 3;
    }
    if (m_provideApplicationConfigurationDetails) {
        rating += 2;
    }
    if (m_provideUnusualBehavior) {
        rating += 1;
    }
    return rating;
}

QString ReportInterface::backtrace() const
{
    return m_backtrace;
}

void ReportInterface::setBacktrace(const QString &backtrace)
{
    m_backtrace = backtrace;
    Q_EMIT backtraceChanged();
}

QString ReportInterface::title() const
{
    return m_reportTitle;
}

void ReportInterface::setTitle(const QString &text)
{
    m_reportTitle = text;
    Q_EMIT titleChanged();
}

void ReportInterface::setDetailText(const QString &text)
{
    m_reportDetailText = text;
    Q_EMIT detailTextChanged();
}

QString ReportInterface::generateReportFullText(DrKonqiStamp stamp, Backtrace inlineBacktrace) const
{
    // Note: no translations should be done in this function's strings

    const CrashedApplication *crashedApp = DrKonqi::crashedApplication();
    const SystemInformation *sysInfo = DrKonqi::systemInformation();

    QString report;

    // Program name and versions
    report.append(QStringLiteral("Application: %1 (%2)\n").arg(crashedApp->fakeExecutableBaseName(), crashedApp->version()));
    if (sysInfo->compiledSources()) {
        report.append(QStringLiteral(" (Compiled from sources)\n"));
    } else {
        report.append(QLatin1Char('\n'));
    }
    report.append(u"ApplicationNotResponding [ANR]: %1\n"_s.arg(crashedApp->wasNotResponding() ? u"true"_s : u"false"_s));
    report.append(QStringLiteral("Qt Version: %1\n").arg(sysInfo->qtVersion()));
    report.append(QStringLiteral("Frameworks Version: %1\n").arg(sysInfo->frameworksVersion()));

    report.append(QStringLiteral("Operating System: %1\n").arg(sysInfo->operatingSystem()));
    report.append(QStringLiteral("Windowing System: %1\n").arg(sysInfo->windowSystem()));

    // LSB output or manually selected distro
    if (!sysInfo->distributionPrettyName().isEmpty()) {
        report.append(QStringLiteral("Distribution: %1\n").arg(sysInfo->distributionPrettyName()));
    } else if (!sysInfo->bugzillaPlatform().isEmpty() && sysInfo->bugzillaPlatform() != QLatin1String("unspecified")) {
        report.append(QStringLiteral("Distribution (Platform): %1\n").arg(sysInfo->bugzillaPlatform()));
    }

    report.append(QStringLiteral("DrKonqi: %1 [%2]\n").arg(QString::fromLatin1(PROJECT_VERSION), DrKonqi::backendClassName()));
    report.append(QLatin1Char('\n'));

    // Details of the crash situation
    if (isBugAwarenessPageDataUseful()) {
        report.append(QStringLiteral("-- Information about the crash:\n"));
        if (!m_reportDetailText.isEmpty()) {
            report.append(m_reportDetailText.trimmed());
        } else {
            // If the user manual reports this crash, he/she should know what to put in here.
            // This message is the only one translated in this function
            report.append(xi18nc("@info/plain",
                                 "<placeholder>In detail, tell us what you were doing "
                                 " when the application crashed.</placeholder>"));
        }
        report.append(QLatin1String("\n\n"));
    }

    // Crash reproducibility
    switch (m_reproducible) {
    case ReproducibleUnsure:
        report.append(QStringLiteral("The reporter is unsure if this crash is reproducible.\n\n"));
        break;
    case ReproducibleNever:
        report.append(QStringLiteral("The crash does not seem to be reproducible.\n\n"));
        break;
    case ReproducibleSometimes:
        report.append(QStringLiteral("The crash can be reproduced sometimes.\n\n"));
        break;
    case ReproducibleEverytime:
        report.append(QStringLiteral("The crash can be reproduced every time.\n\n"));
        break;
    }

    // Backtrace
    switch (inlineBacktrace) {
    case Backtrace::Complete:
        report.append(QStringLiteral("-- Backtrace:\n"));
        break;
    case Backtrace::Reduced:
        report.append(QStringLiteral("-- Backtrace (Reduced):\n"));
        break;
    case Backtrace::Exclude:
        report.append(QStringLiteral("The backtrace was excluded and likely attached as a file.\n"));
        break;
    }
    if (!m_backtrace.isEmpty()) {
        switch (inlineBacktrace) {
        case Backtrace::Complete:
            report.append(m_backtrace.trimmed() + QLatin1Char('\n'));
            break;
        case Backtrace::Reduced:
            report.append(DrKonqi::debuggerManager()->backtraceGenerator()->parser()->simplifiedBacktrace() + QLatin1Char('\n'));
            break;
        case Backtrace::Exclude:
            report.append(QStringLiteral("The backtrace is attached as a comment due to length constraints\n"));
            break;
        }
    } else {
        report.append(QStringLiteral("A useful backtrace could not be generated\n"));
    }

    switch (stamp) {
    case DrKonqiStamp::Include: {
        report.append(QLatin1String("\nReported using DrKonqi"));
        const QString product = m_productMapping->bugzillaProduct();
        const QString originalProduct = m_productMapping->bugzillaProductOriginal();
        if (!originalProduct.isEmpty()) {
            report.append(
                QStringLiteral(
                    "\nThis report was filed against '%1' because the product '%2' could not be located in Bugzilla. Add it to drkonqi's mappings file!")
                    .arg(product, originalProduct));
        }
        break;
    }
    case DrKonqiStamp::Exclude:
        break;
    }

    return report;
}

QString ReportInterface::generateAttachmentComment() const
{
    // Note: no translations should be done in this function's strings

    const CrashedApplication *crashedApp = DrKonqi::crashedApplication();
    const SystemInformation *sysInfo = DrKonqi::systemInformation();

    QString comment;

    // Program name and versions
    comment.append(QStringLiteral("%1 (%2) using Qt %4\n\n").arg(crashedApp->fakeExecutableBaseName(), crashedApp->version(), sysInfo->qtVersion()));

    // Details of the crash situation
    if (isBugAwarenessPageDataUseful()) {
        comment.append(QStringLiteral("%1\n\n").arg(m_reportDetailText.trimmed()));
    }

    // Backtrace (only 6 lines)
    comment.append(QStringLiteral("-- Backtrace (Reduced):\n"));
    QString reducedBacktrace = DrKonqi::debuggerManager()->backtraceGenerator()->parser()->simplifiedBacktrace();
    comment.append(reducedBacktrace.trimmed());

    return comment;
}

Bugzilla::NewBug ReportInterface::newBugReportTemplate() const
{
    const SystemInformation *sysInfo = DrKonqi::systemInformation();

    Bugzilla::NewBug bug;
    bug.product = m_productMapping->bugzillaProduct();
    bug.component = m_productMapping->bugzillaComponent();
    bug.version = m_productMapping->bugzillaVersion();
    bug.op_sys = sysInfo->bugzillaOperatingSystem();
    if (sysInfo->compiledSources()) {
        bug.platform = QLatin1String("Compiled Sources");
    } else {
        bug.platform = sysInfo->bugzillaPlatform();
    }
    bug.keywords = QStringList{QStringLiteral("drkonqi")};
    bug.priority = QLatin1String("NOR");
    bug.severity = QLatin1String("crash");
    bug.summary = m_reportTitle;
    if (DrKonqi::crashedApplication()->wasNotResponding()) {
        bug.summary.prepend("[ANR] "_L1);
    }

    return bug;
}

QByteArray journalPriorityToSentryLevel(const QByteArray &priorityBytes)
{
    bool ok = false;
    auto priority = priorityBytes.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) {
        return "info"_ba;
    }
    // https://www.freedesktop.org/software/systemd/man/latest/systemd.journal-fields.html#PRIORITY=
    switch (priority) {
    case 0:
    case 1:
    case 2:
        return "fatal"_ba;
    case 3:
        return "error"_ba;
    case 4:
        return "warning"_ba;
    case 5:
    case 6:
        return "info"_ba;
    case 7:
        return "debug"_ba;
    };
    return "info"_ba;
}

void ReportInterface::prepareEventPayload()
{
    auto eventPayload = DrKonqi::debuggerManager()->backtraceGenerator()->sentryPayload();
    auto document = QJsonDocument::fromJson(eventPayload);
    auto hash = document.object().toVariantMap();

    // replace the timestamp with the real timestamp (possibly originating in journald)
    hash.insert(u"timestamp"_s, DrKonqi::crashedApplication()->datetime().toUTC().toString(Qt::ISODateWithMs));

    bool hadSomeVerbosity = false;

    {
        QList<QVariant> breadcrumbs;
        const auto logs = DrKonqi::crashedApplication()->m_logs;
        for (const auto &logEntry : logs) {
            QVariantHash breadcrumb;

            breadcrumb.insert(u"type"_s, u"debug"_s);
            const auto category = logEntry.value("QT_CATEGORY"_ba, "default"_ba);
            breadcrumb.insert(u"category"_s, category);
            breadcrumb.insert(u"message"_s, logEntry.value("MESSAGE"_ba));
            // PRIORITY isn't a builtin field, it may be missing. Assume info (6) when it is missing.
            const auto level = journalPriorityToSentryLevel(logEntry.value("PRIORITY"_ba, "6"_ba));
            breadcrumb.insert(u"level"_s, level);
            auto ok = false;
            auto realtime = logEntry.value("_SOURCE_REALTIME_TIMESTAMP"_ba).toULongLong(&ok);
            if (ok && realtime > 0) {
                std::chrono::microseconds timestamp(realtime);
                std::chrono::duration<double> timestampDouble(timestamp);
                breadcrumb.insert(u"timestamp"_s, QJsonValue(timestampDouble.count()));
            }

            breadcrumbs.push_back(breadcrumb);

            // Add an identifier tag if we had some debug output, suggesting some degree of verbosity
            if (level == "debug"_ba && category != "default"_ba) {
                hadSomeVerbosity = true;
            }
        }
        std::reverse(breadcrumbs.begin(), breadcrumbs.end());
        hash.insert(u"breadcrumbs"_s, breadcrumbs);
    }

    {
        constexpr auto CONTEXTS_KEY = "contexts"_L1;
        auto context = hash.take(CONTEXTS_KEY).toHash();
        context.insert(u"gpu"_s,
                       QVariantHash{
                           {u"name"_s, DrKonqi::instance()->m_glRenderer}, //
                           {u"version"_s, QGuiApplication::platformName()}, // NOTE: drkonqi gets invoked with the same platform as the crashed app
                       });
        hash.insert(CONTEXTS_KEY, context);
    }

    {
        constexpr auto TAGS_KEY = "tags"_L1;
        auto tags = hash.take(TAGS_KEY).toHash();
        tags.insert(u"some_verbosity"_s, hadSomeVerbosity);
        // Tag the gui platform as well because the version from the gpu field cannot be easily filtered for
        tags.insert(u"gui_platform"_s, QGuiApplication::platformName()); // NOTE: drkonqi gets invoked with the same platform as the crashed app
        hash.insert(TAGS_KEY, tags);
    }

    constexpr auto EXCEPTION_KEY = "exception"_L1;
    constexpr auto VALUES_KEY = "values"_L1;

    if (DrKonqi::instance()->crashedApplication()->wasNotResponding()) {
        // Because of QJson shortcomings this is a bit of a mouthful.
        // Essentially this force-changes the exception types to "ANR". It does so using a bunch of intermediate objects.
        auto exception = hash.take(EXCEPTION_KEY).toHash();
        auto values = exception.take(VALUES_KEY).toJsonArray();
        QJsonArray newValues;
        for (const auto &value : values) {
            auto valueHash = value.toObject().toVariantHash();
            auto mechanism = valueHash[u"mechanism"_s].toHash();
            mechanism.insert(u"type"_s, u"ANR"_s);
            mechanism.remove(u"handled"_s); // set to unknown. technically the ANR is the handling of the problem ;)
            valueHash.insert(u"mechanism"_s, mechanism);
            valueHash.insert(u"type"_s, u"ApplicationNotResponding"_s);
            valueHash.insert(u"value"_s, u"Aborted because unresponsive (probably by the window manager)"_s);
            newValues.append(QJsonObject::fromVariantHash(valueHash));
        }
        exception.insert(VALUES_KEY, newValues);
        hash.insert(EXCEPTION_KEY, exception);
        hash.insert(u"level"_s, u"error"_s);
    }

    if (!DrKonqi::instance()->m_exceptionName.isEmpty()) {
        auto exception = hash.take(EXCEPTION_KEY).toHash();
        auto values = exception.take(VALUES_KEY).toJsonArray();
        // Prepend since the exception is likely the root of the problem stack.
        values.prepend(QJsonObject({
            {u"type"_s, DrKonqi::instance()->m_exceptionName},
            {u"value"_s, DrKonqi::instance()->m_exceptionWhat},
            {u"mechanism"_s,
             QJsonObject{
                 {u"handled"_s, false},
                 {u"synthetic"_s, false},
                 {u"type"_s, u"drkonqi"_s},
             }},
        }));
        exception.insert(VALUES_KEY, values);
        hash.insert(EXCEPTION_KEY, exception);
    }

    m_sentryPostbox.addEventPayload(QJsonDocument::fromVariant(hash));
    maybePickUpPostbox();
}

void ReportInterface::prepareCrashEvent()
{
    switch (DrKonqi::debuggerManager()->backtraceGenerator()->state()) {
    case BacktraceGenerator::Loaded:
        return prepareEventPayload();
    case BacktraceGenerator::Loading:
    case BacktraceGenerator::NotLoaded:
    case BacktraceGenerator::Failed:
    case BacktraceGenerator::FailedToStart:
        break;
    }
    static bool connected = false;
    if (!connected) {
        connected = true;
        connect(DrKonqi::debuggerManager()->backtraceGenerator(), &BacktraceGenerator::done, this, [this] {
            prepareEventPayload();
        });
    }
    if (DrKonqi::debuggerManager()->backtraceGenerator()->state() != BacktraceGenerator::Loading) {
        DrKonqi::debuggerManager()->backtraceGenerator()->start();
    }
}

void ReportInterface::prepareCrashComment()
{
    m_sentryPostbox.addUserFeedback(m_reportTitle + QLatin1Char('\n') + m_reportDetailText + QLatin1Char('\n') + DrKonqi::kdeBugzillaURL()
                                    + QLatin1String("show_bug.cgi?id=%1").arg(QString::number(m_sentReport)));
    maybePickUpPostbox();
}

void ReportInterface::createCrashMessage(const QString &message)
{
    m_sentryPostbox.addUserFeedback(message);
    maybePickUpPostbox();
}

void ReportInterface::sendBugReport()
{
    prepareCrashEvent();

    bool attach = false;
    Bugzilla::NewBug report = newBugReportTemplate();
    report.description = generateReportFullText(ReportInterface::DrKonqiStamp::Include, ReportInterface::Backtrace::Complete);

    // If the report is too long try to reduce it, try to not include the
    // backtrace and eventually give up.
    // Bugzilla has a hard-limit on the server side, if we cannot strip the
    // report down enough the submission will simply not work.
    // Exhausting the cap with just user input is nigh impossible, so we'll
    // forego handling of the report being too long even without without
    // backtrace.
    // https://bugs.kde.org/show_bug.cgi?id=248807
    if (report.description.size() >= s_maxReportSize) {
        report.description = generateReportFullText(ReportInterface::DrKonqiStamp::Include, ReportInterface::Backtrace::Reduced);
        attach = true;
    }
    if (report.description.size() >= s_maxReportSize) {
        report.description = generateReportFullText(ReportInterface::DrKonqiStamp::Include, ReportInterface::Backtrace::Exclude);
        attach = true;
    }
    Q_ASSERT(!report.description.isEmpty());

    connect(m_bugzillaManager, &BugzillaManager::sendReportErrorInvalidValues, this, &ReportInterface::sendUsingDefaultProduct);
    connect(m_bugzillaManager, &BugzillaManager::reportSent, this, [this, attach](int bugId) {
        if (attach) {
            m_attachToBugNumber = bugId;
            attachBacktrace(QStringLiteral("DrKonqi auto-attaching complete backtrace."));
        } else {
            m_sentReport = bugId;
            prepareCrashComment();
            m_sentryPostbox.deliver();
            maybeDone();
        }
    });
    connect(m_bugzillaManager, &BugzillaManager::sendReportError, this, &ReportInterface::sendReportError);
    m_bugzillaManager->sendReport(report);
}

void ReportInterface::sendUsingDefaultProduct() const
{
    // Fallback function: if some of the custom values fail, we need to reset all the fields to the default
    //(and valid) bugzilla values; and try to resend
    Bugzilla::NewBug bug = newBugReportTemplate();
    bug.product = QLatin1String("kde");
    bug.component = QLatin1String("general");
    bug.platform = QLatin1String("unspecified");
    bug.description = generateReportFullText(ReportInterface::DrKonqiStamp::Include, ReportInterface::Backtrace::Complete);
    m_bugzillaManager->sendReport(bug);
}

void ReportInterface::attachBacktraceWithReport()
{
    attachBacktrace(generateAttachmentComment());
}

void ReportInterface::attachBacktrace(const QString &comment)
{
    // The user was added to the CC list, proceed with the attachment
    connect(m_bugzillaManager, &BugzillaManager::attachToReportSent, this, &ReportInterface::attachSent);
    connect(m_bugzillaManager, &BugzillaManager::attachToReportError, this, &ReportInterface::sendReportError);

    QString reportText = generateReportFullText(ReportInterface::DrKonqiStamp::Include, ReportInterface::Backtrace::Complete);
    QString filename = getSuggestedKCrashFilename(DrKonqi::crashedApplication());
    QLatin1String summary("New crash information added by DrKonqi");

    // Attach the report. The comment of the attachment also includes the bug description
    m_bugzillaManager->attachTextToReport(reportText, filename, summary, m_attachToBugNumber, comment);
}

void ReportInterface::attachSent(int attachId)
{
    Q_UNUSED(attachId);

    // The bug was attached, consider it "sent"
    m_sentReport = attachId;
    prepareCrashComment();
    m_sentryPostbox.deliver();
    maybeDone();
}

QStringList ReportInterface::relatedBugzillaProducts() const
{
    return m_productMapping->relatedBugzillaProducts();
}

bool ReportInterface::isWorthReporting() const
{
    if (DrKonqi::ignoreQuality()) {
        return true;
    }

    // Evaluate if the provided information is useful enough to enable the automatic report
    bool needToReport = false;

    if (!m_userRememberCrashSituation) {
        // This should never happen... but...
        return false;
    }

    int rating = selectedOptionsRating();

    BacktraceParser::Usefulness use = DrKonqi::debuggerManager()->backtraceGenerator()->parser()->backtraceUsefulness();

    switch (use) {
    case BacktraceParser::ReallyUseful: {
        // Perfect backtrace: require at least one option or a 100%-50% reproducible crash
        needToReport = (rating >= 2) || (m_reproducible == ReproducibleEverytime || m_reproducible == ReproducibleSometimes);
        break;
    }
    case BacktraceParser::MayBeUseful: {
        // Not perfect backtrace: require at least two options or a 100% reproducible crash
        needToReport = (rating >= 3) || (m_reproducible == ReproducibleEverytime);
        break;
    }
    case BacktraceParser::ProbablyUseless:
        // Bad backtrace: require at least two options and always reproducible (strict)
        needToReport = (rating >= 5) && (m_reproducible == ReproducibleEverytime);
        break;
    case BacktraceParser::Useless:
    case BacktraceParser::InvalidUsefulness: {
        needToReport = false;
    }
    }

    return needToReport;
}

void ReportInterface::setAttachToBugNumber(uint bugNumber)
{
    // If bugNumber>0, the report is going to be attached to bugNumber
    m_attachToBugNumber = bugNumber;
    Q_EMIT attachToBugNumberChanged();
}

uint ReportInterface::attachToBugNumber() const
{
    return m_attachToBugNumber;
}

BugzillaManager *ReportInterface::bugzillaManager() const
{
    return m_bugzillaManager;
}

ProductMapping *ReportInterface::productMapping() const
{
    return m_productMapping;
}

void ReportInterface::maybeDone()
{
    if (m_sentReport != 0 && m_sentryPostbox.hasDelivered()) {
        Q_EMIT done();
    }
};

ReportInterface *ReportInterface::self()
{
    static ReportInterface interface;
    return &interface;
}

bool ReportInterface::hasCrashEventSent() const
{
    return !isCrashEventSendingEnabled() || m_sentryPostbox.hasDelivered() || m_skipSentry;
}

bool ReportInterface::isCrashEventSendingEnabled() const
{
    qCDebug(DRKONQI_LOG) << "sentry:" << Settings::self()->sentry() //
                         << "forceSentry:" << m_forceSentry //
                         << "hasDeletedFiles:" << DrKonqi::crashedApplication()->hasDeletedFiles() //
                         << "skipSentry:" << m_skipSentry;
    const auto enabled = Settings::self()->sentry() || m_forceSentry;
    return enabled && !DrKonqi::crashedApplication()->hasDeletedFiles();
}

void ReportInterface::setSendWhenReady(bool send)
{
    m_sendWhenReady = send;
    maybePickUpPostbox();
}

void ReportInterface::maybePickUpPostbox()
{
    qWarning() << Q_FUNC_INFO;
    if (m_sendWhenReady && !m_sentryPostbox.hasDelivered() && m_sentryPostbox.isReadyToDeliver()) {
        m_sentryPostbox.deliver();
    }
}
void ReportInterface::sendSentryReport()
{
    m_forceSentry = true;
    trySentry();
    // sendWhenReady is set by the quit handling in main.cpp
}

void ReportInterface::trySentry()
{
    qCDebug(DRKONQI_LOG) << "trying sentry?" << isCrashEventSendingEnabled() << !m_tryingSentry;
    if (isCrashEventSendingEnabled() && !m_tryingSentry) {
        m_tryingSentry = true;
        metaObject()->invokeMethod(this, [this] {
            connect(&m_sentryPostbox, &SentryPostbox::hasDeliveredChanged, this, &ReportInterface::crashEventSent);
            // Send crash event ASAP, if applicable. Trace quality doesn't matter for it.
            prepareCrashEvent();
        });
    }
}

QString ReportInterface::sentryEventId() const
{
    return m_sentryPostbox.eventId();
}

#include "moc_reportinterface.cpp"
