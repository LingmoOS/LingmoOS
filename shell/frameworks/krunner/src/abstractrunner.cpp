/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2020-2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractrunner.h"
#include "abstractrunner_p.h"

#include <QHash>
#include <QIcon>
#include <QMimeData>
#include <QRegularExpression>
#include <QTimer>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

namespace KRunner
{
AbstractRunner::AbstractRunner(QObject *parent, const KPluginMetaData &pluginMetaData)
    : QObject(nullptr)
    , d(new AbstractRunnerPrivate(this, pluginMetaData))
{
    // By now, runners who do "qobject_cast<Krunner::RunnerManager*>(parent)" should have saved the value
    // By setting the parent to a nullptr, we are allowed to move the object to another thread
    Q_ASSERT(parent);
    setObjectName(pluginMetaData.pluginId()); // Only for debugging purposes

    // Suspend matching while we initialize the runner. Once it is ready, the last query will be run
    QTimer::singleShot(0, this, [this]() {
        init();
        // In case the runner didn't specify anything explicitly, we resume matching after the initialization
        bool doesNotHaveExplicitSuspend = true;
        {
            QReadLocker l(&d->lock);
            doesNotHaveExplicitSuspend = !d->suspendMatching.has_value();
        }
        if (doesNotHaveExplicitSuspend) {
            suspendMatching(false);
        }
    });
}

AbstractRunner::~AbstractRunner() = default;

KConfigGroup AbstractRunner::config() const
{
    KConfigGroup runners(KSharedConfig::openConfig(QStringLiteral("krunnerrc")), QStringLiteral("Runners"));
    return runners.group(id());
}

void AbstractRunner::reloadConfiguration()
{
}

void AbstractRunner::addSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
}

void AbstractRunner::setSyntaxes(const QList<RunnerSyntax> &syntaxes)
{
    d->syntaxes = syntaxes;
}

QList<RunnerSyntax> AbstractRunner::syntaxes() const
{
    return d->syntaxes;
}

QMimeData *AbstractRunner::mimeDataForMatch(const QueryMatch &match)
{
    if (match.urls().isEmpty()) {
        return nullptr;
    }
    QMimeData *result = new QMimeData();
    result->setUrls(match.urls());
    return result;
}

void AbstractRunner::run(const KRunner::RunnerContext & /*search*/, const KRunner::QueryMatch & /*action*/)
{
}

QString AbstractRunner::name() const
{
    return d->translatedName;
}

QString AbstractRunner::id() const
{
    return d->runnerDescription.pluginId();
}

KPluginMetaData AbstractRunner::metadata() const
{
    return d->runnerDescription;
}

void AbstractRunner::init()
{
    reloadConfiguration();
}

bool AbstractRunner::isMatchingSuspended() const
{
    QReadLocker lock(&d->lock);
    return d->suspendMatching.value_or(true);
}

void AbstractRunner::suspendMatching(bool suspend)
{
    QWriteLocker lock(&d->lock);
    if (d->suspendMatching.has_value() && d->suspendMatching.value() == suspend) {
        return;
    }

    d->suspendMatching = suspend;
    if (!suspend) {
        Q_EMIT matchingResumed();
    }
}

int AbstractRunner::minLetterCount() const
{
    return d->minLetterCount;
}

void AbstractRunner::setMinLetterCount(int count)
{
    d->minLetterCount = count;
}

QRegularExpression AbstractRunner::matchRegex() const
{
    return d->matchRegex;
}

void AbstractRunner::setMatchRegex(const QRegularExpression &regex)
{
    d->matchRegex = regex;
    d->hasMatchRegex = regex.isValid() && !regex.pattern().isEmpty();
}

void AbstractRunner::setTriggerWords(const QStringList &triggerWords)
{
    int minTriggerWordLetters = 0;
    QString constructedRegex = QStringLiteral("^");
    for (const QString &triggerWord : triggerWords) {
        // We want to link them with an or
        if (constructedRegex.length() > 1) {
            constructedRegex += QLatin1Char('|');
        }
        constructedRegex += QRegularExpression::escape(triggerWord);
        if (minTriggerWordLetters == 0 || triggerWord.length() < minTriggerWordLetters) {
            minTriggerWordLetters = triggerWord.length();
        }
    }
    // If we can reject the query because of the length we don't need the regex
    setMinLetterCount(minTriggerWordLetters);
    setMatchRegex(QRegularExpression(constructedRegex));
}

bool AbstractRunner::hasMatchRegex() const
{
    return d->hasMatchRegex;
}

void AbstractRunner::matchInternal(KRunner::RunnerContext context)
{
    if (context.isValid()) { // Otherwise, we would just waste resources
        match(context);
    }
    Q_EMIT matchInternalFinished(context.runnerJobId(this));
}
// Suspend the runner while reloading the config
void AbstractRunner::reloadConfigurationInternal()
{
    bool isSuspended = isMatchingSuspended();
    suspendMatching(true);
    reloadConfiguration();
    suspendMatching(isSuspended);
}

} // KRunner namespace

#include "moc_abstractrunner.cpp"
