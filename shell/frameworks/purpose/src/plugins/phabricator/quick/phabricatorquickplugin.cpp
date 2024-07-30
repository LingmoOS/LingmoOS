/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "phabricatorquickplugin.h"
#include "difflistmodel.h"
#include "phabricatorrc.h"
#include <qqml.h>

void PhabricatorPurposeQuickPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<DiffListModel>(uri, 1, 0, "DiffListModel");
    qmlRegisterType<PhabricatorRC>(uri, 1, 0, "PhabricatorRC");
}

#include "moc_phabricatorquickplugin.cpp"
