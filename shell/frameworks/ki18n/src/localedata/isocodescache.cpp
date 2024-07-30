/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-localedata.h"

#include "isocodes_p.h"
#include "isocodescache_p.h"
#include "logging.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

// increment those when changing the format
enum : uint32_t {
    Iso3166_1CacheHeader = 0x4B493101,
    Iso3166_2CacheHeader = 0x4B493201,
};

static QString isoCodesPath(QStringView file)
{
#ifndef Q_OS_ANDROID
    auto path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("iso-codes/json/") + file, QStandardPaths::LocateFile);
    if (!path.isEmpty()) {
        return path;
    }

    // search manually in the compile-time determined prefix
    // needed for example for non-installed Windows binaries to work, such as unit tests
    for (const char *installLocation : {"/share", "/bin/data"}) {
        path = QLatin1String(ISO_CODES_PREFIX) + QLatin1String(installLocation) + QLatin1String("/iso-codes/json/") + file;
        if (QFileInfo::exists(path)) {
            return path;
        }
    }

    return {};
#else
    return QLatin1String("assets:/share/iso-codes/json/") + file;
#endif
}

static QString cachePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/org.kde.ki18n/iso-codes/");
}

static QString cacheFilePath(QStringView file)
{
    return cachePath() + file;
}

IsoCodesCache::~IsoCodesCache() = default;

IsoCodesCache *IsoCodesCache::instance()
{
    static IsoCodesCache s_cache;
    return &s_cache;
}

void IsoCodesCache::loadIso3166_1()
{
    if (!m_iso3166_1CacheData && !loadIso3166_1Cache()) {
        QDir().mkpath(cachePath());
        createIso3166_1Cache(isoCodesPath(u"iso_3166-1.json"), cacheFilePath(u"iso_3166-1"));
        loadIso3166_1Cache();
    }
}

static std::unique_ptr<QFile> openCacheFile(QStringView cacheFileName, QStringView isoCodesFileName)
{
    QFileInfo jsonFi(isoCodesPath(isoCodesFileName));
    if (!jsonFi.exists()) { // no source file means we can only use an embedded cache
        auto f = std::make_unique<QFile>(QLatin1String(":/org.kde.ki18n/iso-codes/cache/") + cacheFileName);
        if (!f->open(QFile::ReadOnly) || f->size() < 8) {
            return {};
        }
        return f;
    }
    auto f = std::make_unique<QFile>(cacheFilePath(cacheFileName));
    if (!f->open(QFile::ReadOnly) || f->fileTime(QFile::FileModificationTime) < jsonFi.lastModified() || f->size() < 8) {
        return {};
    }
    return f;
}

bool IsoCodesCache::loadIso3166_1Cache()
{
    auto f = openCacheFile(u"iso_3166-1", u"iso_3166-1.json");
    if (!f) {
        return false;
    }
    m_iso3166_1CacheSize = f->size();

    // validate cache file is usable
    // header matches
    const auto data = f->map(0, m_iso3166_1CacheSize);
    if (*reinterpret_cast<const uint32_t *>(data) != Iso3166_1CacheHeader) {
        return false;
    }
    // lookup tables fit into the available size
    const auto size = *(reinterpret_cast<const uint32_t *>(data) + 1);
    if (sizeof(Iso3166_1CacheHeader) + sizeof(size) + size * sizeof(MapEntry<uint16_t>) * 2 >= m_iso3166_1CacheSize) {
        return false;
    }
    // string table is 0 terminated
    if (data[m_iso3166_1CacheSize - 1] != '\0') {
        return false;
    }

    m_iso3166_1CacheFile = std::move(f);
    m_iso3166_1CacheData = data;
    return true;
}

uint32_t IsoCodesCache::countryCount() const
{
    return m_iso3166_1CacheData ? *(reinterpret_cast<const uint32_t *>(m_iso3166_1CacheData) + 1) : 0;
}

const MapEntry<uint16_t> *IsoCodesCache::countryNameMapBegin() const
{
    return m_iso3166_1CacheData ? reinterpret_cast<const MapEntry<uint16_t> *>(m_iso3166_1CacheData + sizeof(uint32_t) * 2) : nullptr;
}

const MapEntry<uint16_t> *IsoCodesCache::countryAlpha3MapBegin() const
{
    return m_iso3166_1CacheData ? countryNameMapBegin() + countryCount() : nullptr;
}

