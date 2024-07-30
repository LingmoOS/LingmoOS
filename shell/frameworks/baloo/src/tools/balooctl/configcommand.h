/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_CONFIGCOMMAND_H
#define BALOO_CONFIGCOMMAND_H

#include "command.h"

namespace Baloo {

class ConfigCommand : public Baloo::Command
{
public:
    QString command() override;
    QString description() override;

    int exec(const QCommandLineParser& parser) override;
};
}

#endif // BALOO_CONFIGCOMMAND_H
