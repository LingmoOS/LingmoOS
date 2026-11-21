// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "journalstream.h"

int main(int argc, char *argv[])
{
    Lingmo::useOwnJournalStream(QStringLiteral("lingmo-session"));

    Lingmo::Base::EventLoop loop;
    Lingmo::SessionApplication app(argc, argv, loop);

    return app.exec();
}
