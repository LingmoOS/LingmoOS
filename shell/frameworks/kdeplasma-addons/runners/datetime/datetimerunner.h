/*
 *   SPDX-FileCopyrightText: 2010 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef DATETIMERUNNER_H
#define DATETIMERUNNER_H

#include <QDateTime>

#include <KRunner/AbstractRunner>
#include <KRunner/QueryMatch>
#include <QDateTime>
#include <QTimeZone>

using namespace KRunner;

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent, const KPluginMetaData &metaData);
    ~DateTimeRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private:
    QHash<QString, QTimeZone> matchingTimeZones(const QStringView &zoneTerm = QStringView(), const QDateTime referenceDatetime = QDateTime::currentDateTime());
    void addMatch(const QString &text, const QString &clipboardText, const qreal &relevance, const QString &iconName, RunnerContext &context);
};

#endif
