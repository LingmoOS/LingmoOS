/*
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENGINEBASE_P_H
#define KNEWSTUFF3_ENGINEBASE_P_H

#include "cache.h"
#include "enginebase.h"
#include "installation_p.h"
#include <Attica/ProviderManager>

class KNSCore::EngineBasePrivate
{
public:
    QString name;
    QStringList categories;
    QString adoptionCommand;
    QString useLabel;
    bool uploadEnabled = false;
    QUrl providerFileUrl;
    QStringList tagFilter;
    QStringList downloadTagFilter;
    Installation *installation = new Installation();
    Attica::ProviderManager *atticaProviderManager = nullptr;
    QList<Provider::SearchPreset> searchPresets;
    QSharedPointer<Cache> cache;
    bool shouldRemoveDeletedEntries = false;
    QList<Provider::CategoryMetadata> categoriesMetadata;
    QHash<QString, QSharedPointer<KNSCore::Provider>> providers;
    KNSCore::EngineBase::ContentWarningType contentWarningType = KNSCore::EngineBase::ContentWarningType::Static;
};

#endif
