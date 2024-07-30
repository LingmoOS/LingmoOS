/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ISOCODESCACHE_P_H
#define ISOCODESCACHE_P_H

#include "mapentry_p.h"

#include <QByteArray>
#include <QStringView>

#include <cstdint>
#include <memory>
#include <vector>

class QFile;

/** Cache for iso-codes JSON data. */
class IsoCodesCache
{
public:
    ~IsoCodesCache();

    static IsoCodesCache *instance();

    void loadIso3166_1();
    void loadIso3166_2();

    uint32_t countryCount() const;
    const MapEntry<uint16_t> *countryNameMapBegin() const;
    inline const MapEntry<uint16_t> *countryNameMapEnd() const
    {
        return countryNameMapBegin() + countryCount();
    }
    const MapEntry<uint16_t> *countryAlpha3MapBegin() const;
    inline const MapEntry<uint16_t> *countryAlpha3MapEnd() const
    {
        return countryAlpha3MapBegin() + countryCount();
    }
    const char *countryStringTableLookup(uint16_t offset) const;

    uint32_t subdivisionCount() const;
    const MapEntry<uint32_t> *subdivisionNameMapBegin() const;
    inline const MapEntry<uint32_t> *subdivisionNameMapEnd() const
    {
        return subdivisionNameMapBegin() + subdivisionCount();
    }
    uint32_t subdivisionHierachyMapSize() const;
    const MapEntry<uint32_t> *subdivisionParentMapBegin() const;
    inline const MapEntry<uint32_t> *subdivisionParentMapEnd() const
    {
        return subdivisionParentMapBegin() + subdivisionHierachyMapSize();
    }
    const char *subdivisionStringTableLookup(uint16_t offset) const;

    static void createIso3166_1Cache(const QString &isoCodesPath, const QString &cacheFilePath);
    static void createIso3166_2Cache(const QString &isoCodesPath, const QString &cacheFilePath);

private:
    bool loadIso3166_1Cache();
    bool loadIso3166_2Cache();

    std::unique_ptr<QFile> m_iso3166_1CacheFile;
    const uint8_t *m_iso3166_1CacheData = nullptr;
    std::size_t m_iso3166_1CacheSize = 0;
    std::unique_ptr<QFile> m_iso3166_2CacheFile;
    const uint8_t *m_iso3166_2CacheData = nullptr;
    std::size_t m_iso3166_2CacheSize = 0;
};

#endif // ISOCODESCACHE_H
