/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocaentry.h"
#include "sycocadebug.h"
#include <kservice.h>

#include <QBitArray>
#include <QIODevice>
#include <QList>

namespace
{
struct string_entry {
    string_entry(const QString &_key, const KSycocaEntry::Ptr &_payload)
        : hash(0)
        , length(_key.length())
        , keyStr(_key)
        , key(keyStr.unicode())
        , payload(_payload)
    {
    }
    uint hash;
    const int length;
    const QString keyStr;
    const QChar *const key; // always points to keyStr.unicode(); just an optimization
    const KSycocaEntry::Ptr payload;
};
}

class KSycocaDictPrivate
{
public:
    KSycocaDictPrivate()
        : stream(nullptr)
        , offset(0)
        , hashTableSize(0)
    {
    }

    ~KSycocaDictPrivate()
    {
    }

    // Helper for find_string and findMultiString
    qint32 offsetForKey(const QString &key) const;

    // Calculate hash - can be used during loading and during saving.
    quint32 hashKey(const QString &key) const;

    std::vector<std::unique_ptr<string_entry>> m_stringentries;
    QDataStream *stream;
    qint64 offset;
    quint32 hashTableSize;
    QList<qint32> hashList;
};

KSycocaDict::KSycocaDict()
    : d(new KSycocaDictPrivate)
{
}

KSycocaDict::KSycocaDict(QDataStream *str, int offset)
    : d(new KSycocaDictPrivate)
{
    d->stream = str;
    d->offset = offset;

    quint32 test1;
    quint32 test2;
    str->device()->seek(offset);
    (*str) >> test1 >> test2;
    if ((test1 > 0x000fffff) || (test2 > 1024)) {
        KSycoca::flagError();
        d->hashTableSize = 0;
        d->offset = 0;
        return;
    }

    str->device()->seek(offset);
    (*str) >> d->hashTableSize;
    (*str) >> d->hashList;
    d->offset = str->device()->pos(); // Start of hashtable
}

KSycocaDict::~KSycocaDict() = default;

void KSycocaDict::add(const QString &key, const KSycocaEntry::Ptr &payload)
{
    if (key.isEmpty()) {
        return; // Not allowed (should never happen)
    }
    if (!payload) {
        return; // Not allowed!
    }

    d->m_stringentries.push_back(std::make_unique<string_entry>(key, payload));
}

void KSycocaDict::remove(const QString &key)
{
    if (!d) {
        return;
    }

    auto it = std::find_if(d->m_stringentries.begin(), d->m_stringentries.end(), [&key](const std::unique_ptr<string_entry> &entry) {
        return entry->keyStr == key;
    });

    if (it != d->m_stringentries.end()) {
        d->m_stringentries.erase(it);
    } else {
        qCDebug(SYCOCA) << "key not found:" << key;
    }
}

int KSycocaDict::find_string(const QString &key) const
{
    Q_ASSERT(d);

    // qCDebug(SYCOCA) << QString("KSycocaDict::find_string(%1)").arg(key);
    qint32 offset = d->offsetForKey(key);

    // qCDebug(SYCOCA) << QString("offset is %1").arg(offset,8,16);
    if (offset == 0) {
        return 0;
    }

    if (offset > 0) {
        return offset; // Positive ID
    }

    // Lookup duplicate list.
    offset = -offset;

    d->stream->device()->seek(offset);
    // qCDebug(SYCOCA) << QString("Looking up duplicate list at %1").arg(offset,8,16);

    while (true) {
        (*d->stream) >> offset;
        if (offset == 0) {
            break;
        }
        QString dupkey;
        (*d->stream) >> dupkey;
        // qCDebug(SYCOCA) << QString(">> %1 %2").arg(offset,8,16).arg(dupkey);
        if (dupkey == key) {
            return offset;
        }
    }
    // qCDebug(SYCOCA) << "Not found!";

    return 0;
}