const char *IsoCodesCache::countryStringTableLookup(uint16_t offset) const
{
    if (m_iso3166_1CacheData) {
        const auto pos = offset + 2 * sizeof(uint32_t) + 2 * countryCount() * sizeof(MapEntry<uint16_t>);
        return m_iso3166_1CacheSize > pos ? reinterpret_cast<const char *>(m_iso3166_1CacheData + pos) : nullptr;
    }
    return nullptr;
}

void IsoCodesCache::createIso3166_1Cache(const QString &isoCodesPath, const QString &cacheFilePath)
{
    qCDebug(KI18NLD) << "Rebuilding ISO 3166-1 cache";

    QFile file(isoCodesPath);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(KI18NLD) << "Unable to open iso_3166-1.json" << isoCodesPath << file.errorString();
        return;
    }

    std::vector<MapEntry<uint16_t>> alpha2NameMap;
    std::vector<MapEntry<uint16_t>> alpha3alpha2Map;
    QByteArray iso3166_1stringTable;

    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto array = doc.object().value(QLatin1String("3166-1")).toArray();
    for (const auto &entryVal : array) {
        const auto entry = entryVal.toObject();
        const auto alpha2 = entry.value(QLatin1String("alpha_2")).toString();
        if (alpha2.size() != 2) {
            continue;
        }
        const auto alpha2Key = IsoCodes::alpha2CodeToKey(alpha2);

        assert(std::numeric_limits<uint16_t>::max() > iso3166_1stringTable.size());
        alpha2NameMap.push_back({alpha2Key, (uint16_t)iso3166_1stringTable.size()});
        iso3166_1stringTable.append(entry.value(QLatin1String("name")).toString().toUtf8());
        iso3166_1stringTable.append('\0');

        const auto alpha3Key = IsoCodes::alpha3CodeToKey(entry.value(QLatin1String("alpha_3")).toString());
        alpha3alpha2Map.push_back({alpha3Key, alpha2Key});
    }

    std::sort(alpha2NameMap.begin(), alpha2NameMap.end());
    std::sort(alpha3alpha2Map.begin(), alpha3alpha2Map.end());

    // write out binary cache file
    QFile cache(cacheFilePath);
    if (!cache.open(QFile::WriteOnly)) {
        qCWarning(KI18NLD) << "Failed to write ISO 3166-1 cache:" << cache.errorString() << cache.fileName();
        return;
    }

    uint32_t n = Iso3166_1CacheHeader;
    cache.write(reinterpret_cast<const char *>(&n), 4); // header
    n = alpha2NameMap.size();
    cache.write(reinterpret_cast<const char *>(&n), 4); // size
    for (auto entry : alpha2NameMap) {
        cache.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
    }
    for (auto entry : alpha3alpha2Map) {
        cache.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
    }
    cache.write(iso3166_1stringTable);
}

void IsoCodesCache::loadIso3166_2()
{
    if (!m_iso3166_2CacheData && !loadIso3166_2Cache()) {
        QDir().mkpath(cachePath());
        createIso3166_2Cache(isoCodesPath(u"iso_3166-2.json"), cacheFilePath(u"iso_3166-2"));
        loadIso3166_2Cache();
    }
}

bool IsoCodesCache::loadIso3166_2Cache()
{
    auto f = openCacheFile(u"iso_3166-2", u"iso_3166-2.json");
    if (!f) {
        return false;
    }
    m_iso3166_2CacheSize = f->size();

    // validate cache file is usable
    // header matches
    const auto data = f->map(0, m_iso3166_2CacheSize);
    if (*reinterpret_cast<const uint32_t *>(data) != Iso3166_2CacheHeader) {
        return false;
    }
    // name lookup table fits into the available size
    auto size = *(reinterpret_cast<const uint32_t *>(data) + 1);
    auto offset = 3 * sizeof(uint32_t) + size * sizeof(MapEntry<uint32_t>);
    if (offset >= m_iso3166_2CacheSize) {
        return false;
    }
    // hierarchy map boundary check
    size = *(reinterpret_cast<const uint32_t *>(data + offset) - 1);
    offset += size * sizeof(MapEntry<uint32_t>);
    if (offset >= m_iso3166_2CacheSize) {
        return false;
    }
    // string table is 0 terminated
    if (data[m_iso3166_2CacheSize - 1] != '\0') {
        return false;
    }

    m_iso3166_2CacheFile = std::move(f);
    m_iso3166_2CacheData = data;
    return true;
}

