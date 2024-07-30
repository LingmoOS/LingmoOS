/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reviewboardrc.h"
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

ReviewboardRC::ReviewboardRC(QObject *parent)
    : QObject(parent)
{
}

void ReviewboardRC::setPath(const QUrl &filePath)
{
    if (filePath == m_path || !filePath.isLocalFile()) {
        return;
    }

    // The .reviewboardrc files are python files, we'll read and if it doesn't work
    // Well bad luck. See: http://www.reviewboard.org/docs/rbtools/dev/rbt/configuration/

    QFile f(filePath.toLocalFile());
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "couldn't open" << filePath;
        return;
    }

    const QRegularExpression rx(QRegularExpression::anchoredPattern(QStringLiteral("([\\w]+) *= *[\"'](.*)[\"']")));
    QHash<QString, QString> values;
    QTextStream stream(&f);
    while (!stream.atEnd()) {
        QRegularExpressionMatch match = rx.match(stream.readLine());
        if (match.hasMatch()) {
            values.insert(match.captured(1), match.captured(2));
        }
    }

    if (values.contains(QStringLiteral("REVIEWBOARD_URL"))) {
        m_server = QUrl(values[QStringLiteral("REVIEWBOARD_URL")]);
    }
    if (values.contains(QStringLiteral("REPOSITORY"))) {
        m_repository = values[QStringLiteral("REPOSITORY")];
    }
    addExtraData(QStringLiteral("target_groups"), values[QStringLiteral("TARGET_GROUPS")]);
    addExtraData(QStringLiteral("target_people"), values[QStringLiteral("TARGET_PEOPLE")]);
    addExtraData(QStringLiteral("branch"), values[QStringLiteral("BRANCH")]);

    Q_EMIT dataChanged();
}

void ReviewboardRC::addExtraData(const QString &key, const QString &value)
{
    if (!value.isEmpty()) {
        m_extraData.insert(key, value);
    }
}

#include "moc_reviewboardrc.cpp"