QList<int> KSycocaDict::findMultiString(const QString &key) const
{
    qint32 offset = d->offsetForKey(key);
    QList<int> offsetList;
    if (offset == 0) {
        return offsetList;
    }

    if (offset > 0) { // Positive ID: one entry found
        offsetList.append(offset);
        return offsetList;
    }

    // Lookup duplicate list.
    offset = -offset;

    d->stream->device()->seek(offset);
    // qCDebug(SYCOCA) << QString("Looking up duplicate list at %1").arg(offset,8,16);

    while (true) {
        (*d->stream) >> offset;
        if (offset == 0) {
            break;
        }
        QString dupkey;
        (*d->stream) >> dupkey;
        // qCDebug(SYCOCA) << QString(">> %1 %2").arg(offset,8,16).arg(dupkey);
        if (dupkey == key) {
            offsetList.append(offset);
        }
    }
    return offsetList;
}

uint KSycocaDict::count() const
{
    if (!d) {
        return 0;
    }

    return d->m_stringentries.size();
}

void KSycocaDict::clear()
{
    d.reset();
}

uint KSycocaDictPrivate::hashKey(const QString &key) const
{
    int len = key.length();
    uint h = 0;

    for (int i = 0; i < hashList.count(); i++) {
        int pos = hashList[i];
        if (pos == 0) {
            continue;
        } else if (pos < 0) {
            pos = -pos;
            if (pos < len) {
                h = ((h * 13) + (key[len - pos].cell() % 29)) & 0x3ffffff;
            }
        } else {
            pos = pos - 1;
            if (pos < len) {
                h = ((h * 13) + (key[pos].cell() % 29)) & 0x3ffffff;
            }
        }
    }
    return h;
}

// If we have the strings
//    hello
//    world
//    kde
// Then we end up with
//    ABCDE
// where A = diversity of 'h' + 'w' + 'k' etc.
// Also, diversity(-2) == 'l'+'l'+'d' (second character from the end)

// The hasList is used for hashing:
//  hashList = (-2, 1, 3) means that the hash key comes from
//  the 2nd character from the right, then the 1st from the left, then the 3rd from the left.

// Calculate the diversity of the strings at position 'pos'
// NOTE: this code is slow, it takes 12% of the _overall_ `kbuildsycoca5 --noincremental` running time
static int calcDiversity(std::vector<std::unique_ptr<string_entry>> *stringlist, int inPos, uint sz)
{
    if (inPos == 0) {
        return 0;
    }
    QBitArray matrix(sz);
    int pos;

    // static const int s_maxItems = 50;
    // int numItem = 0;

    if (inPos < 0) {
        pos = -inPos;
        for (const auto &entryPtr : *stringlist) {
            const int rpos = entryPtr->length - pos;
            if (rpos > 0) {
                const uint hash = ((entryPtr->hash * 13) + (entryPtr->key[rpos].cell() % 29)) & 0x3ffffff;
                matrix.setBit(hash % sz, true);
            }
            // if (++numItem == s_maxItems)
            //    break;
        }
    } else {
        pos = inPos - 1;
        for (const auto &entryPtr : *stringlist) {
            if (pos < entryPtr->length) {
                const uint hash = ((entryPtr->hash * 13) + (entryPtr->key[pos].cell() % 29)) & 0x3ffffff;
                matrix.setBit(hash % sz, true);
            }
            // if (++numItem == s_maxItems)
            //    break;
        }
    }
    return matrix.count(true);
}

//
// Add the diversity of the strings at position 'pos'
static void addDiversity(std::vector<std::unique_ptr<string_entry>> *stringlist, int pos)
{
    if (pos == 0) {
        return;
    }

    if (pos < 0) {
        pos = -pos;
        for (auto &entryPtr : *stringlist) {
            const int rpos = entryPtr->length - pos;
            if (rpos > 0) {
                entryPtr->hash = ((entryPtr->hash * 13) + (entryPtr->key[rpos].cell() % 29)) & 0x3fffffff;
            }
        }
    } else {
        pos = pos - 1;
        for (auto &entryPtr : *stringlist) {
            if (pos < entryPtr->length) {
                entryPtr->hash = ((entryPtr->hash * 13) + (entryPtr->key[pos].cell() % 29)) & 0x3fffffff;
            }
        }
    }
}

