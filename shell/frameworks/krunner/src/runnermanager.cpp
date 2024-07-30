/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007, 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
    SPDX-FileCopyrightText: 2008 Jordi Polo <mumismo@gmail.com>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnauŋmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "runnermanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QMutableListIterator>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

#include <KConfigWatcher>
#include <KFileUtils>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <memory>

#include "abstractrunner_p.h"
#include "dbusrunner_p.h"
#include "kpluginmetadata_utils_p.h"
#include "krunner_debug.h"
#include "querymatch.h"

namespace KRunner
{
class RunnerManagerPrivate
{
public:
    RunnerManagerPrivate(const KConfigGroup &configurationGroup, const KConfigGroup &stateConfigGroup, RunnerManager *parent)
        : q(parent)
        , context(parent)
        , pluginConf(configurationGroup)
        , stateData(stateConfigGroup)
    {
        initializeKNotifyPluginWatcher();
        matchChangeTimer.setSingleShot(true);
        matchChangeTimer.setTimerType(Qt::TimerType::PreciseTimer); // Without this, autotest will fail due to imprecision of this timer

        QObject::connect(&matchChangeTimer, &QTimer::timeout, q, [this]() {
            matchesChanged();
        });

        // Set up tracking of the last time matchesChanged was signalled
        lastMatchChangeSignalled.start();
        QObject::connect(q, &RunnerManager::matchesChanged, q, [&] {
            lastMatchChangeSignalled.restart();
        });
        loadConfiguration();
    }

    void scheduleMatchesChanged()
    {
        // We avoid over-refreshing the client. We only refresh every this much milliseconds
        constexpr int refreshPeriod = 250;
        // This will tell us if we are reseting the matches to start a new search. RunnerContext::reset() clears its query string for its emission
        if (context.query().isEmpty()) {
            matchChangeTimer.stop();
            // This actually contains the query string for the new search that we're launching (if any):
            if (!this->untrimmedTerm.trimmed().isEmpty()) {
                // We are starting a new search, we shall stall for some time before deciding to show an empty matches list.
                // This stall should be enough for the engine to provide more meaningful result, so we avoid refreshing with
                // an empty results list if possible.
                matchChangeTimer.start(refreshPeriod);
                // We "pretend" that we have refreshed it so the next call will be forced to wait the timeout:
                lastMatchChangeSignalled.restart();
            } else {
                // We have an empty input string, so it's not a real query. We don't expect any results to come, so no need to stall
                Q_EMIT q->matchesChanged(context.matches());
            }
        } else if (lastMatchChangeSignalled.hasExpired(refreshPeriod)) {
            matchChangeTimer.stop();
            Q_EMIT q->matchesChanged(context.matches());
        } else {
            matchChangeTimer.start(refreshPeriod - lastMatchChangeSignalled.elapsed());
        }
    }

    void matchesChanged()
    {
        Q_EMIT q->matchesChanged(context.matches());
    }

    void loadConfiguration()
    {
        const KConfigGroup generalConfig = pluginConf.config()->group(QStringLiteral("General"));
        context.restore(stateData);
    }

    void loadSingleRunner()
    {
        // In case we are not in the single runner mode of we do not have an id
        if (!singleMode || singleModeRunnerId.isEmpty()) {
            currentSingleRunner = nullptr;
            return;
        }

        if (currentSingleRunner && currentSingleRunner->id() == singleModeRunnerId) {
            return;
        }
        currentSingleRunner = q->runner(singleModeRunnerId);
        // If there are no runners loaded or the single runner could no be loaded,
        // this is the case if it was disabled but gets queries using the singleRunnerMode, BUG: 435050
        if (runners.isEmpty() || !currentSingleRunner) {
            loadRunners(singleModeRunnerId);
            currentSingleRunner = q->runner(singleModeRunnerId);
        }
    }

