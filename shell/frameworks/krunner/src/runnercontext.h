/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRUNNER_RUNNERCONTEXT_H
#define KRUNNER_RUNNERCONTEXT_H

#include <QList>
#include <QMetaType>
#include <QSharedDataPointer>

#include "krunner_export.h"

class KConfigGroup;

namespace KRunner
{
class RunnerManager;
class QueryMatch;
class AbstractRunner;
class RunnerContextPrivate;

/**
 * @class RunnerContext runnercontext.h <KRunner/RunnerContext>
 *
 * @short The RunnerContext class provides information related to a search,
 *        including the search term and collected matches.
 */
class KRUNNER_EXPORT RunnerContext final
{
public:
    explicit RunnerContext(RunnerManager *manager = nullptr);

    /**
     * Copy constructor
     */
    RunnerContext(const RunnerContext &other);

    /**
     * Assignment operator
     */
    RunnerContext &operator=(const RunnerContext &other);

    ~RunnerContext();

    /**
     * Sets the query term for this object and attempts to determine
     * the type of the search.
     */
    void setQuery(const QString &term);

    /**
     * @return the current search query term.
     */
    QString query() const;

    /**
     * @returns true if this context is no longer valid and therefore
     * matching using it should abort.
     * While not required to be used within runners, it provides a nice way
     * to avoid unnecessary processing in runners that may run for an extended
     * period (as measured in 10s of ms) and therefore improve the user experience.
     */
    bool isValid() const;

    /**
     * Appends lists of matches to the list of matches.
     *
     * @param matches the matches to add
     * @return true if matches were added, false if matches were e.g. outdated
     */
    bool addMatches(const QList<QueryMatch> &matches);

    /**
     * Appends a match to the existing list of matches.
     *
     * If you are going to be adding multiple matches, it is
     * more performant to use @see addMatches instead.
     *
     * @param match the match to add
     * @return true if the match was added, false otherwise.
     */
    bool addMatch(const QueryMatch &match);

    /**
     * Retrieves all available matches for the current search term.
     *
     * @return a list of matches
     */
    QList<QueryMatch> matches() const;

    /**
     * Request that KRunner updates the query string and stasy open, even after running a match.
     * This method is const so it can be called in a const context.
     *
     * @param text Text that will be displayed in the search field
     * @param cursorPosition Position of the cursor, if this is different than the length of the text,
     * the characters between the position and text will be selected
     *
     * @since 5.90
     */
    void requestQueryStringUpdate(const QString &text, int cursorPosition) const;

    /**
     * @return true if the current query is a single runner query
     */
    bool singleRunnerQueryMode() const;

    /**
     * Set this to true in the AbstractRunner::run method to prevent the entry
     * from being saved to the history.
     * @since 5.90
     */
    void ignoreCurrentMatchForHistory() const;

private:
    KRUNNER_NO_EXPORT void increaseLaunchCount(const QueryMatch &match);
    KRUNNER_NO_EXPORT QString requestedQueryString() const;
    KRUNNER_NO_EXPORT int requestedCursorPosition() const;
    KRUNNER_NO_EXPORT bool shouldIgnoreCurrentMatchForHistory() const;
    // Sets single runner query mode. Note that a call to reset() will turn off single runner query mode.
    KRUNNER_NO_EXPORT void setSingleRunnerQueryMode(bool enabled);

    friend class RunnerManager;
    friend class AbstractRunner;
    friend class DBusRunner;
    friend class RunnerManagerPrivate;

    KRUNNER_NO_EXPORT void restore(const KConfigGroup &config);
    KRUNNER_NO_EXPORT void save(KConfigGroup &config);
    KRUNNER_NO_EXPORT void reset();
    KRUNNER_NO_EXPORT void setJobStartTs(qint64 queryStartTs);
    KRUNNER_NO_EXPORT QString runnerJobId(AbstractRunner *runner) const;

    QExplicitlySharedDataPointer<RunnerContextPrivate> d;
};
}

Q_DECLARE_METATYPE(KRunner::RunnerContext)
#endif
