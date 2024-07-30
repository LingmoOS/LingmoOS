/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2020-2023 Alexander Lohnau<alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRUNNER_ABSTRACTRUNNER_H
#define KRUNNER_ABSTRACTRUNNER_H

#include "krunner_export.h"

#include <QObject>
#include <QStringList>

#include <KPluginFactory>
#include <KPluginMetaData>

#include <memory>

#include "querymatch.h"
#include "runnercontext.h"
#include "runnersyntax.h"

class KConfigGroup;
class QMimeData;
class QRegularExpression;
class QIcon;

namespace KRunner
{
class AbstractRunnerPrivate;

/**
 * @class AbstractRunner abstractrunner.h <KRunner/AbstractRunner>
 *
 * @short An abstract base class for Plasma Runner plugins.
 *
 * Be aware that runners will be moved to their own thread after being instantiated.
 * This means that except for AbstractRunner::run and the constructor, all methods will be non-blocking
 * for the UI.
 * Consider doing heavy resource initialization in the init method instead of the constructor.
 */
class KRUNNER_EXPORT AbstractRunner : public QObject
{
    Q_OBJECT

public:
    ~AbstractRunner() override;

    /**
     * This is the main query method. It should trigger creation of
     * QueryMatch instances through RunnerContext::addMatch and
     * RunnerContext::addMatches.
     *
     * If the runner can run precisely the requested term (RunnerContext::query()),
     * it should create an exact match by setting the type to RunnerContext::ExactMatch.
     * The first runner that creates a QueryMatch will be the
     * default runner. Other runner's matches will be suggested in the
     * interface. Non-exact matches should be offered via RunnerContext::PossibleMatch.
     *
     * The match will be activated via run() if the user selects it.
     *
     * All matches need to be reported once this method returns. Asynchronous runners therefore need
     * to make use of a local event loop to wait for all matches.
     *
     * It is recommended to use local status data in async runners. The simplest way is
     * to have a separate class doing all the work like so:
     *
     * \code
     * void MyFancyAsyncRunner::match(RunnerContext &context)
     * {
     *     QEventLoop loop;
     *     MyAsyncWorker worker(context);
     *     connect(&worker, &MyAsyncWorker::finished, &loop, &MyAsyncWorker::quit);
     *     worker.work();
     *     loop.exec();
     * }
     * \endcode
     *
     * Here MyAsyncWorker creates all the matches and calls RunnerContext::addMatch
     * in some internal slot. It emits the finished() signal once done which will
     * quit the loop and make the match() method return.
     *
     * Execution of the correct action should be handled in the run method.
     *
     * @warning Returning from this method means to end execution of the runner.
     *
     * @sa run(), RunnerContext::addMatch, RunnerContext::addMatches, QueryMatch
     */
    virtual void match(KRunner::RunnerContext &context) = 0;

    /**
     * Called whenever an exact or possible match associated with this
     * runner is triggered.
     *
     * @param context The context in which the match is triggered, i.e. for which
     *                the match was created.
     * @param match The actual match to run/execute.
     */
    virtual void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match);

    /**
     * @return the plugin metadata for this runner that was passed in the constructor
     */
    KPluginMetaData metadata() const;

    /**
     * Returns the translated name from the runner's metadata
     */
    QString name() const;

    /**
     * @return an id from the runner's metadata'
     */
    QString id() const;

    /**
     * Reloads the runner's configuration. This is called when it's KCM in the PluginSelector is applied.
     * This function may be used to set for example using setMatchRegex, setMinLetterCount or setTriggerWords.
     * Also, syntaxes should be updated when this method is called.
     * While reloading the config, matching is suspended.
     */
    virtual void reloadConfiguration();

    /**
     * @return the syntaxes the runner has registered that it accepts and understands
     */
    QList<RunnerSyntax> syntaxes() const;

    /**
     * @return true if the runner is currently busy with non-interuptable work, signaling that
     * the RunnerManager may not query it or read it's config properties
     */
    bool isMatchingSuspended() const;

    /**
     * This is the minimum letter count for the query. If the query is shorter than this value
     * and KRunner is not in the singleRunnerMode, match method is not called.
     * This can be set using the X-Plasma-Runner-Min-Letter-Count property or the setMinLetterCount method.
     * The default value is 0.
     *
     * @see setMinLetterCount
     * @see match
     * @since 5.75
     */
    int minLetterCount() const;

    /**
     * Set the minLetterCount property
     * @param count
     * @since 5.75
     */
    void setMinLetterCount(int count);

    /**
     * If this regex is set with a non empty pattern it must match the query in order for match being called.
     * Just like the minLetterCount property this check is ignored when the runner is in the singleRunnerMode.
     * In case both the regex and the letter count is set the letter count is checked first.
     * @return matchRegex property
     * @see hasMatchRegex
     * @since 5.75
     */
    QRegularExpression matchRegex() const;