    void deleteRunners(const QList<AbstractRunner *> &runners)
    {
        for (const auto runner : runners) {
            if (qobject_cast<DBusRunner *>(runner)) {
                runner->deleteLater();
            } else {
                Q_ASSERT(runner->thread() != q->thread());
                runner->thread()->quit();
                QObject::connect(runner->thread(), &QThread::finished, runner->thread(), &QObject::deleteLater);
                QObject::connect(runner->thread(), &QThread::finished, runner, &QObject::deleteLater);
            }
        }
    }

    void loadRunners(const QString &singleRunnerId = QString())
    {
        QList<KPluginMetaData> offers = RunnerManager::runnerMetaDataList();

        const bool loadAll = stateData.readEntry("loadAll", false);
        const bool noWhiteList = whiteList.isEmpty();

        QList<AbstractRunner *> deadRunners;
        QMutableListIterator<KPluginMetaData> it(offers);
        while (it.hasNext()) {
            const KPluginMetaData &description = it.next();
            qCDebug(KRUNNER) << "Loading runner: " << description.pluginId();

            const QString runnerName = description.pluginId();
            const bool isPluginEnabled = description.isEnabled(pluginConf);
            const bool loaded = runners.contains(runnerName);
            bool selected = loadAll || disabledRunnerIds.contains(runnerName) || (isPluginEnabled && (noWhiteList || whiteList.contains(runnerName)));
            if (!selected && runnerName == singleRunnerId) {
                selected = true;
                disabledRunnerIds << runnerName;
            }

            if (selected) {
                if (!loaded) {
                    if (auto runner = loadInstalledRunner(description)) {
                        qCDebug(KRUNNER) << "Loaded:" << runnerName;
                        runners.insert(runnerName, runner);
                    }
                }
            } else if (loaded) {
                // Remove runner
                deadRunners.append(runners.take(runnerName));
                qCDebug(KRUNNER) << "Plugin disabled. Removing runner: " << runnerName;
            }
        }

        deleteRunners(deadRunners);
        // in case we deleted it up above, just to be sure we do not have a dangeling pointer
        currentSingleRunner = nullptr;
        qCDebug(KRUNNER) << "All runners loaded, total:" << runners.count();
    }

    AbstractRunner *loadInstalledRunner(const KPluginMetaData &pluginMetaData)
    {
        if (!pluginMetaData.isValid()) {
            return nullptr;
        }

        AbstractRunner *runner = nullptr;

        const QString api = pluginMetaData.value(QStringLiteral("X-Plasma-API"));
        const bool isCppPlugin = api.isEmpty();

        if (isCppPlugin) {
            if (auto res = KPluginFactory::instantiatePlugin<AbstractRunner>(pluginMetaData, q)) {
                runner = res.plugin;
            } else {
                qCWarning(KRUNNER).nospace() << "Could not load runner " << pluginMetaData.name() << ":" << res.errorString
                                             << " (library path was:" << pluginMetaData.fileName() << ")";
            }
        } else if (api.startsWith(QLatin1String("DBus"))) {
            runner = new DBusRunner(q, pluginMetaData);
        } else {
            qCWarning(KRUNNER) << "Unknown X-Plasma-API requested for runner" << pluginMetaData.fileName();
            return nullptr;
        }

        if (runner) {
            QPointer<AbstractRunner> ptr(runner);
            q->connect(runner, &AbstractRunner::matchingResumed, q, [this, ptr]() {
                if (ptr) {
                    runnerMatchingResumed(ptr.get());
                }
            });
            if (isCppPlugin) {
                auto thread = new QThread();
                thread->setObjectName(pluginMetaData.pluginId());
                thread->start();
                runner->moveToThread(thread);
            }
            // The runner might outlive the manager due to us waiting for the thread to exit
            q->connect(runner, &AbstractRunner::matchInternalFinished, q, [this](const QString &jobId) {
                onRunnerJobFinished(jobId);
            });

            if (prepped) {
                Q_EMIT runner->prepare();
            }
        }

        return runner;
    }

