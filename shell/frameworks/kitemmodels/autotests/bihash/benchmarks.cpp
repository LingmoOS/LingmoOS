/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QObject>
#include <QRandomGenerator>
#include <QTest>

#define KBIHASH 1
// #define BOOST_BIMAP 1

#define LEFT int
#define RIGHT int
// #define LEFT QString
// #define RIGHT QString

#ifdef KBIHASH
#include "kbihash_p.h"
#endif

#ifdef BOOST_BIMAP
#include <boost/bimap.hpp>
#endif

typedef QHash<LEFT, RIGHT> Hash;

#ifdef KBIHASH
typedef KBiHash<LEFT, RIGHT> Mapping;
#endif

#ifdef BOOST_BIMAP
typedef boost::bimap<LEFT, RIGHT> Mapping;

static Mapping hashToBiMap(const Hash &hash)
{
    Mapping biMap;
    Hash::const_iterator it = hash.constBegin();
    const Hash::const_iterator end = hash.constEnd();
    for (; it != end; ++it) {
        biMap.insert(Mapping::value_type(it.key(), it.value()));
    }
    return biMap;
}
#endif

// #define QBENCHMARK_ONCE

#define MAX_SIZE 25000
#define MAX_DIGITS 5

#define NEW_ROW(num) QTest::newRow(QString("%1").arg(num).toLatin1()) << num;

template<typename T>
T containedValue(int value);

template<>
int containedValue(int value)
{
    return value;
}

template<>
QString containedValue(int value)
{
    return QString("%1").arg(value, MAX_DIGITS);
}

template<typename T>
T updatedValue(int value);

template<>
int updatedValue(int value)
{
    return value + MAX_SIZE;
}

template<>
QString updatedValue(int value)
{
    return QString("%1").arg(value + MAX_SIZE, MAX_DIGITS);
}

