/*
    SPDX-FileCopyrightText: 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lingmoappletitemmodel_p.h"

#include <QFileInfo>
#include <QMimeData>
#include <QStandardPaths>
#include <QVersionNumber>

#include "config-workspace.h"
#include <KAboutData>
#include <KConfig>
#include <KJsonUtils>
#include <KLocalizedString>
#include <KPackage/PackageLoader>
#include <KRuntimePlatform>

using namespace Qt::StringLiterals;

LingmoAppletItem::LingmoAppletItem(const KPluginMetaData &info)
    : AbstractItem()
    , m_info(info)
    , m_runningCount(0)
    , m_local(false)
{
    const QString _f = QStringLiteral(LINGMO_RELATIVE_DATA_INSTALL_DIR) + u"/plasmoids/" + info.pluginId() + u'/';
    QFileInfo dir(QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, _f, QStandardPaths::LocateDirectory));
    m_local = dir.exists() && dir.isWritable();

    setText(QString(m_info.name() + u" - " + m_info.category().toLower()));

    // set plugininfo parts as roles in the model, only way qml can understand it
    setData(name(), LingmoAppletItemModel::NameRole);
    setData(pluginName(), LingmoAppletItemModel::PluginNameRole);
    setData(description(), LingmoAppletItemModel::DescriptionRole);
    setData(category().toLower(), LingmoAppletItemModel::CategoryRole);
    setData(license(), LingmoAppletItemModel::LicenseRole);
    setData(website(), LingmoAppletItemModel::WebsiteRole);
    setData(version(), LingmoAppletItemModel::VersionRole);
    setData(author(), LingmoAppletItemModel::AuthorRole);
    setData(email(), LingmoAppletItemModel::EmailRole);
    setData(apiVersion(), LingmoAppletItemModel::ApiVersionRole);
    setData(isSupported(), LingmoAppletItemModel::IsSupportedRole);
    setData(unsupportedMessage(), LingmoAppletItemModel::UnsupportedMessageRole);
    setData(0, LingmoAppletItemModel::RunningRole);
    setData(m_local, LingmoAppletItemModel::LocalRole);

    QString iconName;

    if (QIcon::hasThemeIcon(m_info.pluginId())) {
        iconName = m_info.pluginId();
    } else if (!m_info.iconName().isEmpty()) {
        if (m_info.iconName().startsWith(QLatin1String("/"))) {
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Applet"));
            pkg.setDefaultPackageRoot(QStringLiteral("lingmo/plasmoids"));
            pkg.setPath(m_info.pluginId());
            if (pkg.isValid()) {
                iconName = pkg.filePath("", pkg.metadata().iconName());
            }
        } else {
            iconName = m_info.iconName();
        }
    } else {
        iconName = QStringLiteral("application-x-lingmo");
    }

    // Do not use setIcon here, we need to pass the icon name to the delegate, otherwise LingmoUI.Icon will load it with the wrong colors
    setData(iconName, Qt::DecorationRole);
}

QString LingmoAppletItem::pluginName() const
{
    return m_info.pluginId();
}

QString LingmoAppletItem::name() const
{
    return m_info.name();
}

QString LingmoAppletItem::description() const
{
    return m_info.description();
}

QString LingmoAppletItem::license() const
{
    return m_info.license();
}

QString LingmoAppletItem::category() const
{
    return m_info.category();
}

QString LingmoAppletItem::website() const
{
    return m_info.website();
}

QString LingmoAppletItem::version() const
{
    return m_info.version();
}

QString LingmoAppletItem::author() const
{
    if (m_info.authors().isEmpty()) {
        return QString();
    }

    return m_info.authors().constFirst().name();
}

QString LingmoAppletItem::email() const
{
    if (m_info.authors().isEmpty()) {
        return QString();
    }

    return m_info.authors().constFirst().emailAddress();
}

int LingmoAppletItem::running() const
{
    return m_runningCount;
}

void LingmoAppletItem::setRunning(int count)
{
    m_runningCount = count;
    setData(count, LingmoAppletItemModel::RunningRole);
    emitDataChanged();
}

QString LingmoAppletItem::apiVersion() const
{
    return m_info.value(QStringLiteral("X-Lingmo-API-Minimum-Version"));
}

bool LingmoAppletItem::isSupported() const
{
    QVersionNumber version = QVersionNumber::fromString(apiVersion());
    if (version.majorVersion() != 6 /*PROJECT_VERSION_MAJOR*/) {
        return false;
    } else if (version.minorVersion() > 6 /*PROJECT_VERSION_MINOR*/) {
        return false;
    }
    return true;
}