    void onRunnerJobFinished(const QString &jobId)
    {
        if (currentJobs.remove(jobId) && currentJobs.isEmpty()) {
            // If there are any new matches scheduled to be notified, we should anticipate it and just refresh right now
            if (matchChangeTimer.isActive()) {
                matchChangeTimer.stop();
                matchesChanged();
            } else if (context.matches().isEmpty()) {
                // we finished our run, and there are no valid matches, and so no
                // signal will have been sent out, so we need to emit the signal ourselves here
                matchesChanged();
            }
            Q_EMIT q->queryFinished(); // NOLINT(readability-misleading-indentation)
        }
        if (!currentJobs.isEmpty()) {
            qCDebug(KRUNNER) << "Current jobs are" << currentJobs;
        }
    }

    void teardown()
    {
        pendingJobsAfterSuspend.clear(); // Do not start old jobs when the match session is over
        if (allRunnersPrepped) {
            for (AbstractRunner *runner : std::as_const(runners)) {
                Q_EMIT runner->teardown();
            }
            allRunnersPrepped = false;
        }

        if (singleRunnerPrepped) {
            if (currentSingleRunner) {
                Q_EMIT currentSingleRunner->teardown();
            }
            singleRunnerPrepped = false;
        }

        prepped = false;
    }

    void runnerMatchingResumed(AbstractRunner *runner)
    {
        Q_ASSERT(runner);
        const QString jobId = pendingJobsAfterSuspend.value(runner);
        if (jobId.isEmpty()) {
            qCDebug(KRUNNER) << runner << "was not scheduled for current query";
            return;
        }
        // Ignore this runner
        if (singleMode && runner->id() != singleModeRunnerId) {
            qCDebug(KRUNNER) << runner << "did not match requested singlerunnermode ID";
            return;
        }

        const QString query = context.query();
        bool matchesCount = singleMode || runner->minLetterCount() <= query.size();
        bool matchesRegex = singleMode || !runner->hasMatchRegex() || runner->matchRegex().match(query).hasMatch();

        if (matchesCount && matchesRegex) {
            startJob(runner);
        } else {
            onRunnerJobFinished(jobId);
        }
    }

    void startJob(AbstractRunner *runner)
    {
        QMetaObject::invokeMethod(runner, "matchInternal", Qt::QueuedConnection, Q_ARG(KRunner::RunnerContext, context));
    }

    // Must only be called once
    void initializeKNotifyPluginWatcher()
    {
        Q_ASSERT(!watcher);
        watcher = KConfigWatcher::create(KSharedConfig::openConfig(pluginConf.config()->name()));
        q->connect(watcher.data(), &KConfigWatcher::configChanged, q, [this](const KConfigGroup &group, const QByteArrayList &changedNames) {
            const QString groupName = group.name();
            if (groupName == QLatin1String("Plugins")) {
                q->reloadConfiguration();
            } else if (groupName == QLatin1String("Runners")) {
                for (auto *runner : std::as_const(runners)) {
                    // Signals from the KCM contain the component name, which is the KRunner plugin's id
                    if (changedNames.contains(runner->metadata().pluginId().toUtf8())) {
                        QMetaObject::invokeMethod(runner, "reloadConfigurationInternal");
                    }
                }
            } else if (group.parent().isValid() && group.parent().name() == QLatin1String("Runners")) {
                for (auto *runner : std::as_const(runners)) {
                    // If the same config group has been modified which gets created in AbstractRunner::config()
                    if (groupName == runner->id()) {
                        QMetaObject::invokeMethod(runner, "reloadConfigurationInternal");
                    }
                }
            }
        });
    }

