/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999-2000 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1996-2000 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGDATA_P_H
#define KCONFIGDATA_P_H

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <map>

/**
 * map/dict/list config node entry.
 * @internal
 */
struct KEntry {
    /** Constructor. @internal */
    KEntry()
        : mValue()
        , bDirty(false)
        , bGlobal(false)
        , bImmutable(false)
        , bDeleted(false)
        , bExpand(false)
        , bReverted(false)
        , bLocalizedCountry(false)
        , bNotify(false)
        , bOverridesGlobal(false)
    {
    }
    /** @internal */
    QByteArray mValue;
    /**
     * Must the entry be written back to disk?
     */
    bool bDirty : 1;
    /**
     * Entry should be written to the global config file
     */
    bool bGlobal : 1;
    /**
     * Entry can not be modified.
     */
    bool bImmutable : 1;
    /**
     * Entry has been deleted.
     */
    bool bDeleted : 1;
    /**
     * Whether to apply dollar expansion or not.
     */
    bool bExpand : 1;
    /**
     * Entry has been reverted to its default value (from a more global file).
     */
    bool bReverted : 1;
    /**
     * Entry is for a localized key. If @c false the value references just language e.g. "de",
     * if @c true the value references language and country, e.g. "de_DE".
     **/
    bool bLocalizedCountry : 1;

    bool bNotify : 1;

    /**
     * Entry will need to be written on a non global file even if it matches default value
     */
    bool bOverridesGlobal : 1;
};

Q_DECLARE_TYPEINFO(KEntry, Q_RELOCATABLE_TYPE);

// These operators are used to check whether an entry which is about
// to be written equals the previous value. As such, this intentionally
// omits the dirty/notify flag from the comparison.
inline bool operator==(const KEntry &k1, const KEntry &k2)
{
    /* clang-format off */
    return k1.bGlobal == k2.bGlobal
        && k1.bImmutable == k2.bImmutable
        && k1.bDeleted == k2.bDeleted
        && k1.bExpand == k2.bExpand
        && k1.mValue == k2.mValue;
    /* clang-format on */
}

inline bool operator!=(const KEntry &k1, const KEntry &k2)
{
    return !(k1 == k2);
}

/**
 * key structure holding both the actual key and the group
 * to which it belongs.
 * @internal
 */
struct KEntryKey {
    /** Constructor. @internal */
    KEntryKey(const QString &_group = QString(), const QByteArray &_key = QByteArray(), bool isLocalized = false, bool isDefault = false)
        : mGroup(_group)
        , mKey(_key)
        , bLocal(isLocalized)
        , bDefault(isDefault)
        , bRaw(false)
    {
    }
    /**
     * The "group" to which this EntryKey belongs
     */
    QString mGroup;
    /**
     * The _actual_ key of the entry in question
     */
    QByteArray mKey;
    /**
     * Entry is localised or not
     */
    bool bLocal : 1;
    /**
     * Entry indicates if this is a default value.
     */
    bool bDefault : 1;
    /** @internal
     * Key is a raw unprocessed key.
     * @warning this should only be set during merging, never for normal use.
     */
    bool bRaw : 1;
};

Q_DECLARE_TYPEINFO(KEntryKey, Q_RELOCATABLE_TYPE);

/**
 * Compares two KEntryKeys (needed for std::map). The order is localized, localized-default,
 * non-localized, non-localized-default
 * @internal
 */
inline bool operator<(const KEntryKey &k1, const KEntryKey &k2)
{
    int result = k1.mGroup.compare(k2.mGroup);
    if (result != 0) {
        return result < 0;
    }

    result = k1.mKey.compare(k2.mKey);
    if (result != 0) {
        return result < 0;
    }

    if (k1.bLocal != k2.bLocal) {
        return k1.bLocal;
    }
    return (!k1.bDefault && k2.bDefault);
}

/**
 * Light-weight view variant of KEntryKey.
 * Used for look-up in the map.
 * @internal
 */
struct KEntryKeyView {
    /** Constructor. @internal */
    KEntryKeyView(QStringView _group, QAnyStringView _key, bool isLocalized = false, bool isDefault = false)
        : mGroup(_group)
        , mKey(_key)
        , bLocal(isLocalized)
        , bDefault(isDefault)
    {
    }
    /**
     * The "group" to which this EntryKey belongs
     */
    const QStringView mGroup;
    /**
     * The _actual_ key of the entry in question
     */
    const QAnyStringView mKey;
    /**
     * Entry is localised or not
     */
    bool bLocal : 1;
    /**
     * Entry indicates if this is a default value.
     */
    bool bDefault : 1;
};

template<typename TEntryKey1, typename TEntryKey2>
bool compareEntryKeyViews(const TEntryKey1 &k1, const TEntryKey2 &k2)
{
    int result = k1.mGroup.compare(k2.mGroup);
    if (result != 0) {
        return result < 0;
    }

    result = QAnyStringView::compare(k1.mKey, k2.mKey);
    if (result != 0) {
        return result < 0;
    }

    if (k1.bLocal != k2.bLocal) {
        return k1.bLocal;
    }
    return (!k1.bDefault && k2.bDefault);
}

inline bool operator<(const KEntryKeyView &k1, const KEntryKey &k2)
{
    return compareEntryKeyViews(k1, k2);
}

inline bool operator<(const KEntryKey &k1, const KEntryKeyView &k2)
{
    return compareEntryKeyViews(k1, k2);
}

/**
 * Struct to use as Compare type with std::map.
 * To enable usage of KEntryKeyView for look-up in the map
 * via the template find() overloads.
 * @internal
 */