QString LingmoAppletItem::unsupportedMessage() const
{
    const QString versionString = apiVersion();
    QVersionNumber version = QVersionNumber::fromString(versionString);

    if (version.isNull()) {
        // TODO: We have to hardcode 6 for now as PROJECT_VERSION_MAJOR is still 5, change it back to PROJECT_VERSION_MAJOR with 6.0
        return i18n(
            "This Widget was written for an unknown older version of Lingmo and is not compatible with Lingmo %1. Please contact the widget's author for an "
            "updated version.",
            6 /*PROJECT_VERSION_MAJOR*/);
    } else if (version.majorVersion() < 6 /*PROJECT_VERSION_MAJOR*/) {
        return i18n("This Widget was written for Lingmo %1 and is not compatible with Lingmo %2. Please contact the widget's author for an updated version.",
                    version.majorVersion(),
                    6 /*PROJECT_VERSION_MAJOR*/);
    } else if (version.majorVersion() > 6 /*PROJECT_VERSION_MAJOR*/) {
        return i18n("This Widget was written for Lingmo %1 and is not compatible with Lingmo %2. Please update Lingmo in order to use the widget.",
                    version.majorVersion(),
                    6 /*PROJECT_VERSION_MAJOR*/);
    } else if (version.minorVersion() > PROJECT_VERSION_MINOR) {
        return i18n(
            "This Widget was written for Lingmo %1 and is not compatible with the latest version of Lingmo. Please update Lingmo in order to use the widget.",
            versionString);
    }

    return QString();
}

