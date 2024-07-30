/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kprocesstest_helper.h"
#include <kprocess.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Missing parameter");
        return -1;
    }
    KProcess p;
    p.setShellCommand(QString::fromLatin1("echo " EOUT "; echo " EERR " >&2"));
    p.setOutputChannelMode(static_cast<KProcess::OutputChannelMode>(atoi(argv[1])));
    fputs(POUT, stdout);
    fflush(stdout);
    p.execute();
    fputs(ROUT, stdout);
    fputs(p.readAllStandardOutput().constData(), stdout);
    fputs(RERR, stdout);
    if (p.outputChannelMode() != KProcess::MergedChannels) {
        fputs(p.readAllStandardError().constData(), stdout);
    }
    return 0;
}