void KSycocaDict::save(QDataStream &str)
{
    if (count() == 0) {
        d->hashTableSize = 0;
        d->hashList.clear();
        str << d->hashTableSize;
        str << d->hashList;
        return;
    }

    d->offset = str.device()->pos();

    // qCDebug(SYCOCA) << "KSycocaDict:" << count() << "entries.";

    // qCDebug(SYCOCA) << "Calculating hash keys..";

    int maxLength = 0;
    // qCDebug(SYCOCA) << "Finding maximum string length";
    for (auto &entryPtr : d->m_stringentries) {
        entryPtr->hash = 0;
        if (entryPtr->length > maxLength) {
            maxLength = entryPtr->length;
        }
    }

    // qCDebug(SYCOCA) << "Max string length=" << maxLength << "existing hashList=" << d->hashList;

    // use "almost prime" number for sz (to calculate diversity) and later
    // for the table size of big tables
    // int sz = d->stringlist.count()*5-1;
    unsigned int sz = count() * 4 + 1;
    while (!(((sz % 3) && (sz % 5) && (sz % 7) && (sz % 11) && (sz % 13)))) {
        sz += 2;
    }

    d->hashList.clear();

    // Times (with warm caches, i.e. after multiple runs)
    // kbuildsycoca5 --noincremental  2.83s user 0.20s system 95% cpu 3.187 total
    // kbuildsycoca5 --noincremental  2.74s user 0.25s system 93% cpu 3.205 total
    // unittest: 0.50-60 msec per iteration / 0.40-50 msec per iteration

    // Now that MimeTypes are not parsed anymore:
    // kbuildsycoca5 --noincremental  2.18s user 0.30s system 91% cpu 2.719 total
    // kbuildsycoca5 --noincremental  2.07s user 0.34s system 89% cpu 2.681 total

    // If I enabled s_maxItems = 50, it goes down to
    // but I don't know if that's a good idea.
    // kbuildsycoca5 --noincremental  1.73s user 0.31s system 85% cpu 2.397 total
    // kbuildsycoca5 --noincremental  1.84s user 0.29s system 95% cpu 2.230 total

    // try to limit diversity scan by "predicting" positions
    // with high diversity
    QList<int> oldvec(maxLength * 2 + 1);
    oldvec.fill(0);
    int mindiv = 0;
    int lastDiv = 0;

    while (true) {
        int divsum = 0;
        int divnum = 0;

        int maxDiv = 0;
        int maxPos = 0;
        for (int pos = -maxLength; pos <= maxLength; ++pos) {
            // cut off
            if (oldvec[pos + maxLength] < mindiv) {
                oldvec[pos + maxLength] = 0;
                continue;
            }

            const int diversity = calcDiversity(&(d->m_stringentries), pos, sz);
            if (diversity > maxDiv) {
                maxDiv = diversity;
                maxPos = pos;
            }
            oldvec[pos + maxLength] = diversity;
            divsum += diversity;
            ++divnum;
        }
        // arbitrary cut-off value 3/4 of average seems to work
        if (divnum) {
            mindiv = (3 * divsum) / (4 * divnum);
        }

        if (maxDiv <= lastDiv) {
            break;
        }
        // qCDebug(SYCOCA) << "Max Div=" << maxDiv << "at pos" << maxPos;
        lastDiv = maxDiv;
        addDiversity(&(d->m_stringentries), maxPos);
        d->hashList.append(maxPos);
    }

    for (auto &entryPtr : d->m_stringentries) {
        entryPtr->hash = d->hashKey(entryPtr->keyStr);
    }
    // fprintf(stderr, "Calculating minimum table size..\n");

    d->hashTableSize = sz;

    // qCDebug(SYCOCA) << "hashTableSize=" << sz << "hashList=" << d->hashList << "oldvec=" << oldvec;

    struct hashtable_entry {
        string_entry *entry;
        QList<string_entry *> *duplicates;
        qint64 duplicate_offset;
    };

    hashtable_entry *hashTable = new hashtable_entry[sz];

    // qCDebug(SYCOCA) << "Clearing hashtable...";
    for (unsigned int i = 0; i < sz; i++) {
        hashTable[i].entry = nullptr;
        hashTable[i].duplicates = nullptr;
    }

    // qCDebug(SYCOCA) << "Filling hashtable...";
    for (const auto &entryPtr : d->m_stringentries) {
        // qCDebug(SYCOCA) << "entry keyStr=" << entry->keyStr << entry->payload.data() << entry->payload->entryPath();
        const int hash = entryPtr->hash % sz;
        if (!hashTable[hash].entry) {
            // First entry
            hashTable[hash].entry = entryPtr.get();
        } else {
            if (!hashTable[hash].duplicates) {
                // Second entry, build duplicate list.
                hashTable[hash].duplicates = new QList<string_entry *>;
                hashTable[hash].duplicates->append(hashTable[hash].entry);
                hashTable[hash].duplicate_offset = 0;
            }
            hashTable[hash].duplicates->append(entryPtr.get());
        }
    }

    str << d->hashTableSize;
    str << d->hashList;

    d->offset = str.device()->pos(); // d->offset points to start of hashTable
    // qCDebug(SYCOCA) << QString("Start of Hash Table, offset = %1").arg(d->offset,8,16);

    // Write the hashtable + the duplicates twice.
    // The duplicates are after the normal hashtable, but the offset of each
    // duplicate entry is written into the normal hashtable.
    for (int pass = 1; pass <= 2; pass++) {
        str.device()->seek(d->offset);
        // qCDebug(SYCOCA) << QString("Writing hash table (pass #%1)").arg(pass);
        for (uint i = 0; i < d->hashTableSize; i++) {
            qint32 tmpid;
            if (!hashTable[i].entry) {
                tmpid = 0;
            } else if (!hashTable[i].duplicates) {
                tmpid = hashTable[i].entry->payload->offset(); // Positive ID
            } else {
                tmpid = -hashTable[i].duplicate_offset; // Negative ID
            }
            str << tmpid;
            // qCDebug(SYCOCA) << QString("Hash table : %1").arg(tmpid,8,16);
        }
        // qCDebug(SYCOCA) << QString("End of Hash Table, offset = %1").arg(str.device()->at(),8,16);

        // qCDebug(SYCOCA) << QString("Writing duplicate lists (pass #%1)").arg(pass);
        for (uint i = 0; i < d->hashTableSize; i++) {
            const QList<string_entry *> *dups = hashTable[i].duplicates;
            if (dups) {
                hashTable[i].duplicate_offset = str.device()->pos();

                /*qCDebug(SYCOCA) << QString("Duplicate lists: Offset = %1 list_size = %2") .arg(hashTable[i].duplicate_offset,8,16).arg(dups->count());
                 */
                for (string_entry *dup : std::as_const(*dups)) {
                    const qint32 offset = dup->payload->offset();
                    if (!offset) {
                        const QString storageId = dup->payload->storageId();
                        qCDebug(SYCOCA) << "about to assert! dict=" << this << "storageId=" << storageId << dup->payload.data();
                        if (dup->payload->isType(KST_KService)) {
                            KService::Ptr service(static_cast<KService *>(dup->payload.data()));
                            qCDebug(SYCOCA) << service->storageId() << service->entryPath();
                        }
                        // save() must have been called on the entry
                        Q_ASSERT_X(offset,
                                   "KSycocaDict::save",
                                   QByteArray("entry offset is 0, save() was not called on " + dup->payload->storageId().toLatin1()
                                              + " entryPath=" + dup->payload->entryPath().toLatin1())
                                       .constData());
                    }
                    str << offset; // Positive ID
                    str << dup->keyStr; // Key (QString)
                }
                str << qint32(0); // End of list marker (0)
            }
        }
        // qCDebug(SYCOCA) << QString("End of Dict, offset = %1").arg(str.device()->at(),8,16);
    }

    // qCDebug(SYCOCA) << "Cleaning up hash table.";
    for (uint i = 0; i < d->hashTableSize; i++) {
        delete hashTable[i].duplicates;
    }
    delete[] hashTable;
}

qint32 KSycocaDictPrivate::offsetForKey(const QString &key) const
{
    if (!stream || !offset) {
        qCWarning(SYCOCA) << "No ksycoca database available! Tried running" << KBUILDSYCOCA_EXENAME << "?";
        return 0;
    }

    if (hashTableSize == 0) {
        return 0; // Unlikely to find anything :-]
    }

    // Read hash-table data
    const uint hash = hashKey(key) % hashTableSize;
    // qCDebug(SYCOCA) << "hash is" << hash;

    const qint64 off = offset + sizeof(qint32) * hash;
    // qCDebug(SYCOCA) << QString("off is %1").arg(off,8,16);
    stream->device()->seek(off);

    qint32 retOffset;
    (*stream) >> retOffset;
    return retOffset;
}
