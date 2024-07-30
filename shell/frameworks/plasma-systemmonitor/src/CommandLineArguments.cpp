/**
 * SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "CommandLineArguments.h"

using namespace Qt::StringLiterals;

CommandLineArguments::CommandLineArguments(QObject *parent)
    : QObject(parent)
{
}

QString CommandLineArguments::pageId() const
{
    if (s_commandLineParser) {
        return s_commandLineParser->value(u"page-id"_s);
    }

    return QString{};
}

QString CommandLineArguments::pageName() const
{
    if (s_commandLineParser) {
        return s_commandLineParser->value(u"page-name"_s);
    }

    return QString{};
}

QVariant CommandLineArguments::aboutData() const
{
    return QVariant::fromValue(KAboutData::applicationData());
}

void CommandLineArguments::setCommandLineParser(std::shared_ptr<QCommandLineParser> parser)
{
    s_commandLineParser = parser;
}