    void addToHistory()
    {
        const QString term = context.query();
        // We want to imitate the shall behavior
        if (!historyEnabled || term.isEmpty() || untrimmedTerm.startsWith(QLatin1Char(' '))) {
            return;
        }
        QStringList historyEntries = readHistoryForCurrentEnv();
        // Avoid removing the same item from the front and prepending it again
        if (!historyEntries.isEmpty() && historyEntries.constFirst() == term) {
            return;
        }

        historyEntries.removeOne(term);
        historyEntries.prepend(term);

        while (historyEntries.count() > 50) { // we don't want to store more than 50 entries
            historyEntries.removeLast();
        }
        writeHistory(historyEntries);
    }

    void writeHistory(const QStringList &historyEntries)
    {
        stateData.group(QStringLiteral("History")).writeEntry(historyEnvironmentIdentifier, historyEntries, KConfig::Notify);
        stateData.sync();
    }

    inline QStringList readHistoryForCurrentEnv()
    {
        return stateData.group(QStringLiteral("History")).readEntry(historyEnvironmentIdentifier, QStringList());
    }

    QString historyEnvironmentIdentifier = QStringLiteral("default");
    RunnerManager *const q;
    RunnerContext context;
    QTimer matchChangeTimer;
    QElapsedTimer lastMatchChangeSignalled;
    QHash<QString, AbstractRunner *> runners;
    QHash<AbstractRunner *, QString> pendingJobsAfterSuspend;
    AbstractRunner *currentSingleRunner = nullptr;
    QSet<QString> currentJobs;
    QString singleModeRunnerId;
    bool prepped = false;
    bool allRunnersPrepped = false;
    bool singleRunnerPrepped = false;
    bool singleMode = false;
    bool historyEnabled = true;
    QStringList whiteList;
    KConfigWatcher::Ptr watcher;
    QString untrimmedTerm;
    KConfigGroup pluginConf;
    KConfigGroup stateData;
    QSet<QString> disabledRunnerIds; // Runners that are disabled but were loaded as single runners
};

RunnerManager::RunnerManager(const KConfigGroup &pluginConfigGroup, const KConfigGroup &stateConfigGroup, QObject *parent)
    : QObject(parent)
    , d(new RunnerManagerPrivate(pluginConfigGroup, stateConfigGroup, this))
{
    Q_ASSERT(pluginConfigGroup.isValid());
    Q_ASSERT(stateConfigGroup.isValid());
}

RunnerManager::RunnerManager(QObject *parent)
    : QObject(parent)
{
    auto defaultStatePtr = KSharedConfig::openConfig(QStringLiteral("krunnerstaterc"), KConfig::NoGlobals, QStandardPaths::GenericDataLocation);
    auto configPtr = KSharedConfig::openConfig(QStringLiteral("krunnerrc"), KConfig::NoGlobals);
    d = std::make_unique<RunnerManagerPrivate>(configPtr->group(QStringLiteral("Plugins")),
                                               defaultStatePtr->group(QStringLiteral("PlasmaRunnerManager")),
                                               this);
}

RunnerManager::~RunnerManager()
{
    d->context.reset();
    d->deleteRunners(d->runners.values());
}

void RunnerManager::reloadConfiguration()
{
    d->pluginConf.config()->reparseConfiguration();
    d->stateData.config()->reparseConfiguration();
    d->loadConfiguration();
    d->loadRunners();
}

void RunnerManager::setAllowedRunners(const QStringList &runners)
{
    d->whiteList = runners;
    if (!d->runners.isEmpty()) {
        // this has been called with runners already created. so let's do an instant reload
        d->loadRunners();
    }
}

AbstractRunner *RunnerManager::loadRunner(const KPluginMetaData &pluginMetaData)
{
    const QString runnerId = pluginMetaData.pluginId();
    if (auto loadedRunner = d->runners.value(runnerId)) {
        return loadedRunner;
    }
    if (!runnerId.isEmpty()) {
        if (AbstractRunner *runner = d->loadInstalledRunner(pluginMetaData)) {
            d->runners.insert(runnerId, runner);
            return runner;
        }
    }
    return nullptr;
}

AbstractRunner *RunnerManager::runner(const QString &pluginId) const
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    return d->runners.value(pluginId, nullptr);
}

