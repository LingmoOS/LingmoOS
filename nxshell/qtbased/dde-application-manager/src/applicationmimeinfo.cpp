// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "applicationmimeinfo.h"
#include "global.h"

constexpr std::array<char, 9> desktopSuffix{u8".desktop"};

QStringList getListFiles() noexcept
{
    QStringList files;
    auto configDirs = getXDGConfigDirs();
    auto dataDirs = getXDGDataDirs();
    auto desktop = getCurrentDesktop().toLower();

    auto appendListFile = [&files, &desktop](const QString &dir) {
        QFileInfo cur{dir};
        if (!cur.exists() or !cur.isDir()) {
            return;
        }

        QDir tmp{cur.absoluteFilePath()};
        auto desktopList = tmp.filePath(QString{"%1-mimeapps.list"}.arg(desktop));
        if (QFileInfo::exists(desktopList)) {
            files.append(desktopList);
        }

        desktopList = tmp.filePath("mimeapps.list");
        if (QFileInfo::exists(desktopList)) {
            files.append(desktopList);
        }
    };

    std::for_each(configDirs.cbegin(), configDirs.cend(), appendListFile);
    std::for_each(dataDirs.cbegin(), dataDirs.cend(), appendListFile);
    std::reverse(files.begin(), files.end());
    return files;
}
namespace {
QString toString(const MimeContent &content) noexcept
{
    QString ret;

    for (auto it = content.constKeyValueBegin(); it != content.constKeyValueEnd(); ++it) {
        ret.append(QString{"[%1]\n"}.arg(it->first));
        const auto &QStringMap = it->second;
        for (auto inner = QStringMap.constKeyValueBegin(); inner != QStringMap.constKeyValueEnd(); ++inner) {
            ret.append(QString{"%1="}.arg(inner->first));
            for (const auto &app : inner->second) {
                ret.append(QString{"%2;"}.arg(app));
            }
            ret.append('\n');
        }
        ret.append('\n');
    }

    return ret;
}

QString removeDesktopSuffix(const QString &str) noexcept
{
    return str.chopped(desktopSuffix.size() - 1);
}
}  // namespace

std::optional<MimeFileBase> MimeFileBase::loadFromFile(const QFileInfo &fileInfo, bool desktopSpec)
{
    bool isWritable{false};
    auto filePath = fileInfo.absoluteFilePath();
    if (filePath.startsWith(getXDGConfigHome()) or filePath.startsWith(getXDGDataHome())) {
        isWritable = true;
    }

    QFile file{filePath};
    if (!file.open(QFile::ReadOnly | QFile::Text | QFile::ExistingOnly)) {
        qWarning() << "open" << filePath << "failed:" << file.errorString();
        return std::nullopt;
    }

    QTextStream stream{&file};
    stream.setEncoding(QStringConverter::Utf8);

    MimeContent content;
    MimeFileParser parser{stream, desktopSpec};
    if (auto err = parser.parse(content); err != ParserError::NoError) {
        qWarning() << "file:" << filePath << "parse failed:" << err;
        return std::nullopt;
    }

    return MimeFileBase{fileInfo, std::move(content), desktopSpec, isWritable};
}

MimeFileBase::MimeFileBase(const QFileInfo &info, MimeContent &&content, bool desktopSpec, bool writable)
    : m_desktopSpec(desktopSpec)
    , m_writable(writable)
    , m_info(info)
    , m_content(std::move(content))
{
}

void MimeFileBase::reload() noexcept
{
    auto newBase = MimeFileBase::loadFromFile(fileInfo(), isDesktopSpecific());
    if (!newBase) {
        qWarning() << "reload" << fileInfo().absoluteFilePath() << "failed, content wouldn't be changed.";
        return;
    }

    m_content = std::move(newBase->m_content);
}

MimeApps::MimeApps(MimeFileBase &&base)
    : MimeFileBase(std::move(base))
{
}

std::optional<MimeApps> MimeApps::createMimeApps(const QString &filePath, bool desktopSpec) noexcept
{
    auto baseOpt = MimeFileBase::loadFromFile(QFileInfo{filePath}, desktopSpec);

    if (!baseOpt) {
        return std::nullopt;
    }

    return MimeApps{std::move(baseOpt).value()};
}

void MimeApps::insertToSection(const QString &section, const QString &mimeType, const QString &appId) noexcept
{
    auto &map = content();

    auto targetSection = map.find(section);
    if (targetSection == map.end()) {
        targetSection = map.insert(section, {});
    }

    QStringList newApps{QString{appId}.append(desktopSuffix.data())};
    auto oldApps = targetSection->find(mimeType);
    if (oldApps != targetSection->end()) {
        newApps.append(*oldApps);
    }

    targetSection->insert(mimeType, newApps);
}

void MimeApps::addAssociation(const QString &mimeType, const QString &appId) noexcept
{
    insertToSection(addedAssociations, mimeType, appId);
}

void MimeApps::removeAssociation(const QString &mimeType, const QString &appId) noexcept
{
    insertToSection(removedAssociations, mimeType, appId);
}

void MimeApps::setDefaultApplication(const QString &mimeType, const QString &appId) noexcept
{
    auto &map = content();

    auto defaultSection = map.find(defaultApplications);
    if (defaultSection == map.end()) {
        defaultSection = map.insert(defaultApplications, {});
    }

    defaultSection->insert(mimeType, {QString{appId}.append(desktopSuffix.data())});
}

void MimeApps::unsetDefaultApplication(const QString &mimeType) noexcept
{
    auto &map = content();

    auto defaultSection = map.find(defaultApplications);
    if (defaultSection == map.end()) {
        return;
    }

    defaultSection->remove(mimeType);
}

