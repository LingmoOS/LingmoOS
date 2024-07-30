/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_COMMAND_H
#define BALOO_COMMAND_H

#include <QString>
#include <QCommandLineParser>

namespace Baloo {

/**
 * An Abstract class from which all other balooctl commands can inherit from
 */
class Command
{
public:
    virtual ~Command();

    virtual QString command() = 0;
    virtual QString description() = 0;

    virtual int exec(const QCommandLineParser& parser) = 0;

};

}

#endif // BALOO_COMMAND_H
