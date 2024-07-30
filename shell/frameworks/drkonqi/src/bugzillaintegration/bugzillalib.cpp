/*******************************************************************
 * bugzillalib.cpp
 * SPDX-FileCopyrightText: 2009, 2011 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2012 George Kiagiadakis <kiagiadakis.george@gmail.com>
 * SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "bugzillalib.h"

#include <QReadWriteLock>
#include <QRegularExpression>

#include "drkonqi_debug.h"
#include "drkonqi_globals.h"
#include "libbugzilla/bugzilla.h"
#include "libbugzilla/clients/attachmentclient.h"
#include "libbugzilla/clients/bugclient.h"
#include "libbugzilla/clients/productclient.h"
#include "libbugzilla/connection.h"

static const char showBugUrl[] = "show_bug.cgi?id=%1";

// Extra filter rigging. We don't want to leak secrets via qdebug, so install
// a message handler which does nothing more than replace secrets in debug
// messages with placeholders.
// This is used as a global static (since message handlers are meant to be
// static) and is slightly synchronizing across threads WRT the filter hash.
struct QMessageFilterContainer {
    QMessageFilterContainer();
    ~QMessageFilterContainer();
    void insert(const QString &needle, const QString &replace);
    void clear();

    QString filter(const QString &msg);

    // Message handler is called across threads. Synchronize for good measure.
    QReadWriteLock lock;
    QtMessageHandler handler;

private:
    QHash<QString, QString> filters;
};

Q_GLOBAL_STATIC(QMessageFilterContainer, s_messageFilter)

QMessageFilterContainer::QMessageFilterContainer()
{
    handler = qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        s_messageFilter->handler(type, context, s_messageFilter->filter(msg));
    });
}

QMessageFilterContainer::~QMessageFilterContainer()
{
    qInstallMessageHandler(handler);
}

void QMessageFilterContainer::insert(const QString &needle, const QString &replace)
{
    if (needle.isEmpty()) {
        return;
    }

    QWriteLocker locker(&lock);
    filters[needle] = replace;
}

QString QMessageFilterContainer::filter(const QString &msg)
{
    QReadLocker locker(&lock);
    QString filteredMsg = msg;
    for (auto it = filters.constBegin(); it != filters.constEnd(); ++it) {
        filteredMsg.replace(it.key(), it.value());
    }
    return filteredMsg;
}

void QMessageFilterContainer::clear()
{
    QWriteLocker locker(&lock);
    filters.clear();
}

BugzillaManager::BugzillaManager(const QString &bugTrackerUrl, QObject *parent)
    : QObject(parent)
    , m_bugTrackerUrl(bugTrackerUrl.isEmpty() ? KDE_BUGZILLA_URL : bugTrackerUrl)
{
    Q_ASSERT(bugTrackerUrl.endsWith(QLatin1Char('/')));
    Bugzilla::setConnection(new Bugzilla::HTTPConnection(QUrl(m_bugTrackerUrl + QStringLiteral("rest"))));
}

void BugzillaManager::lookupVersion()
{
    KJob *job = Bugzilla::version();
    connect(job, &KJob::finished, this, [this](KJob *job) {
        try {
            QString version = Bugzilla::version(job);
            setFeaturesForVersion(version);
            Q_EMIT bugzillaVersionFound();
        } catch (Bugzilla::Exception &e) {
            // Version detection problems simply mean we'll not mark the version
            // found and the UI will not allow reporting.
            qCWarning(DRKONQI_LOG) << e.whatString();
            Q_EMIT bugzillaVersionError(e.whatString());
        }
    });
}

void BugzillaManager::setFeaturesForVersion(const QString &version)
{
    // A procedure to change Dr Konqi behaviour automatically when Bugzilla
    // software versions change.
    //
    // Changes should be added to Dr Konqi AHEAD of when the corresponding
    // Bugzilla software changes are released into bugs.kde.org, so that
    // Dr Konqi can continue to operate smoothly, without bug reports and a
    // reactive KDE software release.
    //
    // If Bugzilla announces a change to its software that affects Dr Konqi,
    // add executable code to implement the change automatically when the
    // Bugzilla software version changes. It goes at the end of this procedure
    // and elsewhere in this class (BugzillaManager) and/or other classes where
    // the change should actually be implemented.

    const int nVersionParts = 3;
    QStringList digits = version.split(QRegularExpression(QStringLiteral("[._-]")), Qt::SkipEmptyParts);
    while (digits.count() < nVersionParts) {
        digits << QLatin1String("0");
    }
    if (digits.count() > nVersionParts) {
        qCWarning(DRKONQI_LOG)
            << QStringLiteral("Current Bugzilla version %1 has more than %2 parts. Check that this is not a problem.").arg(version).arg(nVersionParts);
    }

    qCDebug(DRKONQI_LOG) << "VERSION" << version;
}

void BugzillaManager::tryLogin(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
    refreshToken();
}

void BugzillaManager::refreshToken()
{
    Q_ASSERT(!m_username.isEmpty());
    Q_ASSERT(!m_password.isEmpty());
    m_logged = false;

    // Rest token and qdebug filters
    Bugzilla::connection().setToken(QString());
    s_messageFilter->clear();
    s_messageFilter->insert(m_password, QStringLiteral("PASSWORD"));

    KJob *job = Bugzilla::login(m_username, m_password);
    connect(job, &KJob::finished, this, [this](KJob *job) {
        try {
            auto details = Bugzilla::login(job);
            m_token = details.token;
            if (m_token.isEmpty()) {
                throw Bugzilla::RuntimeException(QStringLiteral("Did not receive a token"));
            }

            s_messageFilter->insert(m_token, QStringLiteral("TOKEN"));
            Bugzilla::connection().setToken(m_token);
            m_logged = true;

            Q_EMIT loginFinished(true);
        } catch (Bugzilla::Exception &e) {
            qCWarning(DRKONQI_LOG) << e.whatString();
            // Version detection problems simply mean we'll not mark the version
            // found and the UI will not allow reporting.
            Q_EMIT loginError(e.whatString());
        }
    });
}

bool BugzillaManager::getLogged() const
{
    return m_logged;
}

QString BugzillaManager::getUsername() const
{
    return m_username;
}

void BugzillaManager::sendReport(const Bugzilla::NewBug &bug)
{
    auto job = Bugzilla::BugClient().create(bug);
    connect(job, &KJob::finished, this, [this](KJob *job) {
        try {
            int id = Bugzilla::BugClient().create(job);
            Q_ASSERT(id > 0);
            Q_EMIT reportSent(id);
        } catch (Bugzilla::Exception &e) {
            qCWarning(DRKONQI_LOG) << e.whatString();
            Q_EMIT sendReportError(e.whatString());
        }
    });
}

void BugzillaManager::attachTextToReport(const QString &text, const QString &filename, const QString &summary, int bugId, const QString &comment)
{
    Bugzilla::NewAttachment attachment;
    attachment.ids = QList<int>{bugId};
    attachment.data = text;
    attachment.file_name = filename;
    attachment.summary = summary;
    attachment.comment = comment;
    attachment.content_type = QLatin1String("text/plain");

    auto job = Bugzilla::AttachmentClient().createAttachment(bugId, attachment);
    connect(job, &KJob::finished, this, [this, bugId](KJob *job) {
        try {
            const QList<int> attachmentIds = Bugzilla::AttachmentClient().createAttachment(job);
            Q_ASSERT(attachmentIds.size() == 1); // NB: attachmentIds are not bug ids!
            Q_EMIT attachToReportSent(bugId);
        } catch (Bugzilla::Exception &e) {
            qCWarning(DRKONQI_LOG) << e.whatString();
            Q_EMIT attachToReportError(e.whatString());
        }
    });
}

void BugzillaManager::fetchProductInfo(const QString &product)
{
    auto job = Bugzilla::ProductClient().get(product);
    connect(job, &KJob::finished, this, [this](KJob *job) {
        try {
            auto ptr = Bugzilla::ProductClient().get(job);
            Q_ASSERT(ptr);
            Q_EMIT productInfoFetched(ptr);
        } catch (Bugzilla::Exception &e) {
            qCWarning(DRKONQI_LOG) << e.whatString();
            // This doesn't have a string because it is actually not used for
            // anything...
            Q_EMIT productInfoError();
        }
    });
}

QString BugzillaManager::urlForBug(int bug_number) const
{
    return QString(m_bugTrackerUrl) + QString::fromLatin1(showBugUrl).arg(bug_number);
}

#include "moc_bugzillalib.cpp"
