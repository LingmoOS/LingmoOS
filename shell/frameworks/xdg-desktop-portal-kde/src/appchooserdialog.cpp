/*
 * SPDX-FileCopyrightText: 2017-2019 Red Hat Inc
 * SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText:  2017-2019 Jan Grulich <jgrulich@redhat.com>
 */

#include "appchooserdialog.h"
#include "appchooser_debug.h"

#include <algorithm>
#include <iterator>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>

#include <QApplication>
#include <QDir>
#include <QMimeDatabase>
#include <QSettings>
#include <QStandardPaths>

#include <KApplicationTrader>
#include <KBuildSycocaProgressDialog>
#include <KIO/MimeTypeFinderJob>
#include <KLocalizedString>
#include <KProcess>
#include <KSycoca>

AppChooserDialog::AppChooserDialog(const QStringList &choices,
                                   const QString &lastUsedApp,
                                   const QString &fileName,
                                   const QString &mimeName,
                                   bool autoRemember,
                                   QObject *parent)
    : QuickDialog(parent)
    , m_model(new AppModel(this))
    , m_appChooserData(new AppChooserData(this))
    , m_autoRemember(autoRemember)
{
    QVariantMap props = {
        {"title", i18nc("@title:window", "Choose Application")},
        // fileName is actually the full path, confusingly enough. But showing the
        // whole thing is overkill; let's just show the user the file itself
        {"mainText", xi18nc("@info", "Choose an application to open <filename>%1</filename>", QUrl::fromLocalFile(fileName).fileName())},
    };

    auto filterModel = new AppFilterModel(this);
    filterModel->setSourceModel(m_model);

    m_appChooserData->setFileName(fileName);
    m_appChooserData->setLastUsedApp(lastUsedApp);
    filterModel->setLastUsedApp(lastUsedApp);

    auto findDefaultApp = [this, filterModel]() {
        KService::Ptr defaultService = KApplicationTrader::preferredService(m_appChooserData->mimeName());
        if (defaultService && defaultService->isValid()) {
            QString id = defaultService->desktopEntryName();
            m_appChooserData->setDefaultApp(id);
            filterModel->setDefaultApp(id);
        }
    };

    auto findPreferredApps = [this, choices]() {
        if (!choices.isEmpty()) {
            m_model->setPreferredApps(choices);
            return;
        }
        QStringList choices;
        const KService::List appServices = KApplicationTrader::queryByMimeType(m_appChooserData->mimeName(), [](const KService::Ptr &service) -> bool {
            return service->isValid();
        });
        std::transform(appServices.begin(), appServices.end(), std::back_inserter(choices), [](const KService::Ptr &service) {
            return service ? service->desktopEntryName() : QString();
        });
        m_model->setPreferredApps(choices);
    };

    if (mimeName.isEmpty()) {
        auto job = new KIO::MimeTypeFinderJob(QUrl::fromUserInput(fileName));
        job->setAuthenticationPromptEnabled(false);
        connect(job, &KIO::MimeTypeFinderJob::result, this, [this, job, findDefaultApp, findPreferredApps]() {
            if (job->error() == KJob::NoError) {
                m_appChooserData->setMimeName(job->mimeType());
                findDefaultApp();
                findPreferredApps();
            } else {
                qCWarning(XdgDesktopPortalKdeAppChooser) << "couldn't get mimetype:" << job->errorString();
            }
        });
        job->start();
    } else {
        m_appChooserData->setMimeName(mimeName);
        findDefaultApp();
        findPreferredApps();
    }

    qmlRegisterSingletonInstance<AppFilterModel>("org.kde.xdgdesktopportal", 1, 0, "AppModel", filterModel);
    qmlRegisterSingletonInstance<AppChooserData>("org.kde.xdgdesktopportal", 1, 0, "AppChooserData", m_appChooserData);

    create(QStringLiteral("qrc:/AppChooserDialog.qml"), props);

    connect(m_appChooserData, &AppChooserData::openDiscover, this, &AppChooserDialog::onOpenDiscover);
    connect(m_appChooserData, &AppChooserData::applicationSelected, this, &AppChooserDialog::onApplicationSelected);

    connect(KSycoca::self(), &KSycoca::databaseChanged, this, [this, findDefaultApp, findPreferredApps] {
        m_model->loadApplications();
        findDefaultApp();
        findPreferredApps();
    });
}