QList<AbstractRunner *> RunnerManager::runners() const
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }
    return d->runners.values();
}

RunnerContext *RunnerManager::searchContext() const
{
    return &d->context;
}

QList<QueryMatch> RunnerManager::matches() const
{
    return d->context.matches();
}

bool RunnerManager::run(const QueryMatch &match, const KRunner::Action &selectedAction)
{
    if (!match.isValid() || !match.isEnabled()) { // The model should prevent this
        return false;
    }

    // Modify the match and run it
    QueryMatch m = match;
    m.setSelectedAction(selectedAction);
    m.runner()->run(d->context, m);
    // To allow the RunnerContext to increase the relevance of often launched apps
    d->context.increaseLaunchCount(m);

    if (!d->context.shouldIgnoreCurrentMatchForHistory()) {
        d->addToHistory();
    }
    if (d->context.requestedQueryString().isEmpty()) {
        return true;
    } else {
        Q_EMIT requestUpdateQueryString(d->context.requestedQueryString(), d->context.requestedCursorPosition());
        return false;
    }
}

QMimeData *RunnerManager::mimeDataForMatch(const QueryMatch &match) const
{
    return match.isValid() ? match.runner()->mimeDataForMatch(match) : nullptr;
}

QList<KPluginMetaData> RunnerManager::runnerMetaDataList()
{
    QList<KPluginMetaData> pluginMetaDatas = KPluginMetaData::findPlugins(QStringLiteral("kf6/krunner"));
    QSet<QString> knownRunnerIds;
    knownRunnerIds.reserve(pluginMetaDatas.size());
    for (const KPluginMetaData &pluginMetaData : std::as_const(pluginMetaDatas)) {
        knownRunnerIds.insert(pluginMetaData.pluginId());
    }

    const QStringList dBusPlugindirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("krunner/dbusplugins"), QStandardPaths::LocateDirectory);
    const QStringList dbusRunnerFiles = KFileUtils::findAllUniqueFiles(dBusPlugindirs, QStringList(QStringLiteral("*.desktop")));
    for (const QString &dbusRunnerFile : dbusRunnerFiles) {
        KPluginMetaData pluginMetaData = parseMetaDataFromDesktopFile(dbusRunnerFile);
        if (pluginMetaData.isValid() && !knownRunnerIds.contains(pluginMetaData.pluginId())) {
            pluginMetaDatas.append(pluginMetaData);
            knownRunnerIds.insert(pluginMetaData.pluginId());
        }
    }

    return pluginMetaDatas;
}

void RunnerManager::setupMatchSession()
{
    if (d->prepped) {
        return;
    }

    d->prepped = true;
    if (d->singleMode) {
        if (d->currentSingleRunner) {
            Q_EMIT d->currentSingleRunner->prepare();
            d->singleRunnerPrepped = true;
        }
    } else {
        for (AbstractRunner *runner : std::as_const(d->runners)) {
            if (!d->disabledRunnerIds.contains(runner->name())) {
                Q_EMIT runner->prepare();
            }
        }

        d->allRunnersPrepped = true;
    }
}

void RunnerManager::matchSessionComplete()
{
    if (!d->prepped) {
        return;
    }

    d->teardown();
    // We save the context config after each session, just like the history entries
    // BUG: 424505
    d->context.save(d->stateData);
}

