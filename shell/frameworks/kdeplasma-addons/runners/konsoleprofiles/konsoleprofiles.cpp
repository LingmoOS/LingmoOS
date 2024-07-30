/*
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *   based on kate session from sebas
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "konsoleprofiles.h"

// KF
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

KonsoleProfiles::KonsoleProfiles(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
    addSyntax({QStringLiteral(":q:"), QStringLiteral("konsole :q:")}, i18n("Finds Konsole profiles matching :q:."));
    addSyntax(QStringLiteral("konsole"), i18n("Lists all the Konsole profiles in your account."));
    setTriggerWords({m_triggerWord});
}

void KonsoleProfiles::match(RunnerContext &context)
{
    QString term = context.query();
    term = term.remove(m_triggerWord).simplified();
    for (int i = 0, count = m_model->rowCount(); i < count; ++i) {
        QModelIndex idx = m_model->index(i);
        const QString name = idx.data(ProfilesModel::NameRole).toString();
        if (name.contains(term, Qt::CaseInsensitive)) {
            const QString profileIdentifier = idx.data(ProfilesModel::ProfileIdentifierRole).toString();
            QueryMatch match(this);
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Low);
            match.setIconName(idx.data(ProfilesModel::IconNameRole).toString());
            match.setData(profileIdentifier);
            match.setText(QStringLiteral("Konsole: ") + name);
            match.setRelevance((float)term.length() / (float)name.length());
            context.addMatch(match);
        }
    }
}

void KonsoleProfiles::run(const RunnerContext & /*context*/, const QueryMatch &match)
{
    const QString profile = match.data().toString();
    m_model->openProfile(profile);
}

void KonsoleProfiles::init()
{
    // Only create this in the correct thread. Inside we use KDirWatch which is thread sensitive.
    m_model = new ProfilesModel(this);
    m_model->setAppName(m_triggerWord);
}

K_PLUGIN_CLASS_WITH_JSON(KonsoleProfiles, "plasma-runner-konsoleprofiles.json")

#include "konsoleprofiles.moc"