uint32_t IsoCodesCache::subdivisionCount() const
{
    return m_iso3166_2CacheData ? *(reinterpret_cast<const uint32_t *>(m_iso3166_2CacheData) + 1) : 0;
}

const MapEntry<uint32_t> *IsoCodesCache::subdivisionNameMapBegin() const
{
    return m_iso3166_2CacheData ? reinterpret_cast<const MapEntry<uint32_t> *>(m_iso3166_2CacheData + 2 * sizeof(uint32_t)) : nullptr;
}

uint32_t IsoCodesCache::subdivisionHierachyMapSize() const
{
    return m_iso3166_2CacheData
        ? *(reinterpret_cast<const uint32_t *>(m_iso3166_2CacheData + 2 * sizeof(uint32_t) + subdivisionCount() * sizeof(MapEntry<uint32_t>)))
        : 0;
}

const MapEntry<uint32_t> *IsoCodesCache::subdivisionParentMapBegin() const
{
    return m_iso3166_2CacheData
        ? reinterpret_cast<const MapEntry<uint32_t> *>(m_iso3166_2CacheData + 3 * sizeof(uint32_t) + subdivisionCount() * sizeof(MapEntry<uint32_t>))
        : nullptr;
}

const char *IsoCodesCache::subdivisionStringTableLookup(uint16_t offset) const
{
    if (m_iso3166_2CacheData) {
        const auto pos = offset + 3 * sizeof(uint32_t) + (subdivisionCount() + subdivisionHierachyMapSize()) * sizeof(MapEntry<uint32_t>);
        return m_iso3166_2CacheSize > pos ? reinterpret_cast<const char *>(m_iso3166_2CacheData + pos) : nullptr;
    }
    return nullptr;
}

void IsoCodesCache::createIso3166_2Cache(const QString &isoCodesPath, const QString &cacheFilePath)
{
    qCDebug(KI18NLD) << "Rebuilding ISO 3166-2 cache";
    QFile file(isoCodesPath);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(KI18NLD) << "Unable to open iso_3166-2.json" << isoCodesPath << file.errorString();
        return;
    }

    std::vector<MapEntry<uint32_t>> subdivNameMap;
    std::vector<MapEntry<uint32_t>> subdivParentMap;
    QByteArray iso3166_2stringTable;

    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto array = doc.object().value(QLatin1String("3166-2")).toArray();
    for (const auto &entryVal : array) {
        const auto entry = entryVal.toObject();
        const auto key = IsoCodes::subdivisionCodeToKey(entry.value(QLatin1String("code")).toString());

        assert(std::numeric_limits<uint16_t>::max() > iso3166_2stringTable.size());
        subdivNameMap.push_back({key, (uint16_t)iso3166_2stringTable.size()});
        iso3166_2stringTable.append(entry.value(QLatin1String("name")).toString().toUtf8());
        iso3166_2stringTable.append('\0');

        const auto parentKey = IsoCodes::parentCodeToKey(entry.value(QLatin1String("parent")).toString());
        if (parentKey) {
            subdivParentMap.push_back({key, parentKey});
        }
    }

    std::sort(subdivNameMap.begin(), subdivNameMap.end());
    std::sort(subdivParentMap.begin(), subdivParentMap.end());

    // write out binary cache file
    QFile cache(cacheFilePath);
    if (!cache.open(QFile::WriteOnly)) {
        qCWarning(KI18NLD) << "Failed to write ISO 3166-2 cache:" << cache.errorString() << cache.fileName();
        return;
    }

    uint32_t n = Iso3166_2CacheHeader;
    cache.write(reinterpret_cast<const char *>(&n), 4); // header
    n = subdivNameMap.size();
    cache.write(reinterpret_cast<const char *>(&n), 4); // size of the name map
    for (auto entry : subdivNameMap) {
        cache.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
    }
    n = subdivParentMap.size();
    cache.write(reinterpret_cast<const char *>(&n), 4); // size of the hierarchy map
    for (auto entry : subdivParentMap) {
        cache.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
    }
    cache.write(iso3166_2stringTable);
}