void RunnerManager::launchQuery(const QString &untrimmedTerm, const QString &runnerName)
{
    d->pendingJobsAfterSuspend.clear(); // Do not start old jobs when we got a new query
    QString term = untrimmedTerm.trimmed();
    const QString prevSingleRunner = d->singleModeRunnerId;
    d->untrimmedTerm = untrimmedTerm;

    // Set the required values and load the runner
    d->singleModeRunnerId = runnerName;
    d->singleMode = !runnerName.isEmpty();
    d->loadSingleRunner();
    // If we could not load the single runner we reset
    if (!runnerName.isEmpty() && !d->currentSingleRunner) {
        reset();
        return;
    }
    if (term.isEmpty()) {
        QTimer::singleShot(0, this, &RunnerManager::queryFinished);
        reset();
        return;
    }

    if (d->context.query() == term && prevSingleRunner == runnerName) {
        // we already are searching for this!
        return;
    }

    if (!d->singleMode && d->runners.isEmpty()) {
        d->loadRunners();
    }

    reset();
    d->context.setQuery(term);

    QHash<QString, AbstractRunner *> runnable;

    // if the name is not empty we will launch only the specified runner
    if (d->singleMode) {
        runnable.insert(QString(), d->currentSingleRunner);
        d->context.setSingleRunnerQueryMode(true);
    } else {
        runnable = d->runners;
    }

    qint64 startTs = QDateTime::currentMSecsSinceEpoch();
    d->context.setJobStartTs(startTs);
    setupMatchSession();
    for (KRunner::AbstractRunner *r : std::as_const(runnable)) {
        const QString &jobId = d->context.runnerJobId(r);
        if (r->isMatchingSuspended()) {
            d->pendingJobsAfterSuspend.insert(r, jobId);
            d->currentJobs.insert(jobId);
            continue;
        }
        // If this runner is loaded but disabled
        if (!d->singleMode && d->disabledRunnerIds.contains(r->id())) {
            continue;
        }
        // The runners can set the min letter count as a property, this way we don't
        // have to spawn threads just for the runner to reject the query, because it is too short
        if (!d->singleMode && term.length() < r->minLetterCount()) {
            continue;
        }
        // If the runner has one ore more trigger words it can set the matchRegex to prevent
        // thread spawning if the pattern does not match
        if (!d->singleMode && r->hasMatchRegex() && !r->matchRegex().match(term).hasMatch()) {
            continue;
        }

        d->currentJobs.insert(jobId);
        d->startJob(r);
    }
    // In the unlikely case that no runner gets queried we have to emit the signals here
    if (d->currentJobs.isEmpty()) {
        QTimer::singleShot(0, this, [this]() {
            d->currentJobs.clear();
            Q_EMIT matchesChanged({});
            Q_EMIT queryFinished();
        });
    }
}

QString RunnerManager::query() const
{
    return d->context.query();
}

QStringList RunnerManager::history() const
{
    return d->readHistoryForCurrentEnv();
}

void RunnerManager::removeFromHistory(int index)
{
    QStringList changedHistory = history();
    if (index < changedHistory.length()) {
        changedHistory.removeAt(index);
        d->writeHistory(changedHistory);
    }
}

QString RunnerManager::getHistorySuggestion(const QString &typedQuery) const
{
    const QStringList historyList = history();
    for (const QString &entry : historyList) {
        if (entry.startsWith(typedQuery, Qt::CaseInsensitive)) {
            return entry;
        }
    }
    return QString();
}

void RunnerManager::reset()
{
    if (!d->currentJobs.empty()) {
        Q_EMIT queryFinished();
        d->currentJobs.clear();
    }
    d->context.reset();
}

KPluginMetaData RunnerManager::convertDBusRunnerToJson(const QString &filename) const
{
    return parseMetaDataFromDesktopFile(filename);
}

bool RunnerManager::historyEnabled()
{
    return d->historyEnabled;
}

void RunnerManager::setHistoryEnabled(bool enabled)
{
    d->historyEnabled = enabled;
    Q_EMIT historyEnabledChanged();
}

// Gets called by RunnerContext to inform that we got new matches
void RunnerManager::onMatchesChanged()
{
    d->scheduleMatchesChanged();
}
void RunnerManager::setHistoryEnvironmentIdentifier(const QString &identifier)
{
    Q_ASSERT(!identifier.isEmpty());
    d->historyEnvironmentIdentifier = identifier;
}

} // KRunner namespace

#include "moc_runnermanager.cpp"