    /**
     * Set the matchRegex property
     * @param regex
     * @since 5.75
     */
    void setMatchRegex(const QRegularExpression &regex);

    /**
     * Constructs internally a regex which requires the query to start with the trigger words.
     * Multiple words are concatenated with or, for instance: "^word1|word2|word3".
     * The trigger words are internally escaped.
     * Also the minLetterCount is set to the shortest word in the list.
     * @since 5.75
     * @see matchRegex
     */
    void setTriggerWords(const QStringList &triggerWords);

    /**
     * If the runner has a valid regex and non empty regex
     * @internal
     * @since 5.75
     */
    bool hasMatchRegex() const;

Q_SIGNALS:
    /**
     * This signal is emitted when matching is about to commence, giving runners
     * an opportunity to prepare themselves, e.g. loading data sets or preparing
     * IPC or network connections. Things that should be loaded once and remain
     * extant for the lifespan of the AbstractRunner should be done in init().
     * @see init()
     */
    void prepare();

    /**
     * This signal is emitted when a session of matches is complete, giving runners
     * the opportunity to tear down anything set up as a result of the prepare()
     * method.
     */
    void teardown();

protected:
    friend class RunnerManager;
    friend class RunnerManagerPrivate;

    /**
     * Constructor for a KRunner plugin
     *
     * @note You should connect here to the prepare/teardown signals. However, avoid doing heavy initialization here
     * in favor of doing it in AbstractRunner::init
     *
     * @param parent parent object for this runner
     * @param pluginMetaData metadata that was embedded in the runner
     * @param args for compatibility with KPluginFactory, since 6.0 this can be omitted
     * @since 5.72
     */
    explicit AbstractRunner(QObject *parent, const KPluginMetaData &pluginMetaData);

    /**
     * Sets whether or not the runner is available for match requests. Useful to
     * prevent queries when the runner is in a busy state.
     * @note Do not permanently suspend the runner. This is only intended as a temporary measure to
     * avoid useless queries being launched or async fetching of config/data being interfered with.
     */
    void suspendMatching(bool suspend);

    /**
     * Provides access to the runner's configuration object.
     * This config is saved in the "krunnerrc" file in the [Runners][<pluginId>] config group
     * Settings should be written in a KDE config module. See https://develop.kde.org/docs/plasma/krunner/#runner-configuration
     */
    KConfigGroup config() const;

    /**
     * Adds a registered syntax that this runner understands. This is used to
     * display to the user what this runner can understand and how it can be
     * used.
     *
     * @param syntax the syntax to register
     */
    void addSyntax(const RunnerSyntax &syntax);

    /**
     * Utility overload for creating a syntax based on the given parameters
     * @see RunnerSyntax
     * @since 5.106
     */
    inline void addSyntax(const QString &exampleQuery, const QString &description)
    {
        addSyntax(QStringList(exampleQuery), description);
    }

    /// @copydoc addSyntax(const QString &exampleQuery, const QString &description)
    inline void addSyntax(const QStringList &exampleQueries, const QString &description)
    {
        addSyntax(KRunner::RunnerSyntax(exampleQueries, description));
    }

    /**
     * Sets the list of syntaxes; passing in an empty list effectively clears
     * the syntaxes.
     *
     * @param the syntaxes to register for this runner
     */
    void setSyntaxes(const QList<RunnerSyntax> &syntaxes);

    /**
     * Reimplement this to run any initialization routines on first load.
     * Because it is executed in the runner's thread, it will not block the UI and is thus preferred.
     * By default, it calls reloadConfiguration();
     *
     * Until the runner is initialized, it will not be queried by the RunnerManager.
     */
    virtual void init();

    /**
     * Reimplement this if you want your runner to support serialization and drag and drop.
     * By default, this sets the QMimeData urls to the ones specified in @ref QueryMatch::urls
     */
    virtual QMimeData *mimeDataForMatch(const KRunner::QueryMatch &match);

private:
    std::unique_ptr<AbstractRunnerPrivate> const d;
    KRUNNER_NO_EXPORT Q_INVOKABLE void matchInternal(KRunner::RunnerContext context);
    KRUNNER_NO_EXPORT Q_INVOKABLE void reloadConfigurationInternal();
    KRUNNER_NO_EXPORT Q_SIGNAL void matchInternalFinished(const QString &jobId);
    KRUNNER_NO_EXPORT Q_SIGNAL void matchingResumed();
    friend class RunnerManager;
    friend class RunnerContext;
    friend class RunnerContextPrivate;
    friend class QueryMatchPrivate;
    friend class DBusRunner; // Because it "overrides" matchInternal
};

} // KRunner namespace
#endif
