/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <qdatetime.h>
#include <qdebug.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qvarlengtharray.h>

#include <CoreFoundation/CoreFoundation.h>

#include <sys/sysctl.h>

/* helper classes to convert from CF types to Qt */

static QString q_toString(const CFStringRef &str)
{
    CFIndex length = CFStringGetLength(str);
    QVarLengthArray<UniChar> buffer(length);

    CFRange range = {0, length};
    CFStringGetCharacters(str, range, buffer.data());
    return QString(reinterpret_cast<const QChar *>(buffer.data()), length);
}

template<typename T>
static inline T convertCFNumber(const CFNumberRef &num, CFNumberType type)
{
    T n;
    CFNumberGetValue(num, type, &n);
    return n;
}

static QVariant q_toVariant(const CFTypeRef &obj)
{
    const CFTypeID typeId = CFGetTypeID(obj);

    if (typeId == CFStringGetTypeID()) {
        return QVariant(q_toString(static_cast<const CFStringRef>(obj)));
    }

    if (typeId == CFNumberGetTypeID()) {
        const CFNumberRef num = static_cast<const CFNumberRef>(obj);
        const CFNumberType type = CFNumberGetType(num);
        switch (type) {
        case kCFNumberSInt8Type:
            return QVariant::fromValue(convertCFNumber<char>(num, type));
        case kCFNumberSInt16Type:
            return QVariant::fromValue(convertCFNumber<qint16>(num, type));
        case kCFNumberSInt32Type:
            return QVariant::fromValue(convertCFNumber<qint32>(num, type));
        case kCFNumberSInt64Type:
            return QVariant::fromValue(convertCFNumber<qint64>(num, type));
        case kCFNumberCharType:
            return QVariant::fromValue(convertCFNumber<uchar>(num, type));
        case kCFNumberShortType:
            return QVariant::fromValue(convertCFNumber<short>(num, type));
        case kCFNumberIntType:
            return QVariant::fromValue(convertCFNumber<int>(num, type));
        case kCFNumberLongType:
            return QVariant::fromValue(convertCFNumber<long>(num, type));
        case kCFNumberLongLongType:
            return QVariant::fromValue(convertCFNumber<long long>(num, type));
        case kCFNumberFloatType:
            return QVariant::fromValue(convertCFNumber<float>(num, type));
        case kCFNumberDoubleType:
            return QVariant::fromValue(convertCFNumber<double>(num, type));
        default:
            if (CFNumberIsFloatType(num)) {
                return QVariant::fromValue(convertCFNumber<double>(num, kCFNumberDoubleType));
            }
            return QVariant::fromValue(convertCFNumber<quint64>(num, kCFNumberLongLongType));
        }
    }

    if (typeId == CFDateGetTypeID()) {
        QDateTime dt;
        dt.setSecsSinceEpoch(qint64(kCFAbsoluteTimeIntervalSince1970));
        return dt.addSecs(int(CFDateGetAbsoluteTime(static_cast<const CFDateRef>(obj))));
    }

    if (typeId == CFDataGetTypeID()) {
        const CFDataRef cfdata = static_cast<const CFDataRef>(obj);
        return QByteArray(reinterpret_cast<const char *>(CFDataGetBytePtr(cfdata)), CFDataGetLength(cfdata));
    }

    if (typeId == CFBooleanGetTypeID()) {
        return QVariant(bool(CFBooleanGetValue(static_cast<const CFBooleanRef>(obj))));
    }

    if (typeId == CFArrayGetTypeID()) {
        const CFArrayRef cfarray = static_cast<const CFArrayRef>(obj);
        QList<QVariant> list;
        CFIndex size = CFArrayGetCount(cfarray);
        bool metNonString = false;
        for (CFIndex i = 0; i < size; ++i) {
            QVariant value = q_toVariant(CFArrayGetValueAtIndex(cfarray, i));
            if (value.userType() != QMetaType::QString) {
                metNonString = true;
            }
            list << value;
        }
        if (metNonString) {
            return list;
        } else {
            return QVariant(list).toStringList();
        }
    }

    if (typeId == CFDictionaryGetTypeID()) {
        const CFDictionaryRef cfdict = static_cast<const CFDictionaryRef>(obj);
        const CFTypeID arrayTypeId = CFArrayGetTypeID();
        int size = int(CFDictionaryGetCount(cfdict));
        QVarLengthArray<CFPropertyListRef> keys(size);
        QVarLengthArray<CFPropertyListRef> values(size);
        CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

        QMultiMap<QString, QVariant> map;
        for (int i = 0; i < size; ++i) {
            QString key = q_toString(static_cast<const CFStringRef>(keys[i]));

            if (CFGetTypeID(values[i]) == arrayTypeId) {
                const CFArrayRef cfarray = static_cast<const CFArrayRef>(values[i]);
                CFIndex arraySize = CFArrayGetCount(cfarray);
                for (CFIndex j = arraySize - 1; j >= 0; --j) {
                    map.insert(key, q_toVariant(CFArrayGetValueAtIndex(cfarray, j)));
                }
            } else {
                map.insert(key, q_toVariant(values[i]));
            }
        }
        return QVariant::fromValue(map);
    }

    return QVariant();
}

QMap<QString, QVariant> q_toVariantMap(const CFMutableDictionaryRef &dict)
{
    Q_ASSERT(dict);

    QMap<QString, QVariant> result;

    const int count = CFDictionaryGetCount(dict);
    QVarLengthArray<void *> keys(count);
    QVarLengthArray<void *> values(count);

    CFDictionaryGetKeysAndValues(dict, const_cast<const void **>(keys.data()), const_cast<const void **>(values.data()));

    for (int i = 0; i < count; ++i) {
        const QString key = q_toString((CFStringRef)keys[i]);
        const QVariant value = q_toVariant((CFTypeRef)values[i]);
        result[key] = value;
    }

    return result;
}

bool q_sysctlbyname(const char *name, QString &result)
{
    char *property = nullptr;
    size_t size = 0;
    int error = 0;
    if (name && sysctlbyname(name, nullptr, &size, nullptr, 0) == 0 && size > 0) {
        property = new char[size];
        error = sysctlbyname(name, property, &size, nullptr, 0);
        if (!error) {
            result = QLatin1String(property);
        }
        delete[] property;
    }
    return !error;
}
