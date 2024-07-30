/*
 *   SPDX-FileCopyrightText: 2007 Ryan Bitanga <ephebiphobic@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <sonnet/speller.h>

#include <KRunner/AbstractRunner>
#include <QMutex>
#include <QSharedPointer>

using namespace KRunner;

/**
 * This checks the spelling of query
 */
class SpellCheckRunner : public AbstractRunner
{
    Q_OBJECT

public:
    SpellCheckRunner(QObject *parent, const KPluginMetaData &metaData);
    ~SpellCheckRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &action) override;

    void reloadConfiguration() override;
    QMimeData *mimeDataForMatch(const QueryMatch &match) override;

private:
    void loadData();
    QString findLang(const QStringList &terms);

    QString m_triggerWord;
    QStringList m_availableLangCodes; // en_US, ru_RU, ...
    QMap<QString, QString> m_languages; // key=language name, value=language code
    bool m_requireTriggerWord;
    QMap<QString, QSharedPointer<Sonnet::Speller>> m_spellers; // spellers

    friend class SpellCheckRunnerTest;
};

#endif
