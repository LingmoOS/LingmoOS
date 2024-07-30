/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "runnercontext.h"

#include <cmath>

#include <QPointer>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QSharedData>
#include <QUrl>

#include <KConfigGroup>
#include <KShell>

#include "abstractrunner.h"
#include "abstractrunner_p.h"
#include "querymatch.h"
#include "runnermanager.h"

namespace KRunner
{
KRUNNER_EXPORT int __changeCountBeforeSaving = 5; // For tests
class RunnerContextPrivate : public QSharedData
{
public:
    explicit RunnerContextPrivate(RunnerManager *manager)
        : QSharedData()
        , m_manager(manager)
    {
    }

    RunnerContextPrivate(const RunnerContextPrivate &p)
        : QSharedData(p)
        , m_manager(p.m_manager)
        , launchCounts(p.launchCounts)
        , changedLaunchCounts(p.changedLaunchCounts)
    {
    }

    ~RunnerContextPrivate()
    {
    }

    void invalidate()
    {
        m_isValid = false;
    }

    void addMatch(const QueryMatch &match)
    {
        if (match.runner() && match.runner()->d->hasUniqueResults) {
            if (uniqueIds.contains(match.id())) {
                const QueryMatch &existentMatch = uniqueIds.value(match.id());
                if (existentMatch.runner() && existentMatch.runner()->d->hasWeakResults) {
                    // There is an existing match with the same ID and we are allowed to replace it
                    matches.removeOne(existentMatch);
                    matches.append(match);
                }
            } else {
                // There is no existing match with the same id
                uniqueIds.insert(match.id(), match);
                matches.append(match);
            }
        } else {
            // Runner has the unique results property not set
            matches.append(match);
        }
    }

    void matchesChanged()
    {
        if (m_manager) {
            QMetaObject::invokeMethod(m_manager, "onMatchesChanged");
        }
    }

