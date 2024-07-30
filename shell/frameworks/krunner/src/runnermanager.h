/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Ryan P. Bitanga <ryan.bitanga@gmail.com>
    SPDX-FileCopyrightText: 2008 Jordi Polo <mumismo@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnauŋmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRUNNER_RUNNERMANAGER_H
#define KRUNNER_RUNNERMANAGER_H

#include <QList>
#include <QObject>

#include <KPluginMetaData>

#include "abstractrunner.h"
#include "action.h"
#include "krunner_export.h"
#include <memory>

class KConfigGroup;
namespace KRunner
{
class AbstractRunnerTest;
}

namespace KRunner
{
class QueryMatch;
class AbstractRunner;
class RunnerContext;
class RunnerManagerPrivate;

/**
 * @class RunnerManager runnermanager.h <KRunner/RunnerManager>
 *
 * @short The RunnerManager class decides what installed runners are runnable,
 *        and their ratings. It is the main proxy to the runners.
 */
class KRUNNER_EXPORT RunnerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList history READ history)
    Q_PROPERTY(bool historyEnabled READ historyEnabled WRITE setHistoryEnabled NOTIFY historyEnabledChanged)

public:
    /**
     * Constructs a RunnerManager with the given parameters
     * @param configurationGroup Config group used for reading enabled plugins
     * @param stateGroup Config group used for storing history
     * @since 6.0
     */
    explicit RunnerManager(const KConfigGroup &pluginConfigGroup, const KConfigGroup &stateGroup, QObject *parent);

    /**
     * Constructs a RunnerManager using the default locations for state/plugin config
     */
    explicit RunnerManager(QObject *parent = nullptr);
    ~RunnerManager() override;

    /**
     * Finds and returns a loaded runner or a nullptr
     * @param pluginId the name of the runner plugin
     * @return Pointer to the runner
     */
    AbstractRunner *runner(const QString &pluginId) const;

    /**
     * @return the list of all currently loaded runners
     */
    QList<AbstractRunner *> runners() const;

    /**
     * Retrieves the current context
     * @return pointer to the current context
     */
    RunnerContext *searchContext() const;

    /**
     * Retrieves all available matches found so far for the previously launched query
     * @return List of matches
     */
    QList<QueryMatch> matches() const;

    /**
     * Runs a given match. This also respects the extra handling for the InformationalMatch.
     * This also handles the history automatically
     * @param match the match to be executed
     * @param selectedAction the action returned by @ref QueryMatch::actions that has been selected by the user, nullptr if none
     * @return if the RunnerWindow should close
     * @since 6.0
     */
    bool run(const QueryMatch &match, const KRunner::Action &action = {});

    /**
     * @return the current query term set in @ref launchQuery
     */
    QString query() const;

    /**
     * @return History of this runner for the current activity. If the RunnerManager is not history
     * aware the global entries will be returned.
     * @since 5.78
     */
    QStringList history() const;

    /**
     * Delete the given index from the history.
     * @param historyEntry
     * @since 5.78
     */
    Q_INVOKABLE void removeFromHistory(int index);

    /**
     * Get the suggested history entry for the typed query. If no entry is found an empty string is returned.
     * @param typedQuery
     * @return completion for typedQuery
     * @since 5.78
     */
    Q_INVOKABLE QString getHistorySuggestion(const QString &typedQuery) const;

    /**
     * If history completion is enabled, the default value is true.
     * @since 5.78
     */
    bool historyEnabled();

    /**
     * Enables/disabled the history feature for the RunnerManager instance.
     * The value will not be persisted and is only kept during the object's lifetime.
     *
     * @since 6.0
     */
    void setHistoryEnabled(bool enabled);

    /**
     * Causes a reload of the current configuration
     * This gets called automatically when the config in the KCM is saved
     */
    void reloadConfiguration();

    /**
     * Sets a whitelist for the plugins that can be loaded by this manager.
     * Runners that are disabled through the config will not be loaded.
     *
     * @param plugins the plugin names of allowed runners
     */
    void setAllowedRunners(const QStringList &runners);

    /**
     * Attempts to add the AbstractRunner plugin represented
     * by the plugin info passed in. Usually one can simply let
     * the configuration of plugins handle loading Runner plugins,
     * but in cases where specific runners should be loaded this
     * allows for that to take place
     * @note Consider using @ref setAllowedRunners in case you want to only allow specific runners
     *
     * @param pluginMetaData the metaData to use to load the plugin
     * @return the loaded runner or nullptr
     */
    AbstractRunner *loadRunner(const KPluginMetaData &pluginMetaData);

    /**
     * @return mime data of the specified match
     */
    QMimeData *mimeDataForMatch(const QueryMatch &match) const;

    /**
     * @return metadata list of all known Runner plugins
     * @since 5.72
     */
    static QList<KPluginMetaData> runnerMetaDataList();

public Q_SLOTS:
    /**
     * Call this method when the runners should be prepared for a query session.
     * Call matchSessionComplete when the query session is finished for the time
     * being.
     * @see matchSessionComplete
     */
    void setupMatchSession();

    /**
     * Call this method when the query session is finished for the time
     * being.
     * @see prepareForMatchSession
     */
    void matchSessionComplete();

    /**
     * Launch a query, this will create threads and return immediately.
     * When the information will be available can be known using the
     * matchesChanged signal.
     *
     * @param term the term we want to find matches for
     * @param runnerId optional, if only one specific runner is to be used;
     *               providing an id will put the manager into single runner mode
     */
    void launchQuery(const QString &term, const QString &runnerId = QString());

    /**
     * Reset the current data and stops the query
     */
    void reset();

    /**
     * Set the environment identifier for recording history and launch counts
     * @internal
     * @since 6.0
     */
    Q_INVOKABLE void setHistoryEnvironmentIdentifier(const QString &identifier);

Q_SIGNALS:
    /**
     * Emitted each time a new match is added to the list
     */
    void matchesChanged(const QList<KRunner::QueryMatch> &matches);

    /**
     * Emitted when the launchQuery finish
     */
    void queryFinished();

    /**
     * Put the given search term in the KRunner search field
     * @param term The term that should be displayed
     * @param cursorPosition Where the cursor should be positioned
     * @since 6.0
     */
    void requestUpdateQueryString(const QString &term, int cursorPosition);

    /**
     * @see @p historyEnabled
     * @since 5.78
     */
    void historyEnabledChanged();

private:
    // exported for dbusrunnertest
    KPluginMetaData convertDBusRunnerToJson(const QString &filename) const;
    KRUNNER_NO_EXPORT Q_INVOKABLE void onMatchesChanged();

    std::unique_ptr<RunnerManagerPrivate> d;

    friend class RunnerManagerPrivate;
    friend AbstractRunnerTest;
    friend AbstractRunner;
};

}
#endif
