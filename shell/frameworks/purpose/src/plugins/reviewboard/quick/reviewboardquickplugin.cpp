/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "reviewboardquickplugin.h"
#include "rbrepositoriesmodel.h"
#include "rbreviewslistmodel.h"
#include "reviewboardrc.h"
#include <qqml.h>

void RBPurposeQuickPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<RepositoriesModel>(uri, 1, 0, "RepositoriesModel");
    qmlRegisterType<ReviewsListModel>(uri, 1, 0, "ReviewsListModel");
    qmlRegisterType<ReviewboardRC>(uri, 1, 0, "ReviewboardRC");
}

#include "moc_reviewboardquickplugin.cpp"