    QReadWriteLock lock;
    QPointer<RunnerManager> m_manager;
    bool m_isValid = true;
    QList<QueryMatch> matches;
    QHash<QString, int> launchCounts;
    int changedLaunchCounts = 0; // We want to sync them while the app is running, but for each query it is overkill
    QString term;
    bool singleRunnerQueryMode = false;
    bool shouldIgnoreCurrentMatchForHistory = false;
    QMap<QString, QueryMatch> uniqueIds;
    QString requestedText;
    int requestedCursorPosition = 0;
    qint64 queryStartTs = 0;
};

RunnerContext::RunnerContext(RunnerManager *manager)
    : d(new RunnerContextPrivate(manager))
{
}

// copy ctor
RunnerContext::RunnerContext(const RunnerContext &other)
{
    QReadLocker locker(&other.d->lock);
    d = other.d;
}

RunnerContext::~RunnerContext()
{
}

RunnerContext &RunnerContext::operator=(const RunnerContext &other)
{
    if (this->d == other.d) {
        return *this;
    }

    auto oldD = d; // To avoid the old ptr getting destroyed while the mutex is locked
    QWriteLocker locker(&d->lock);
    QReadLocker otherLocker(&other.d->lock);
    d = other.d;
    return *this;
}

/**
 * Resets the search term for this object.
 * This removes all current matches in the process and
 * turns off single runner query mode.
 * Copies of this object that are used by runner are invalidated
 * and adding matches will be a noop.
 */
void RunnerContext::reset()
{
    {
        QWriteLocker locker(&d->lock);
        // We will detach if we are a copy of someone. But we will reset
        // if we are the 'main' context others copied from. Resetting
        // one RunnerContext makes all the copies obsolete.

        // We need to mark the q pointer of the detached RunnerContextPrivate
        // as dirty on detach to avoid receiving results for old queries
        d->invalidate();
    }

    d.detach();
    // But out detached version is valid!
    d->m_isValid = true;

    // we still have to remove all the matches, since if the
    // ref count was 1 (e.g. only the RunnerContext is using
    // the dptr) then we won't get a copy made
    d->matches.clear();
    d->term.clear();
    d->matchesChanged();

    d->uniqueIds.clear();
    d->singleRunnerQueryMode = false;
    d->shouldIgnoreCurrentMatchForHistory = false;
}

void RunnerContext::setQuery(const QString &term)
{
    if (!this->query().isEmpty()) {
        reset();
    }

    if (term.isEmpty()) {
        return;
    }

    d->requestedText.clear(); // Invalidate this field whenever the query changes
    d->term = term;
}

QString RunnerContext::query() const
{
    // the query term should never be set after
    // a search starts. in fact, reset() ensures this
    // and setQuery(QString) calls reset()
    return d->term;
}

bool RunnerContext::isValid() const
{
    QReadLocker locker(&d->lock);
    return d->m_isValid;
}

bool RunnerContext::addMatches(const QList<QueryMatch> &matches)
{
    if (matches.isEmpty() || !isValid()) {
        // Bail out if the query is empty or the qptr is dirty
        return false;
    }

    {
        QWriteLocker locker(&d->lock);
        for (QueryMatch match : matches) {
            // Give previously launched matches a slight boost in relevance
            // The boost smoothly saturates to 0.5;
            if (int count = d->launchCounts.value(match.id())) {
                match.setRelevance(match.relevance() + 0.5 * (1 - exp(-count * 0.3)));
            }
            d->addMatch(match);
        }
    }
    d->matchesChanged();

    return true;
}

bool RunnerContext::addMatch(const QueryMatch &match)
{
    return addMatches({match});
}

QList<QueryMatch> RunnerContext::matches() const
{
    QReadLocker locker(&d->lock);
    QList<QueryMatch> matches = d->matches;
    return matches;
}

void RunnerContext::requestQueryStringUpdate(const QString &text, int cursorPosition) const
{
    d->requestedText = text;
    d->requestedCursorPosition = cursorPosition;
}

void RunnerContext::setSingleRunnerQueryMode(bool enabled)
{
    d->singleRunnerQueryMode = enabled;
}

bool RunnerContext::singleRunnerQueryMode() const
{
    return d->singleRunnerQueryMode;
}

void RunnerContext::ignoreCurrentMatchForHistory() const
{
    d->shouldIgnoreCurrentMatchForHistory = true;
}

bool RunnerContext::shouldIgnoreCurrentMatchForHistory() const
{
    return d->shouldIgnoreCurrentMatchForHistory;
}

/**
 * Sets the launch counts for the associated match ids
 *
 * If a runner adds a match to this context, the context will check if the
 * match id has been launched before and increase the matches relevance
 * correspondingly. In this manner, any front end can implement adaptive search
 * by sorting items according to relevance.
 *
 * @param config the config group where launch data was stored
 */
void RunnerContext::restore(const KConfigGroup &config)
{
    const QStringList cfgList = config.readEntry("LaunchCounts", QStringList());

    for (const QString &entry : cfgList) {
        if (int idx = entry.indexOf(QLatin1Char(' ')); idx != -1) {
            const int count = entry.mid(0, idx).toInt();
            const QString id = entry.mid(idx + 1);
            d->launchCounts[id] = count;
        }
    }
}

void RunnerContext::save(KConfigGroup &config)
{
    if (d->changedLaunchCounts < __changeCountBeforeSaving) {
        return;
    }
    d->changedLaunchCounts = 0;
    QStringList countList;
    countList.reserve(d->launchCounts.size());
    for (auto it = d->launchCounts.cbegin(), end = d->launchCounts.cend(); it != end; ++it) {
        countList << QString::number(it.value()) + QLatin1Char(' ') + it.key();
    }

    config.writeEntry("LaunchCounts", countList);
    config.sync();
}

void RunnerContext::increaseLaunchCount(const QueryMatch &match)
{
    ++d->launchCounts[match.id()];
    ++d->changedLaunchCounts;
}

QString RunnerContext::requestedQueryString() const
{
    return d->requestedText;
}
int RunnerContext::requestedCursorPosition() const
{
    return d->requestedCursorPosition;
}

void RunnerContext::setJobStartTs(qint64 queryStartTs)
{
    d->queryStartTs = queryStartTs;
}
QString RunnerContext::runnerJobId(AbstractRunner *runner) const
{
    return QLatin1String("%1-%2-%3").arg(runner->id(), query(), QString::number(d->queryStartTs));
}

} // KRunner namespace