QString AppChooserDialog::selectedApplication() const
{
    return m_selectedApplication;
}

void AppChooserDialog::onApplicationSelected(const QString &desktopFile, const bool remember)
{
    m_selectedApplication = desktopFile;

    // When used by the private interface for plasma-integration autoremember is off and plasma-integration takes
    // care of remembering.
    if (m_autoRemember && remember && !m_appChooserData->mimeName().isEmpty()) {
        KService::Ptr serv = KService::serviceByDesktopName(desktopFile);
        KApplicationTrader::setPreferredService(m_appChooserData->mimeName(), serv);
        // kbuildsycoca is the one reading mimeapps.list, so we need to run it now
        KBuildSycocaProgressDialog::rebuildKSycoca(QApplication::activeWindow());
    }

    accept();
}

void AppChooserDialog::onOpenDiscover()
{
    QStringList args;
    if (!m_appChooserData->mimeName().isEmpty()) {
        args << QStringLiteral("--mime") << m_appChooserData->mimeName();
    }
    KProcess::startDetached(QStringLiteral("plasma-discover"), args);
}

void AppChooserDialog::updateChoices(const QStringList &choices)
{
    m_model->setPreferredApps(choices);
}

ApplicationItem::ApplicationItem(const QString &name, const KService::Ptr &service)
    : m_applicationName(name)
    , m_applicationService(service)
    , m_applicationCategory(AllApplications)
{
    const QStringList names = service->mimeTypes();
    const QMimeDatabase database;
    for (const QString &name : names) {
        QMimeType mime = database.mimeTypeForName(name);
        if (mime.isValid()) {
            m_supportedMimeTypes.append(mime);
        }
    }
}

QString ApplicationItem::applicationName() const
{
    return m_applicationName;
}

QString ApplicationItem::applicationGenericName() const
{
    return m_applicationService->genericName();
}

QString ApplicationItem::applicationUntranslatedGenericName() const
{
    return m_applicationService->untranslatedGenericName();
}

QString ApplicationItem::applicationIcon() const
{
    return m_applicationService->icon();
}

QString ApplicationItem::applicationDesktopFile() const
{
    return m_applicationService->desktopEntryName();
}

QList<QMimeType> ApplicationItem::supportedMimeTypes() const
{
    return m_supportedMimeTypes;
}

void ApplicationItem::setApplicationCategory(ApplicationItem::ApplicationCategory category)
{
    m_applicationCategory = category;
}

ApplicationItem::ApplicationCategory ApplicationItem::applicationCategory() const
{
    return m_applicationCategory;
}

bool ApplicationItem::operator==(const ApplicationItem &item) const
{
    return item.applicationDesktopFile() == applicationDesktopFile();
}

AppFilterModel::AppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::DescendingOrder);
}

void AppFilterModel::setShowOnlyPreferredApps(bool show)
{
    if (m_showOnlyPreferredApps == show) {
        return;
    }

    m_showOnlyPreferredApps = show;
    Q_EMIT showOnlyPreferredAppsChanged();
    invalidate();
}

bool AppFilterModel::showOnlyPreferredApps() const
{
    return m_showOnlyPreferredApps;
}

void AppFilterModel::setDefaultApp(const QString &defaultApp)
{
    m_defaultApp = defaultApp;

    invalidate();
}

QString AppFilterModel::defaultApp() const
{
    return m_defaultApp;
}

void AppFilterModel::setLastUsedApp(const QString &lastUsedApp)
{
    if (m_lastUsedApp == lastUsedApp) {
        return;
    }

    m_lastUsedApp = lastUsedApp;
    invalidate();
}

QString AppFilterModel::lastUsedApp() const
{
    return m_lastUsedApp;
}

void AppFilterModel::setFilter(const QString &text)
{
    m_filter = text;

    if (!m_filter.isEmpty() && m_showOnlyPreferredApps) {
        m_showOnlyPreferredApps = false;
        emit showOnlyPreferredAppsChanged();
    }

    invalidate();
}

QString AppFilterModel::filter() const
{
    return m_filter;
}

