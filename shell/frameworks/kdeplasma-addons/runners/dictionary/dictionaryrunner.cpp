/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>
#include <QClipboard>
#include <QEventLoop>
#include <QGuiApplication>
#include <QIcon>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QTimer>

namespace
{
const char CONFIG_TRIGGERWORD[] = "triggerWord";
QMutex s_initMutex;
}

DictionaryRunner::DictionaryRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
}

void DictionaryRunner::reloadConfiguration()
{
    KConfigGroup c = config();
    m_triggerWord = c.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define"));
    if (!m_triggerWord.isEmpty()) {
        m_triggerWord.append(QLatin1Char(' '));
        setTriggerWords({m_triggerWord});
    } else {
        setMatchRegex(QRegularExpression());
    }
    setSyntaxes({RunnerSyntax(i18nc("Dictionary keyword", "%1:q:", m_triggerWord), i18n("Finds the definition of :q:."))});
}

void DictionaryRunner::match(RunnerContext &context)
{
    QString query = context.query();
    if (query.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        query.remove(0, m_triggerWord.length());
    }
    if (query.isEmpty()) {
        return;
    }

    {
        QEventLoop loop;
        QTimer::singleShot(400, &loop, [&loop]() {
            loop.quit();
        });
        loop.exec();
    }
    if (!context.isValid()) {
        return;
    }
    QString returnedQuery;
    QMetaObject::invokeMethod(&m_dictEngine, "requestDefinition", Qt::QueuedConnection, Q_ARG(const QString &, query));
    QEventLoop loop;
    connect(&m_dictEngine, &DictEngine::definitionRecieved, &loop, [&loop, &query, &returnedQuery, &context](const QString &html) {
        returnedQuery = html;
        loop.quit();
    });
    loop.exec();
    if (!context.isValid() || returnedQuery.isEmpty()) {
        return;
    }

    static const QRegularExpression removeHtml(QLatin1String("<[^>]*>"));
    QString definitions(returnedQuery);
    definitions.remove(QLatin1Char('\r')).remove(removeHtml);
    while (definitions.contains(QLatin1String("  "))) {
        definitions.replace(QLatin1String("  "), QLatin1String(" "));
    }
    QStringList lines = definitions.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (lines.length() < 2) {
        return;
    }
    lines.removeFirst();

    QList<QueryMatch> matches;
    int item = 0;
    static const QRegularExpression partOfSpeech(QLatin1String("(?: ([a-z]{1,5})){0,1} [0-9]{1,2}: (.*)"));
    QString lastPartOfSpeech;
    for (const QString &line : std::as_const(lines)) {
        const auto reMatch = partOfSpeech.match(line);
        if (!reMatch.hasMatch()) {
            continue;
        }
        if (!reMatch.capturedView(1).isEmpty()) {
            lastPartOfSpeech = reMatch.captured(1);
        }
        QueryMatch match(this);
        match.setMultiLine(true);
        match.setText(lastPartOfSpeech + QLatin1String(": ") + reMatch.captured(2));
        match.setRelevance(1 - (static_cast<double>(++item) / static_cast<double>(lines.length())));
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Moderate);
        match.setIconName(QStringLiteral("accessories-dictionary"));
        matches.append(match);
    }
    context.addMatches(matches);
}

void DictionaryRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    QString query = context.query();
    if (query.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        query.remove(0, m_triggerWord.length());
    }
    QGuiApplication::clipboard()->setText(query + QLatin1Char(' ') + match.text());
    KNotification::event(KNotification::Notification, name(), i18n("Definition for \"%1\" has been copied to clipboard", query), metadata().iconName());
}

K_PLUGIN_CLASS_WITH_JSON(DictionaryRunner, "plasma-runner-dictionary.json")

#include "dictionaryrunner.moc"
