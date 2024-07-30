/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_STATUSCOMMAND_H
#define BALOO_STATUSCOMMAND_H

#include "command.h"

namespace Baloo {

class StatusCommand : public Command
{
public:
    QString command() override;
    QString description() override;

    int exec(const QCommandLineParser& parser) override;
};
}

#endif // BALOO_STATUSCOMMAND_H
