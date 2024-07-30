/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2011 The LevelDB Authors. All rights reserved.

    SPDX-License-Identifier: LGPL-2.1-or-later AND BSD-3-Clause
*/

#ifndef BALOO_STORAGE_LEVELDB_UTIL_CODING_H
#define BALOO_STORAGE_LEVELDB_UTIL_CODING_H

#include <QByteArray>
#include <QVector>

namespace Baloo {

/*
 * This is a stripped down version of various encode/decode functions for
 * 32/64 bit fixed/variable data types. If you need other functions than the
 * ones available here you can take a look in the git baloo history
 */

inline void putFixed64(QByteArray* dst, quint64 value)
{
    dst->append(reinterpret_cast<const char*>(&value), sizeof(value));
}

/*
 * temporaryStorage is used to avoid an internal allocation of a temporary
 * buffer which is needed for serialization. Since this function is normally
 * called inside a loop, the temporary buffer must not be reallocated on every
 * call.
 */
void putDifferentialVarInt32(QByteArray &temporaryStorage, QByteArray* dst, const QVector<quint32>& values);
char* getDifferentialVarInt32(char* input, char* limit, QVector<quint32>* values);
extern const char* getVarint32Ptr(const char* p, const char* limit, quint32* v);

inline quint64 decodeFixed64(const char* ptr)
{
    // Load the raw bytes
    quint64 result;
    memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
    return result;
}

// Internal routine for use by fallback path of GetVarint32Ptr
extern char* getVarint32PtrFallback(char* p, char* limit, quint32* value);
inline char* getVarint32Ptr(char* p, char* limit, quint32* value)
{
    if (p >= limit) {
        return nullptr;
    }

    quint32 result = *(reinterpret_cast<const unsigned char*>(p));
    if ((result & 128) == 0) {
        *value = result;
        return p + 1;
    }

    return getVarint32PtrFallback(p, limit, value);
}

}

#endif