class BiHashBenchmarks : public QObject
{
    Q_OBJECT
public:
    BiHashBenchmarks(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private:
    Hash createHash(int numElements)
    {
        Hash hash;
        hash.reserve(numElements);
        for (int i = 0; i < numElements; ++i) {
            hash.insert(containedValue<Hash::key_type>(i), containedValue<Hash::mapped_type>(i));
        }
        return hash;
    }
    void getTestData();

    QRandomGenerator m_randomGenerator;

private Q_SLOTS:

    void testInsert();
    void testLookup();
    void testReverseLookup();
    void testRemoveKey();
    void testRemoveValue();
    void testUpdateKey();
    void testUpdateValue();

    void testInsert_data();
    void testLookup_data();
    void testReverseLookup_data();
    void testRemoveKey_data();
    void testRemoveValue_data();
    void testUpdateKey_data();
    void testUpdateValue_data();
};

void BiHashBenchmarks::testInsert()
{
    QFETCH(int, numElements);

#if defined(KBIHASH)
    Mapping biHash;
    QBENCHMARK_ONCE {
        for (int i = 0; i < numElements; ++i) {
            biHash.insert(containedValue<Hash::key_type>(i), containedValue<Hash::mapped_type>(i));
        }
        biHash.clear();
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap;
    QBENCHMARK_ONCE {
        for (int i = 0; i < numElements; ++i) {
            biMap.insert(Mapping::value_type(containedValue<Hash::key_type>(i), containedValue<Hash::mapped_type>(i)));
        }
        biMap.clear();
    }
#else
    Hash hash;
    QBENCHMARK_ONCE {
        for (int i = 0; i < numElements; ++i) {
            hash.insert(containedValue<Hash::key_type>(i), containedValue<Hash::mapped_type>(i));
        }
        hash.clear();
    }
#endif
}

void BiHashBenchmarks::getTestData()
{
    QTest::addColumn<int>("numElements");

    NEW_ROW(1);
    NEW_ROW(2);
    NEW_ROW(3);
    NEW_ROW(5);
    NEW_ROW(8);
    NEW_ROW(13);
    NEW_ROW(21);
    NEW_ROW(34);

    NEW_ROW(50);
    NEW_ROW(100);
    NEW_ROW(150);
    NEW_ROW(200);
    NEW_ROW(250);

    NEW_ROW(500);
    NEW_ROW(1000);
    NEW_ROW(1500);
    NEW_ROW(2000);
    NEW_ROW(2500);

    NEW_ROW(5000);
    NEW_ROW(10000);
    NEW_ROW(15000);
    NEW_ROW(20000);
    NEW_ROW(MAX_SIZE);
}

void BiHashBenchmarks::testInsert_data()
{
    getTestData();
}

void BiHashBenchmarks::testLookup()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    Hash::mapped_type result;
    const Hash::key_type key = containedValue<Hash::key_type>(m_randomGenerator.bounded(numElements));

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    QBENCHMARK_ONCE {
        result = biHash.leftToRight(key);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    QBENCHMARK_ONCE {
        result = biMap.left[key];
    }
#else
    QBENCHMARK_ONCE {
        result = hash.value(key);
    }
#endif
    Q_UNUSED(result);
}

void BiHashBenchmarks::testLookup_data()
{
    getTestData();
}

void BiHashBenchmarks::testReverseLookup()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    Hash::key_type result;
    const Hash::mapped_type value = containedValue<Hash::mapped_type>(m_randomGenerator.bounded(numElements));

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    QBENCHMARK_ONCE {
        result = biHash.rightToLeft(value);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    QBENCHMARK_ONCE {
        result = biMap.right[value];
    }
#else
    QBENCHMARK_ONCE {
        result = hash.key(value);
    }
#endif
    Q_UNUSED(result);
}

void BiHashBenchmarks::testReverseLookup_data()
{
    getTestData();
}

void BiHashBenchmarks::testRemoveKey()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    const Hash::key_type key = containedValue<Hash::key_type>(m_randomGenerator.bounded(numElements));
    Hash::mapped_type value;

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    QBENCHMARK_ONCE {
        value = biHash.takeLeft(key);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    Mapping::size_type t;
    QBENCHMARK_ONCE {
        t = biMap.erase(key);
    }
#else
    QBENCHMARK_ONCE {
        value = hash.take(key);
    }
#endif
    Q_UNUSED(value);
}

void BiHashBenchmarks::testRemoveKey_data()
{
    getTestData();
}

void BiHashBenchmarks::testRemoveValue()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    Hash::key_type result;
    const Hash::mapped_type value = containedValue<Hash::mapped_type>(m_randomGenerator.bounded(numElements));

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    QBENCHMARK_ONCE {
        result = biHash.takeRight(value);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    Mapping::size_type t;
    QBENCHMARK_ONCE {
        t = biMap.right.erase(value);
    }
#else
    QBENCHMARK_ONCE {
        result = hash.key(value);
        hash.remove(result);
    }
#endif
    Q_UNUSED(result);
}

void BiHashBenchmarks::testRemoveValue_data()
{
    getTestData();
}

void BiHashBenchmarks::testUpdateKey()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    const int num = m_randomGenerator.bounded(numElements);
    const Hash::key_type oldKey = containedValue<Hash::key_type>(num);
    const Hash::key_type newKey = updatedValue<Hash::key_type>(num);

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    QBENCHMARK_ONCE {
        Mapping::right_iterator it = biHash.findRight(biHash.leftToRight(oldKey));
        biHash.updateLeft(it, newKey);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    QBENCHMARK_ONCE {
        Mapping::right_iterator it = biMap.left.find(oldKey);
        it->first = newKey;
    }
#else
    QBENCHMARK_ONCE {
        const Hash::mapped_type value = hash.take(oldKey);
        hash.insert(newKey, value);
    }
#endif
}

void BiHashBenchmarks::testUpdateKey_data()
{
    getTestData();
}

void BiHashBenchmarks::testUpdateValue()
{
    QFETCH(int, numElements);
    Hash hash = createHash(numElements);

    const int num = m_randomGenerator.bounded(numElements);
    const Hash::key_type key = containedValue<Hash::key_type>(num);

#if defined(KBIHASH)
    Mapping biHash = Mapping::fromHash(hash);
    const Hash::mapped_type newValue = updatedValue<Hash::mapped_type>(num);
    QBENCHMARK_ONCE {
        Mapping::left_iterator it = biHash.findLeft(key);
        biHash.updateRight(it, newValue);
    }
#elif defined(BOOST_BIMAP)
    Mapping biMap = hashToBiMap(hash);
    const Hash::mapped_type newValue = updatedValue<Hash::mapped_type>(num);
    QBENCHMARK_ONCE {
        Mapping::left_iterator it = biMap.left.find(key);
        it->second = newValue;
    }
#else
    const Hash::mapped_type newValue = updatedValue<Hash::mapped_type>(num);
    QBENCHMARK_ONCE {
        hash[key] = newValue;
    }
#endif
}

void BiHashBenchmarks::testUpdateValue_data()
{
    getTestData();
}

QTEST_MAIN(BiHashBenchmarks)
#include "benchmarks.moc"