bool AppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    ApplicationItem::ApplicationCategory category =
        static_cast<ApplicationItem::ApplicationCategory>(sourceModel()->data(index, AppModel::ApplicationCategoryRole).toInt());

    const bool canShowInList = m_showOnlyPreferredApps ? (category == ApplicationItem::PreferredApplication) : true;
    if (!canShowInList) {
        return false;
    }

    if (m_filter.isEmpty()) {
        return true;
    }

    const QString appName = index.data(AppModel::ApplicationNameRole).toString();
    if (appName.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }

    // Match in GenericName
    const QString genericName = index.data(AppModel::ApplicationGenericNameRole).toString();
    if (genericName.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }

    // Match in untranslated GenericName
    const QString untranslatedGenericName = index.data(AppModel::ApplicationUntranslatedGenericNameRole).toString();
    if (untranslatedGenericName.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }

    // Match in MimeTypes
    const auto supportedMimeTypes = index.data(AppModel::ApplicationSupportedMimeTypesRole).value<QList<QMimeType>>();
    return std::any_of(supportedMimeTypes.cbegin(), supportedMimeTypes.cend(), [this, category](const QMimeType &type) {
        if (type.name().contains(m_filter, Qt::CaseInsensitive)) {
            return true;
        }

        const QStringList aliases = type.aliases();
        const bool aliasesMatched = std::any_of(aliases.cbegin(), aliases.cend(), [this](const QString &name) {
            return name.contains(m_filter, Qt::CaseInsensitive);
        });
        if (aliasesMatched) {
            return true;
        }

        const QStringList suffixes = type.suffixes();
        if (suffixes.contains(m_filter) || suffixes.contains(m_filter.mid(m_filter.lastIndexOf(QLatin1Char('.')) + 1))) {
            return true;
        }

        return false;
    });
}

bool AppFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (sourceModel()->data(left, AppModel::ApplicationDesktopFileRole) == m_defaultApp) {
        return false;
    }
    if (sourceModel()->data(right, AppModel::ApplicationDesktopFileRole) == m_defaultApp) {
        return true;
    }

    const QString leftName = left.data(AppModel::ApplicationNameRole).toString();
    const QString rightName = right.data(AppModel::ApplicationNameRole).toString();

    // Prioritize name match when filter is not empty
    if (!m_filter.isEmpty()) {
        const bool leftMatched = leftName.contains(m_filter, Qt::CaseInsensitive);
        const bool rightMatched = rightName.contains(m_filter, Qt::CaseInsensitive);
        if (leftMatched && !rightMatched) {
            return false;
        } else if (!leftMatched && rightMatched) {
            return true;
        }
    }

    if (sourceModel()->data(left, AppModel::ApplicationDesktopFileRole) == m_lastUsedApp) {
        return false;
    }
    if (sourceModel()->data(right, AppModel::ApplicationDesktopFileRole) == m_lastUsedApp) {
        return true;
    }
    ApplicationItem::ApplicationCategory leftCategory =
        static_cast<ApplicationItem::ApplicationCategory>(sourceModel()->data(left, AppModel::ApplicationCategoryRole).toInt());
    ApplicationItem::ApplicationCategory rightCategory =
        static_cast<ApplicationItem::ApplicationCategory>(sourceModel()->data(right, AppModel::ApplicationCategoryRole).toInt());

    if (int comp = leftCategory - rightCategory; comp != 0) {
        return comp > 0;
    }

    return QString::localeAwareCompare(leftName, rightName) > 0;
}

QString AppChooserData::defaultApp() const
{
    return m_defaultApp;
}

void AppChooserData::setDefaultApp(const QString &defaultApp)
{
    m_defaultApp = defaultApp;
    Q_EMIT defaultAppChanged();
}

QString AppChooserData::lastUsedApp() const
{
    return m_lastUsedApp;
}

void AppChooserData::setLastUsedApp(const QString &lastUsedApp)
{
    if (m_lastUsedApp == lastUsedApp) {
        return;
    }

    m_lastUsedApp = lastUsedApp;
    Q_EMIT lastUsedAppChanged();
}

AppChooserData::AppChooserData(QObject *parent)
    : QObject(parent)
{
}

QString AppChooserData::fileName() const
{
    return m_fileName;
}

void AppChooserData::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    Q_EMIT fileNameChanged();
}