static bool matchesKeywords(QStringView keywords, const QString &pattern)
{
    const auto l = keywords.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const auto &keyword : l) {
        if (keyword.startsWith(pattern, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool LingmoAppletItem::matches(const QString &pattern) const
{
    const QJsonObject rawData = m_info.rawData();
    if (matchesKeywords(KJsonUtils::readTranslatedString(rawData, QStringLiteral("Keywords")), pattern)) {
        return true;
    }

    // Add English name and keywords so users in other languages won't have to switch IME when searching.
    if (!QLocale().name().startsWith(QLatin1String("en_"))) {
        const QString name(rawData[QStringLiteral("KPlugin")][QStringLiteral("Name")].toString());
        const QString keywords(rawData[QStringLiteral("KPlugin")][QStringLiteral("Name")].toString());
        if (name.startsWith(pattern, Qt::CaseInsensitive) || matchesKeywords(keywords, pattern)) {
            return true;
        }
    }

    return AbstractItem::matches(pattern);
}

QStringList LingmoAppletItem::keywords() const
{
    const static QString keywordsJsonKey = QStringLiteral("X-KDE-Keywords");
    constexpr QLatin1Char separator(',');

    const QJsonObject rawData = m_info.rawData();
    if (rawData.contains(keywordsJsonKey)) {
        QStringList keywords = m_info.value(keywordsJsonKey).split(separator);
        keywords << KJsonUtils::readTranslatedString(rawData, keywordsJsonKey).split(separator);
        keywords.removeDuplicates();
        return keywords;
    }
    return {};
}

bool LingmoAppletItem::isLocal() const
{
    return m_local;
}

bool LingmoAppletItem::passesFiltering(const KCategorizedItemsViewModels::Filter &filter) const
{
    if (filter.first == QLatin1String("running")) {
        return running();
    } else if (filter.first == QLatin1String("local")) {
        return isLocal();
    } else if (filter.first == QLatin1String("category")) {
        return m_info.category().compare(filter.second.toString(), Qt::CaseInsensitive) == 0;
    } else {
        return false;
    }
}

QMimeData *LingmoAppletItem::mimeData() const
{
    QMimeData *data = new QMimeData();
    QByteArray appletName;
    appletName += pluginName().toUtf8();
    data->setData(mimeTypes().at(0), appletName);
    return data;
}

QStringList LingmoAppletItem::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("text/x-plasmoidservicename");
    return types;
}

QVariant LingmoAppletItem::data(int role) const
{
    switch (role) {
    case LingmoAppletItemModel::ScreenshotRole:
        // null = not yet done, empty = tried and failed
        if (m_screenshot.isNull()) {
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Applet"));
            pkg.setDefaultPackageRoot(QStringLiteral("lingmo/plasmoids"));
            pkg.setPath(m_info.pluginId());
            if (pkg.isValid()) {
                const_cast<LingmoAppletItem *>(this)->m_screenshot = pkg.filePath("screenshot");
            } else {
                const_cast<LingmoAppletItem *>(this)->m_screenshot = QString();
            }
        } else if (m_screenshot.isEmpty()) {
            return QVariant();
        }
        return m_screenshot;
    default:
        return AbstractItem::data(role);
    }
}

// LingmoAppletItemModel

LingmoAppletItemModel::LingmoAppletItemModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_startupCompleted(false)
{
    setSortRole(Qt::DisplayRole);
}

QHash<int, QByteArray> LingmoAppletItemModel::roleNames() const
{
    QHash<int, QByteArray> newRoleNames = QAbstractItemModel::roleNames();
    newRoleNames[NameRole] = "name";
    newRoleNames[PluginNameRole] = "pluginName";
    newRoleNames[DescriptionRole] = "description";
    newRoleNames[CategoryRole] = "category";
    newRoleNames[LicenseRole] = "license";
    newRoleNames[WebsiteRole] = "website";
    newRoleNames[VersionRole] = "version";
    newRoleNames[AuthorRole] = "author";
    newRoleNames[EmailRole] = "email";
    newRoleNames[RunningRole] = "running";
    newRoleNames[LocalRole] = "local";
    newRoleNames[ScreenshotRole] = "screenshot";
    newRoleNames[ApiVersionRole] = "apiVersion";
    newRoleNames[IsSupportedRole] = "isSupported";
    newRoleNames[UnsupportedMessageRole] = "unsupportedMessage";
    return newRoleNames;
}

void LingmoAppletItemModel::populateModel()
{
    clear();

    auto filter = [this](const KPluginMetaData &plugin) -> bool {
        const QStringList provides = plugin.value(QStringLiteral("X-Lingmo-Provides"), QStringList());

        if (!m_provides.isEmpty()) {
            const bool providesFulfilled = std::any_of(m_provides.cbegin(), m_provides.cend(), [&provides](const QString &p) {
                return provides.contains(p);
            });

            if (!providesFulfilled) {
                return false;
            }
        }

        if (!plugin.isValid() || plugin.rawData().value(QStringLiteral("NoDisplay")).toBool() || plugin.category() == QLatin1String("Containments")) {
            // we don't want to show the hidden category
            return false;
        }

        static const auto formFactors = KRuntimePlatform::runtimePlatform();
        // If runtimePlatformis not defined, accept everything
        bool inFormFactor = formFactors.isEmpty();

        for (const QString &formFactor : formFactors) {
            if (plugin.formFactors().isEmpty() || plugin.formFactors().contains(formFactor)) {
                inFormFactor = true;
                break;
            }
        }

        if (!inFormFactor) {
            return false;
        }

        return true;
    };

    QList<KPluginMetaData> packages =
        KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Applet"), QStringLiteral("lingmo/plasmoids"), filter);

    // NOTE: Those 2 extra searches are for pure retrocompatibility, to list old plasmoids
    // Just to give the user the possibility to remove them.
    // Eventually after a year or two, this code can be removed to drop this transition support

    // Search all of those that have a desktop file metadata, those are lingmo 5 plasmoids
    QList<KPackage::Package> kPackages = KPackage::PackageLoader::self()->listKPackages(QStringLiteral("Lingmo/Applet"), QStringLiteral("lingmo/plasmoids"));
    for (const KPackage::Package &package : kPackages) {
        KPluginMetaData data = package.metadata();
        if (package.filePath("metadata").endsWith(QStringLiteral("metadata.desktop")) && filter(data)) {
            appendRow(new LingmoAppletItem(data));
        }
    }
    // Search all packages that have json metadata but not a correct Lingmo/Applet and put them at the end: assume they are lingmo5 plasmoids
    packages.append(
        KPackage::PackageLoader::self()->findPackages(QString(), QStringLiteral("lingmo/plasmoids"), [&filter](const KPluginMetaData &plugin) -> bool {
            return plugin.value(QStringLiteral("KPackageStructure")) != QStringLiteral("Lingmo/Applet") && filter(plugin);
        }));

    for (const KPluginMetaData &plugin : packages) {
        appendRow(new LingmoAppletItem(plugin));
    }

    Q_EMIT modelPopulated();
}

void LingmoAppletItemModel::setRunningApplets(const QHash<QString, int> &apps)
{
    // for each item, find that string and set the count
    for (int r = 0; r < rowCount(); ++r) {
        QStandardItem *i = item(r);
        LingmoAppletItem *p = dynamic_cast<LingmoAppletItem *>(i);

        if (p) {
            const int running = apps.value(p->pluginName());
            p->setRunning(running);
        }
    }
}

void LingmoAppletItemModel::setRunningApplets(const QString &name, int count)
{
    for (int r = 0; r < rowCount(); ++r) {
        QStandardItem *i = item(r);
        LingmoAppletItem *p = dynamic_cast<LingmoAppletItem *>(i);
        if (p && p->pluginName() == name) {
            p->setRunning(count);
        }
    }
}

QStringList LingmoAppletItemModel::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("text/x-plasmoidservicename");
    return types;
}

QMimeData *LingmoAppletItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0) {
        return nullptr;
    }

    QStringList types = mimeTypes();

    if (types.isEmpty()) {
        return nullptr;
    }

    QMimeData *data = new QMimeData();

    QString format = types.at(0);

    QByteArray appletNames;
    int lastRow = -1;
    for (const QModelIndex &index : indexes) {
        if (index.row() == lastRow) {
            continue;
        }

        lastRow = index.row();
        LingmoAppletItem *selectedItem = (LingmoAppletItem *)itemFromIndex(index);
        appletNames += '\n' + selectedItem->pluginName().toUtf8();
        // qDebug() << selectedItem->pluginName() << index.column() << index.row();
    }

    data->setData(format, appletNames);
    return data;
}

QStringList LingmoAppletItemModel::provides() const
{
    return m_provides;
}

void LingmoAppletItemModel::setProvides(const QStringList &provides)
{
    if (m_provides == provides) {
        return;
    }

    m_provides = provides;
    if (m_startupCompleted) {
        populateModel();
    }
}

void LingmoAppletItemModel::setApplication(const QString &app)
{
    m_application = app;
    if (m_startupCompleted) {
        populateModel();
    }
}

bool LingmoAppletItemModel::startupCompleted() const
{
    return m_startupCompleted;
}

void LingmoAppletItemModel::setStartupCompleted(bool complete)
{
    m_startupCompleted = complete;
}

QString &LingmoAppletItemModel::Application()
{
    return m_application;
}

// #include <lingmoappletitemmodel_p.moc>