struct KEntryKeyCompare {
    using is_transparent = void;

    bool operator()(const KEntryKey &k1, const KEntryKey &k2) const
    {
        return (k1 < k2);
    }

    bool operator()(const KEntryKeyView &k1, const KEntryKey &k2) const
    {
        return (k1 < k2);
    }

    bool operator()(const KEntryKey &k1, const KEntryKeyView &k2) const
    {
        return (k1 < k2);
    }
};

/**
 * Returns the minimum key that has @a mGroup == @p group.
 *
 * @note The returned "minimum key" is consistent with KEntryKey's operator<().
 *       The return value of this function can be passed to KEntryMap::lowerBound().
 */
inline KEntryKeyView minimumGroupKeyView(const QString &group)
{
    return KEntryKeyView(group, QAnyStringView{}, true, false);
}

QDebug operator<<(QDebug dbg, const KEntryKey &key);
QDebug operator<<(QDebug dbg, const KEntry &entry);

/**
 * \relates KEntry
 * type specifying a map of entries (key,value pairs).
 * The keys are actually a key in a particular config file group together
 * with the group name.
 * @internal
 */
class KEntryMap : public std::map<KEntryKey, KEntry, KEntryKeyCompare>
{
public:
    enum SearchFlag {
        SearchDefaults = 1,
        SearchLocalized = 2,
    };
    Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

    enum EntryOption {
        EntryDirty = 1,
        EntryGlobal = 2,
        EntryImmutable = 4,
        EntryDeleted = 8,
        EntryExpansion = 16,
        EntryRawKey = 32,
        EntryLocalizedCountry = 64,
        EntryNotify = 128,
        EntryDefault = (SearchDefaults << 16),
        EntryLocalized = (SearchLocalized << 16),
    };
    Q_DECLARE_FLAGS(EntryOptions, EntryOption)

    iterator findExactEntry(const QString &group, QAnyStringView key = QAnyStringView(), SearchFlags flags = SearchFlags());

    iterator findEntry(const QString &group, QAnyStringView key = QAnyStringView(), SearchFlags flags = SearchFlags());

    const_iterator findEntry(const QString &group, QAnyStringView key = QAnyStringView(), SearchFlags flags = SearchFlags()) const
    {
        return constFindEntry(group, key, flags);
    }

    const_iterator constFindEntry(const QString &group, QAnyStringView key = QAnyStringView(), SearchFlags flags = SearchFlags()) const;

    /**
     * Returns true if the entry gets dirtied or false in other case
     */
    bool setEntry(const QString &group, const QByteArray &key, const QByteArray &value, EntryOptions options);

    void setEntry(const QString &group, const QByteArray &key, const QString &value, EntryOptions options)
    {
        setEntry(group, key, value.toUtf8(), options);
    }

    QString getEntry(const QString &group,
                     QAnyStringView key,
                     const QString &defaultValue = QString(),
                     SearchFlags flags = SearchFlags(),
                     bool *expand = nullptr) const;

    bool hasEntry(const QString &group, QAnyStringView key = QAnyStringView(), SearchFlags flags = SearchFlags()) const;

    bool getEntryOption(const const_iterator &it, EntryOption option) const;
    bool getEntryOption(const QString &group, QAnyStringView key, SearchFlags flags, EntryOption option) const
    {
        return getEntryOption(findEntry(group, key, flags), option);
    }

    void setEntryOption(iterator it, EntryOption option, bool bf);
    void setEntryOption(const QString &group, QAnyStringView key, SearchFlags flags, EntryOption option, bool bf)
    {
        setEntryOption(findEntry(group, key, flags), option, bf);
    }

    bool revertEntry(const QString &group, QAnyStringView key, EntryOptions options, SearchFlags flags = SearchFlags());

    template<typename ConstIteratorUser>
    void forEachEntryWhoseGroupStartsWith(const QString &groupPrefix, ConstIteratorUser callback) const
    {
        for (auto it = lower_bound(minimumGroupKeyView(groupPrefix)), end = cend(); it != end && it->first.mGroup.startsWith(groupPrefix); ++it) {
            callback(it);
        }
    }

    template<typename ConstIteratorPredicate>
    bool anyEntryWhoseGroupStartsWith(const QString &groupPrefix, ConstIteratorPredicate predicate) const
    {
        for (auto it = lower_bound(minimumGroupKeyView(groupPrefix)), end = cend(); it != end && it->first.mGroup.startsWith(groupPrefix); ++it) {
            if (predicate(it)) {
                return true;
            }
        }
        return false;
    }

    template<typename ConstIteratorUser>
    void forEachEntryOfGroup(const QString &theGroup, ConstIteratorUser callback) const
    {
        const auto theEnd = cend();
        auto it = constFindEntry(theGroup);
        if (it != theEnd) {
            ++it; // advance past the special group entry marker

            for (; (it != theEnd) && (it->first.mGroup == theGroup); ++it) {
                callback(it);
            }
        }
    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KEntryMap::SearchFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KEntryMap::EntryOptions)

/**
 * \relates KEntry
 * type for iterating over keys in a KEntryMap in sorted order.
 * @internal
 */
typedef KEntryMap::iterator KEntryMapIterator;

/**
 * \relates KEntry
 * type for iterating over keys in a KEntryMap in sorted order.
 * It is const, thus you cannot change the entries in the iterator,
 * only examine them.
 * @internal
 */
typedef KEntryMap::const_iterator KEntryMapConstIterator;

#endif
