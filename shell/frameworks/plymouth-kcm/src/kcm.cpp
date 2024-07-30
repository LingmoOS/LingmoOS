/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *  SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "kcm.h"
#include "config-kcm.h"

#include <chrono>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

#include <KIO/FileCopyJob>

using namespace std::chrono_literals;

K_PLUGIN_CLASS_WITH_JSON(KCMPlymouth, "kcm_plymouth.json")

KCMPlymouth::KCMPlymouth(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData)
    , m_model(new QStandardItemModel(this))
{
    qmlRegisterAnonymousType<QStandardItemModel>("KCMPlymouth", 1);
    qmlRegisterAnonymousType<KCMPlymouth>("KCMPlymouth", 1);
    setButtons(Apply);
    setAuthActionName(QStringLiteral("org.kde.kcontrol.kcmplymouth.save"));

    m_model->setItemRoleNames({
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {PluginNameRole, QByteArrayLiteral("pluginName")},
        {ScreenhotRole, QByteArrayLiteral("screenshot")},
        {UninstallableRole, QByteArrayLiteral("uninstallable")},
    });
}

KCMPlymouth::~KCMPlymouth()
{
}

void KCMPlymouth::reloadModel()
{
    m_model->clear();

    QDir dir(QStringLiteral(PLYMOUTH_THEMES_DIR));
    if (!dir.exists()) {
        return;
    }

    KConfigGroup installedCg(KSharedConfig::openConfig(QStringLiteral("kplymouththemeinstallerrc")), QStringLiteral("DownloadedThemes"));

    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);

    const auto list = dir.entryInfoList();
    for (const QFileInfo &fileInfo : list) {
        const QString pluginName = fileInfo.fileName();
        QDir themeDir(fileInfo.filePath());

        KConfig file(themeDir.filePath(pluginName + QLatin1String(".plymouth")), KConfig::SimpleConfig);
        KConfigGroup grp = file.group(QStringLiteral("Plymouth Theme"));

        QString displayName = grp.readEntry("Name", QString());
        if (displayName.isEmpty()) {
            displayName = pluginName;
        }

        QStandardItem *row = new QStandardItem(displayName);
        row->setData(pluginName, PluginNameRole);
        row->setData(grp.readEntry("Description", QString()), DescriptionRole);
        row->setData(installedCg.entryMap().contains(fileInfo.fileName()), UninstallableRole);

        // the theme has a preview
        if (QFile::exists(themeDir.path() + QStringLiteral("/preview.png"))) {
            row->setData(QUrl::fromLocalFile(themeDir.path() + QStringLiteral("/preview.png")), ScreenhotRole);
            // fetch it downloaded from kns
        } else {
            const QString fileName = installedCg.readEntry(fileInfo.fileName(), QString());
            if (fileName.isEmpty()) {
                row->setData(QString(), ScreenhotRole);
            } else {
                row->setData(QUrl::fromLocalFile(fileName + QStringLiteral(".png")), ScreenhotRole);
            }
        }

        m_model->appendRow(row);
    }

    Q_EMIT selectedPluginIndexChanged();
}

void KCMPlymouth::onEntryEvent(const KNSCore::Entry &entry)
{
    static QStringList alreadyCopiedThumbnails;
    if (entry.isValid() && entry.status() == KNSCore::Entry::Installed && !alreadyCopiedThumbnails.contains(entry.uniqueId())) {
        alreadyCopiedThumbnails.append(entry.uniqueId());
        KIO::file_copy(QUrl(entry.previewUrl(KNSCore::Entry::PreviewBig1)),
                       QUrl::fromLocalFile(QString(entry.installedFiles().constFirst() + QStringLiteral(".png"))),
                       -1,
                       KIO::Overwrite | KIO::HideProgressInfo);
    }
    reloadModel();
}

QStandardItemModel *KCMPlymouth::themesModel()
{
    return m_model;
}

QString KCMPlymouth::selectedPlugin() const
{
    return m_selectedPlugin;
}

void KCMPlymouth::setSelectedPlugin(const QString &plugin)
{
    if (m_selectedPlugin == plugin) {
        return;
    }

    m_selectedPlugin = plugin;
    Q_EMIT selectedPluginChanged();
    Q_EMIT selectedPluginIndexChanged();

    setNeedsSave(true);
}

bool KCMPlymouth::busy() const
{
    return m_busy;
}

void KCMPlymouth::setBusy(const bool &busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    Q_EMIT busyChanged();
}

int KCMPlymouth::selectedPluginIndex() const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->data(m_model->index(i, 0), PluginNameRole).toString() == m_selectedPlugin) {
            return i;
        }
    }
    return -1;
}

void KCMPlymouth::load()
{
    reloadModel();

    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral(PLYMOUTH_CONFIG_PATH)), QStringLiteral("Daemon"));

    setSelectedPlugin(cg.readEntry("Theme"));

    setNeedsSave(false);
}

void KCMPlymouth::save()
{
    setBusy(true);
    QVariantMap helperargs;
    helperargs[QStringLiteral("theme")] = m_selectedPlugin;

    KAuth::Action action(authActionName());
    action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmplymouth"));
    action.setArguments(helperargs);
    // We don't know how long this will take. The helper will need to generate N=installed_kernels initrds.
    // Be very generous with the timeout! https://bugs.kde.org/show_bug.cgi?id=400641
    // NB: there is also a timeout in the helper
    action.setTimeout(std::chrono::milliseconds(15min).count());

    KAuth::ExecuteJob *job = action.execute();
    bool rc = job->exec();
    if (!rc) {
        if (job->error() == KAuth::ActionReply::UserCancelledError) {
            Q_EMIT showErrorMessage(i18n("Unable to authenticate/execute the action: %1 (%2)", job->error(), job->errorString()));
        }
        load();
    }
    setBusy(false);
}

void KCMPlymouth::uninstall(const QString &plugin)
{
    QVariantMap helperargs;
    helperargs[QStringLiteral("theme")] = plugin;

    // KAuth::Action action(authActionName());
    KAuth::Action action(QStringLiteral("org.kde.kcontrol.kcmplymouth.uninstall"));
    action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmplymouth"));
    action.setArguments(helperargs);

    KAuth::ExecuteJob *job = action.execute();
    bool rc = job->exec();
    if (!rc) {
        Q_EMIT showErrorMessage(i18n("Unable to authenticate/execute the action: %1 (%2)", job->error(), job->errorString()));
    } else {
        KConfigGroup installedCg(KSharedConfig::openConfig(QStringLiteral("kplymouththemeinstallerrc")), QStringLiteral("DownloadedThemes"));
        installedCg.deleteEntry(plugin);
        Q_EMIT showSuccessMessage(i18n("Theme uninstalled successfully."));
        load();
    }
}

void KCMPlymouth::defaults()
{ /*TODO
     if (!) {
         return;
     }
 */
}

#include "kcm.moc"

#include "moc_kcm.cpp"