AppList MimeApps::queryTypes(QString appId) const noexcept
{
    AppList ret;
    appId.append(desktopSuffix.data());
    const auto &lists = content();

    if (const auto &adds = lists.constFind(addedAssociations); adds != lists.cend()) {
        std::for_each(adds->constKeyValueBegin(), adds->constKeyValueEnd(), [&ret, &appId](const auto &it) {
            if (it.second.contains(appId)) {
                ret.added.append(it.first);
            }
        });
    }

    if (const auto &removes = lists.constFind(removedAssociations); removes != lists.cend()) {
        std::for_each(removes->constKeyValueBegin(), removes->constKeyValueEnd(), [&ret, &appId](const auto &it) {
            if (it.second.contains(appId)) {
                ret.removed.removeOne(it.first);
            }
        });
    }

    return ret;
}

bool MimeApps::writeToFile() const noexcept
{
    auto filePath = fileInfo().absoluteFilePath();
    if (!isWritable()) {
        qDebug() << "shouldn't write file:" << filePath;
        return false;
    }

    QFile file{filePath};

    if (!file.open(QFile::ExistingOnly | QFile::Text | QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "open" << filePath << "failed:" << file.errorString();
        return false;
    }

    auto newContent = toString(content()).toLocal8Bit();
    auto bytes = file.write(newContent);

    if (bytes != newContent.size()) {
        qWarning() << "incomplete content, write:" << bytes << "total:" << newContent.size();
        return false;
    }

    return true;
}

QString MimeApps::queryDefaultApp(const QString &type) const noexcept
{
    const auto &map = content();
    auto defaultSection = map.find(defaultApplications);
    if (defaultSection == map.end()) {
        return {};
    }

    auto defaultApp = defaultSection->find(type);
    if (defaultApp == defaultSection->end()) {
        return {};
    }

    const auto &app = defaultApp.value();
    if (app.isEmpty()) {
        return {};
    }

    return removeDesktopSuffix(app.first());
}

QStringList MimeCache::queryTypes(QString appId) const noexcept
{
    QStringList ret;
    appId.append(desktopSuffix.data());
    const auto &cache = content()[mimeCache];
    for (auto it = cache.constKeyValueBegin(); it != cache.constKeyValueEnd(); ++it) {
        if (it->second.contains(appId)) {
            ret.append(it->first);
        }
    }
    return ret;
}

std::optional<MimeCache> MimeCache::createMimeCache(const QString &filePath) noexcept
{
    auto baseOpt = MimeFileBase::loadFromFile(QFileInfo{filePath}, false);

    if (!baseOpt) {
        return std::nullopt;
    }

    return MimeCache{std::move(baseOpt).value()};
}

MimeCache::MimeCache(MimeFileBase &&base)
    : MimeFileBase(std::move(base))
{
}

QStringList MimeCache::queryApps(const QString &type) const noexcept
{
    const auto &content = this->content();
    auto it = content.constFind(mimeCache);
    if (it == content.constEnd()) {
        qDebug() << "this cache is broken, please reload.";
        return {};
    }

    QStringList ret;
    if (auto kv = it->constFind(type); kv != it->constEnd()) {
        const auto &apps = kv.value();
        for (const auto &e : apps) {
            if (!e.endsWith(desktopSuffix.data())) {
                continue;
            }
            ret.append(removeDesktopSuffix(e));
        }
    }
    return ret;
}

void createUserConfig(const QString &filename) noexcept
{
    QFile userFile{filename};
    if (!userFile.open(QFile::WriteOnly | QFile::Text)) {
        qCritical() << "failed to create user file:" << filename << userFile.errorString();
    }

    decltype(auto) initContent = u8"[Default Applications]";
    if (userFile.write(initContent) == sizeof(initContent) - 1 and userFile.flush()) {
        qInfo() << "create user mimeapps:" << filename;
        return;
    }

    qWarning() << "failed to write content into" << filename << userFile.errorString();
    userFile.remove();
}

std::optional<MimeInfo> MimeInfo::createMimeInfo(const QString &directory) noexcept
{
    MimeInfo ret;
    auto dirPath = QDir::cleanPath(directory);
    QDir dir;

    if (!QFileInfo::exists(dirPath)) {
        qCritical() << "directory " << dirPath << "doesn't exists.";
        return std::nullopt;
    }
    dir.setPath(dirPath);
    ret.m_directory = dirPath;

    QFileInfo cacheFile{dir.filePath("mimeinfo.cache")};
    if (cacheFile.exists() and cacheFile.isFile()) {
        ret.m_cache = MimeCache::createMimeCache(cacheFile.absoluteFilePath());
    }

    QFileInfo desktopAppList{dir.filePath(QString{"%1-mimeapps.list"}.arg(getCurrentDesktop().toLower()))};
    if (desktopAppList.exists() and desktopAppList.isFile()) {
        auto desktopApps = MimeApps::createMimeApps(desktopAppList.absoluteFilePath(), true);
        if (desktopApps) {
            ret.m_appsList.emplace_back(std::move(desktopApps).value());
        }
    }

    QFileInfo appList{dir.filePath("mimeapps.list")};
    appList.setCaching(false);

    auto userMimeApps = appList.absoluteFilePath();
    if (userMimeApps.startsWith(getXDGConfigHome()) and (!appList.exists() or !appList.isFile())) {
        createUserConfig(userMimeApps);
    }

    if (appList.exists() and appList.isFile()) {
        auto apps = MimeApps::createMimeApps(appList.absoluteFilePath(), false);
        if (apps) {
            ret.m_appsList.emplace_back(std::move(apps).value());
        }
    }

    return ret;
}

void MimeInfo::reload() noexcept
{
    for (auto &app : m_appsList) {
        app.reload();
    }

    if (m_cache) {
        m_cache->reload();
    }
}
