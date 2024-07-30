/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "lingmo-desktop-runner.h"

#include <KIO/CommandLauncherJob>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

#include <QDebug>

#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(LingmoDesktopRunner, "lingmo-runner-lingmo-desktop.json")

static const QString s_lingmoService = QLatin1String("org.kde.lingmoshell");

LingmoDesktopRunner::LingmoDesktopRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
    , m_desktopConsoleKeyword(i18nc("Note this is a KRunner keyword", "desktop console"))
    , m_kwinConsoleKeyword(i18nc("Note this is a KRunner keyword", "wm console"))
{
    setObjectName(QLatin1String("Lingmo-Desktop"));
    addSyntax(m_desktopConsoleKeyword, i18n("Opens the Lingmo desktop interactive console with a file path to a script on disk."));
    addSyntax(i18nc("Note this is a KRunner keyword", "desktop console :q:"),
              i18n("Opens the Lingmo desktop interactive console with a file path to a script on disk."));
    addSyntax(m_kwinConsoleKeyword, i18n("Opens the KWin interactive console with a file path to a script on disk."));
    addSyntax(i18nc("Note this is a KRunner keyword", "wm console :q:"), i18n("Opens the KWin interactive console with a file path to a script on disk."));
}

void LingmoDesktopRunner::match(RunnerContext &context)
{
    if (context.query().startsWith(m_desktopConsoleKeyword, Qt::CaseInsensitive)) {
        QueryMatch match(this);
        match.setId(QStringLiteral("lingmo-desktop-console"));
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        match.setIconName(QStringLiteral("lingmo"));
        match.setText(i18n("Open Lingmo desktop interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
    if (context.query().startsWith(m_kwinConsoleKeyword, Qt::CaseInsensitive)) {
        QueryMatch match(this);
        match.setId(QStringLiteral("lingmo-desktop-console"));
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        match.setIconName(QStringLiteral("kwin"));
        match.setText(i18n("Open KWin interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
}

void LingmoDesktopRunner::run(const RunnerContext &context, const QueryMatch & /*match*/)
{
    bool showLingmoConsole = context.query().startsWith(m_desktopConsoleKeyword, Qt::CaseInsensitive);
    QStringList args{showLingmoConsole ? QStringLiteral("--lingmo") : QStringLiteral("--kwin")};
    auto job = new KIO::CommandLauncherJob(QStringLiteral("lingmo-interactiveconsole"), args);
    job->start();
}

#include "lingmo-desktop-runner.moc"
