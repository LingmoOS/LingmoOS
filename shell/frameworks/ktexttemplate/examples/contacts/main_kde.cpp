/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "mainwindow.h"

#include "kdelocalizer.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "ktexttemplate_paths.h"

#define TEMPLATE_DIR KTEXTTEMPLATE_TEMPLATE_PATH "/gettext/"

int main(int argc, char **argv)
{
    const QByteArray &ba = QByteArray("contacts_kde");
    const KLocalizedString name = ki18n("KTextTemplate KDE i18n example");
    KAboutData aboutData(ba, ba, name, ba, name);
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    AppMainWindow<KDELocalizer> win(TEMPLATE_DIR);
    win.show();

    app.exec();
}