QString AppChooserData::mimeName() const
{
    return m_mimeName;
}

void AppChooserData::setMimeName(const QString &mimeName)
{
    if (m_mimeName != mimeName) {
        m_mimeName = mimeName;
        Q_EMIT mimeNameChanged();
        Q_EMIT mimeDescChanged();
    }
}

QString AppChooserData::mimeDesc() const
{
    return QMimeDatabase().mimeTypeForName(m_mimeName).comment();
}

AppModel::AppModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadApplications();
}

void AppModel::setPreferredApps(const QStringList &possiblyAliasedList)
{
    m_hasPreferredApps = false;
    Q_EMIT hasPreferredAppsChanged();

    // In the event that we get incoming NoDisplay entries that are AliasFor another desktop file,
    // switch the NoDisplay name for the aliased name.
    QStringList list;
    for (const auto &entry : possiblyAliasedList) {
        if (const auto value = m_noDisplayAliasesFor.value(entry); !value.isEmpty()) {
            list << value;
        } else {
            list << entry;
        }
    }

    for (ApplicationItem &item : m_list) {
        bool changed = false;

        // First reset to initial type
        if (item.applicationCategory() != ApplicationItem::AllApplications) {
            item.setApplicationCategory(ApplicationItem::AllApplications);
            changed = true;
        }

        if (list.contains(item.applicationDesktopFile())) {
            item.setApplicationCategory(ApplicationItem::PreferredApplication);
            changed = true;
            m_hasPreferredApps = true;
            Q_EMIT hasPreferredAppsChanged();
        }

        if (changed) {
            const int row = m_list.indexOf(item);
            if (row >= 0) {
                QModelIndex index = createIndex(row, 0, AppModel::ApplicationCategoryRole);
                Q_EMIT dataChanged(index, index);
            }
        }
    }
}

QVariant AppModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    if (row >= 0 && row < m_list.count()) {
        const ApplicationItem &item = m_list.at(row);

        switch (role) {
        case ApplicationNameRole:
            return item.applicationName();
        case ApplicationGenericNameRole:
            return item.applicationGenericName();
        case ApplicationUntranslatedGenericNameRole:
            return item.applicationUntranslatedGenericName();
        case ApplicationIconRole:
            return item.applicationIcon();
        case ApplicationDesktopFileRole:
            return item.applicationDesktopFile();
        case ApplicationCategoryRole:
            return static_cast<int>(item.applicationCategory());
        case ApplicationSupportedMimeTypesRole:
            return QVariant::fromValue(item.supportedMimeTypes());
        default:
            break;
        }
    }

    return {};
}

int AppModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_list.count();
}

QHash<int, QByteArray> AppModel::roleNames() const
{
    return {
        {ApplicationNameRole, QByteArrayLiteral("applicationName")},
        {ApplicationIconRole, QByteArrayLiteral("applicationIcon")},
        {ApplicationDesktopFileRole, QByteArrayLiteral("applicationDesktopFile")},
        {ApplicationCategoryRole, QByteArrayLiteral("applicationCategory")},
    };
}

void AppModel::loadApplications()
{
    beginResetModel();
    m_list.clear();
    m_noDisplayAliasesFor.clear();

    const KService::List appServices = KApplicationTrader::query([](const KService::Ptr &service) -> bool {
        return service->isValid();
    });
    for (const KService::Ptr &service : appServices) {
        if (service->noDisplay()) {
            if (const auto alias = service->aliasFor(); !alias.isEmpty()) {
                m_noDisplayAliasesFor.insert(service->desktopEntryName(), service->aliasFor());
            }
            continue; // no display after all
        }

        const QString fullName = service->property<QString>(QStringLiteral("X-GNOME-FullName"));
        const QString name = fullName.isEmpty() ? service->name() : fullName;
        ApplicationItem appItem(name, service);

        if (!m_list.contains(appItem)) {
            m_list.append(appItem);
        }
    }

    endResetModel();
}

void AppChooserData::setHistory(const QStringList &history)
{
    m_history = history;
    Q_EMIT historyChanged();
}

void AppChooserData::setShellAccess(bool enable)
{
    m_shellAccess = enable;
    Q_EMIT shellAccessChanged();
}
