/*
    SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2016-2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2022 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "systemsettingsrunner.h"

#include <algorithm>

#include <QMimeData>
#include <QUrl>
#include <QUrlQuery>

#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <KSycoca>
#include <LingmoActivities/ResourceInstance>

#include "../app/kcmmetadatahelpers.h"

K_PLUGIN_CLASS_WITH_JSON(SystemsettingsRunner, "systemsettingsrunner.json")

SystemsettingsRunner::SystemsettingsRunner(QObject *parent, const KPluginMetaData &metaData)
    : KRunner::AbstractRunner(parent, metaData)
{
    addSyntax(QStringLiteral(":q:"), i18n("Finds system settings modules whose names or descriptions match :q:"));
    connect(this, &SystemsettingsRunner::prepare, this, [this]() {
        m_modules = findKCMsMetaData(MetaDataSource::All);
    });
    connect(this, &SystemsettingsRunner::teardown, this, [this]() {
        m_modules.clear();
    });
}

void SystemsettingsRunner::match(KRunner::RunnerContext &context)
{
    QList<KRunner::QueryMatch> matches;
    const QString query = context.query();
    const QStringList queryWords{query.split(QLatin1Char(' '))};
    for (const KPluginMetaData &data : std::as_const(m_modules)) {
        qreal relevance = -1;
        const auto checkMatchAndRelevance = [&query, &relevance, &queryWords](const QString &value, qreal relevanceValue) {
            if (value.startsWith(query, Qt::CaseInsensitive)) {
                relevance = relevanceValue + 0.1;
                return true;
            }
            for (const QString &queryWord : queryWords) {
                if (!value.contains(queryWord, Qt::CaseInsensitive)) {
                    return false;
                }
            }
            relevance = relevanceValue;
            return true;
        };

        const QString name = data.name();
        const QString description = data.description();
        const QStringList keywords = data.value(QStringLiteral("X-KDE-Keywords")).split(QLatin1Char(','));
        // check for matches and set relevance
        if (query.length() < 3) {
            if (name.startsWith(query, Qt::CaseInsensitive)) {
                relevance = 0.9;
            } else {
                continue;
            }
        } else if (name.compare(query, Qt::CaseInsensitive) == 0) {
            relevance = 1;
        } else if (checkMatchAndRelevance(name, 0.8)) { // name starts with query or contains all query words
        } else if (checkMatchAndRelevance(description, 0.5)) { // description starts with query or contains all query words
        } else if (std::any_of(keywords.begin(), keywords.end(), [&query](const QString &keyword) {
                       return keyword.startsWith(query, Qt::CaseInsensitive);
                   })) {
            if (keywords.contains(query, Qt::CaseInsensitive)) { // any of the keywords matches query
                relevance = 0.5;
            } else { // any of the keywords starts with query
                relevance = 0.2;
            }
        } else { // none of the properties matches
            continue; // skip this KCM
        }

        KRunner::QueryMatch::CategoryRelevance categoryRelevance = KRunner::QueryMatch::CategoryRelevance::Low;
        if (name.compare(query, Qt::CaseInsensitive) == 0) { // name matches exactly
            categoryRelevance = KRunner::QueryMatch::CategoryRelevance::Highest;
        } else if (name.startsWith(query, Qt::CaseInsensitive)) {
            categoryRelevance = KRunner::QueryMatch::CategoryRelevance::High;
        } else if (description.startsWith(query, Qt::CaseInsensitive)) {
            categoryRelevance = KRunner::QueryMatch::CategoryRelevance::Moderate;
        } else if (keywords.contains(query, Qt::CaseInsensitive)) { // any of the keywords matches exactly
            categoryRelevance = KRunner::QueryMatch::CategoryRelevance::Moderate;
        }

        KRunner::QueryMatch match(this);
        match.setText(name);
        match.setUrls({QUrl(QLatin1String("applications://") + data.pluginId())});
        match.setSubtext(description);
        match.setIconName(data.iconName()); // If it is not set, KRunner will fall back to the runner's icon
        match.setId(data.pluginId()); // KRunner needs the id to adjust the relevance for often launched KCMs
        match.setData(QVariant::fromValue(data));
        match.setRelevance(relevance);
        match.setCategoryRelevance(categoryRelevance);

        if (isKinfoCenterKcm(data)) {
            match.setMatchCategory(i18nd("systemsettings", "System Information"));
        } else {
            match.setMatchCategory(i18nd("systemsettings", "System Settings"));
        }

        matches << match;
    }
    context.addMatches(matches);
}

void SystemsettingsRunner::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    const auto data = match.data().value<KPluginMetaData>();

    KIO::CommandLauncherJob *job = nullptr;
    if (isKinfoCenterKcm(data)) {
        job = new KIO::CommandLauncherJob(QStringLiteral("kinfocenter"), {data.pluginId()});
        job->setDesktopName(QStringLiteral("org.kde.kinfocenter"));
    } else if (!data.value(QStringLiteral("X-KDE-System-Settings-Parent-Category")).isEmpty()) {
        job = new KIO::CommandLauncherJob(QStringLiteral("systemsettings"), {data.pluginId()});
        job->setDesktopName(QStringLiteral("systemsettings"));
    } else {
        // Systemsettings only uses predefined namespaces that kcmshell5/6 also knows about
        job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell6"), {data.pluginId()});
    }
    auto delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);
    job->start();

    KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("systemsettings:") + data.pluginId()), QStringLiteral("org.kde.krunner"));
}

QMimeData *SystemsettingsRunner::mimeDataForMatch(const KRunner::QueryMatch &match)
{
    const auto value = match.data().value<KPluginMetaData>();
    if (value.isValid()) {
        if (KService::Ptr ptr = KService::serviceByStorageId(value.pluginId() + QLatin1String(".desktop"))) {
            auto data = new QMimeData();
            data->setUrls(QList<QUrl>{QUrl::fromLocalFile(ptr->entryPath())});
            return data;
        }
    }
    return nullptr;
}

#include "systemsettingsrunner.moc"

#include "moc_systemsettingsrunner.cpp"
