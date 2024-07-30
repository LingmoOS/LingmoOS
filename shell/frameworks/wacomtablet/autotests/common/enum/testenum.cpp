/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/enum.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testenum.cpp
 *
 * @test UnitTest for the device properties
 */
class TestEnum : public QObject
{
    Q_OBJECT

private slots:
    void testCompare();
    void testConstructor();
    void testFind();
    void testIterator();
    void testKey();
    void testKeys();
    void testList();
    void testOperator();
    void testSize();
};

QTEST_MAIN(TestEnum)

/*
 * Forward Declarations, Typedefs & Helper classes.
 */
class EnumTest;
struct EnumTestTemplateSpecializationLessFunctor;
struct EnumTestTemplateSpecializationEqualFunctor;

typedef Enum<EnumTest, QString, EnumTestTemplateSpecializationLessFunctor, EnumTestTemplateSpecializationEqualFunctor> EnumTestTemplateSpecialization;

struct EnumTestTemplateSpecializationLessFunctor {
    bool operator()(const EnumTestTemplateSpecialization *v1, const EnumTestTemplateSpecialization *v2)
    {
        return (v1->key() < v2->key());
    }
};

struct EnumTestTemplateSpecializationEqualFunctor {
    bool operator()(const QString &v1, const QString &v2)
    {
        return (v1.compare(v2, Qt::CaseInsensitive) == 0);
    }
};

// template specialization of helper class
template<>
EnumTestTemplateSpecialization::Container EnumTestTemplateSpecialization::instances = EnumTestTemplateSpecialization::Container();

/*
 * A helper class required for this unit test.
 */
class EnumTest : public EnumTestTemplateSpecialization
{
private:
    explicit EnumTest(const QString &value)
        : EnumTestTemplateSpecialization(this, value)
    {
    }

public:
    static const EnumTest VAL01_PRIO10;
    static const EnumTest VAL02_PRIO10;
    static const EnumTest VAL03_PRIO10;
    static const EnumTest VAL01_PRIO50;
    static const EnumTest VAL01_PRIO99;
    static const EnumTest VAL02_PRIO99;
};

// Do NOT reorder these instances! This has to be done by the enum itself.
const EnumTest EnumTest::VAL01_PRIO10(QLatin1String("VAL01_PRIO10"));
const EnumTest EnumTest::VAL03_PRIO10(QLatin1String("VAL03_PRIO10"));
const EnumTest EnumTest::VAL02_PRIO10(QLatin1String("VAL02_PRIO10"));
const EnumTest EnumTest::VAL02_PRIO99(QLatin1String("VAL02_PRIO99"));
const EnumTest EnumTest::VAL01_PRIO99(QLatin1String("VAL01_PRIO99"));
const EnumTest EnumTest::VAL01_PRIO50(QLatin1String("VAL01_PRIO50"));

void TestEnum::testCompare()
{
    // make sure sort order is correct
    EnumTest::const_iterator iter = EnumTest::begin();
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(iter++->key(), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(iter++->key(), QLatin1String("VAL02_PRIO99"));
    QCOMPARE(iter++->key(), QLatin1String("VAL03_PRIO10"));
}

void TestEnum::testConstructor()
{
    // copy constructor
    EnumTest test(EnumTest::VAL01_PRIO10);
    QVERIFY(test == EnumTest::VAL01_PRIO10);
}

void TestEnum::testFind()
{
    QCOMPARE(EnumTest::find(QLatin1String("NON_EXISTANT")), nullptr);

    const EnumTest *find = EnumTest::find(QLatin1String("VAL01_PRIO50"));
    QVERIFY(find != nullptr);
    QCOMPARE(*find, EnumTest::VAL01_PRIO50);
}

void TestEnum::testIterator()
{
    EnumTest::const_iterator begin = EnumTest::begin();
    EnumTest::const_iterator end = EnumTest::end();

    EnumTest::const_iterator iter, second;

    // begin(), operator*, operator->
    QCOMPARE(begin->key(), QLatin1String("VAL01_PRIO10"));
    QCOMPARE((*begin).key(), EnumTest::VAL01_PRIO10.key());
    QVERIFY(*begin == EnumTest::VAL01_PRIO10);
    QVERIFY(&(*begin) == &EnumTest::VAL01_PRIO10);

    // operator==
    QVERIFY(begin == EnumTest::begin());
    QVERIFY(end == EnumTest::end());

    // operator!=
    QVERIFY(begin != EnumTest::end());
    QVERIFY(end != EnumTest::begin());

    // operator=
    iter = begin;
    QVERIFY(iter == iter);
    QVERIFY(iter == begin);
    QVERIFY(iter == EnumTest::begin());

    // operator++
    second = begin;
    ++second;
    QCOMPARE(second->key(), QLatin1String("VAL01_PRIO50"));

    iter = begin;
    QVERIFY(iter++ == begin);
    QVERIFY(iter == second);

    // operator++(int)
    iter = begin;
    QVERIFY(++iter == second);

    // operator--
    iter = second;
    QVERIFY(iter-- == second);
    QVERIFY(iter == begin);

    // operator--(int)
    iter = second;
    QVERIFY(--iter == begin);

    // end()
    iter = end;
    --iter;
    QCOMPARE(iter->key(), QLatin1String("VAL03_PRIO10"));
}

void TestEnum::testKey()
{
    QCOMPARE(EnumTest::VAL01_PRIO10.key(), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(EnumTest::VAL01_PRIO50.key(), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(EnumTest::VAL01_PRIO99.key(), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(EnumTest::VAL02_PRIO10.key(), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(EnumTest::VAL02_PRIO99.key(), QLatin1String("VAL02_PRIO99"));
    QCOMPARE(EnumTest::VAL03_PRIO10.key(), QLatin1String("VAL03_PRIO10"));
}

void TestEnum::testKeys()
{
    QStringList values = EnumTest::keys();

    QStringList::ConstIterator iter = values.constBegin();
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(*(iter++), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(*(iter++), QLatin1String("VAL02_PRIO99"));
    QCOMPARE(*(iter++), QLatin1String("VAL03_PRIO10"));
}

void TestEnum::testList()
{
    QList<EnumTest> list = EnumTest::list();
    QList<EnumTest>::ConstIterator iter = list.constBegin();

    QVERIFY(*(iter++) == EnumTest::VAL01_PRIO10);
    QVERIFY(*(iter++) == EnumTest::VAL01_PRIO50);
    QVERIFY(*(iter++) == EnumTest::VAL01_PRIO99);
    QVERIFY(*(iter++) == EnumTest::VAL02_PRIO10);
    QVERIFY(*(iter++) == EnumTest::VAL02_PRIO99);
    QVERIFY(*(iter++) == EnumTest::VAL03_PRIO10);
}

void TestEnum::testOperator()
{
    // operator==
    QVERIFY(EnumTest::VAL01_PRIO10 == EnumTest::VAL01_PRIO10);
    QVERIFY(EnumTest::VAL02_PRIO10 == EnumTest::VAL02_PRIO10);

    // operator!=
    QVERIFY(EnumTest::VAL01_PRIO10 != EnumTest::VAL02_PRIO10);

    // operator=
    EnumTest test = EnumTest::VAL01_PRIO10;
    QVERIFY(test == EnumTest::VAL01_PRIO10);
}

void TestEnum::testSize()
{
    int count = 6;
    QCOMPARE((int)EnumTest::size(), count);
    QCOMPARE(EnumTest::keys().size(), count);
    QCOMPARE(EnumTest::list().size(), count);
}

#include "testenum.moc"
