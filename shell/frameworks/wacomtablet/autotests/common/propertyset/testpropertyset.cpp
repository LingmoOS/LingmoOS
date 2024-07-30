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

#include "common/property.h"
#include "common/propertyset.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testpropertyset.cpp
 *
 * @test UnitTest for the properties
 */
class TestPropertySet : public QObject
{
    Q_OBJECT

private slots:
    void testCompare();
    void testConstructor();
    void testFind();
    void testId();
    void testIds();
    void testIterator();
    void testKey();
    void testKeys();
    void testList();
    void testMap();
    void testOperator();
    void testSize();
};

QTEST_MAIN(TestPropertySet)

// forward declarations & typedefs
class PropertySetTest;
typedef PropertySet<PropertySetTest> PropertySetTestTemplateSpecialization;

/*
 * A helper class required for this unit test.
 */
class PropertySetTest : public PropertySetTestTemplateSpecialization
{
private:
    explicit PropertySetTest(const Property &id, const QString &key)
        : PropertySetTestTemplateSpecialization(this, id, key)
    {
    }

public:
    static const PropertySetTest VAL01_PRIO10;
    static const PropertySetTest VAL02_PRIO10;
    static const PropertySetTest VAL03_PRIO10;
    static const PropertySetTest VAL01_PRIO50;
    static const PropertySetTest VAL01_PRIO99;
    static const PropertySetTest VAL02_PRIO99;
};

// template specialization
template<>
PropertySetTestTemplateSpecialization::PropertySetTemplateSpecialization::Container
    PropertySetTestTemplateSpecialization::PropertySetTemplateSpecialization::instances =
        PropertySetTestTemplateSpecialization::PropertySetTemplateSpecialization::Container();

// PropertySetTest instances - the order defined here should be kept by the comparator
const PropertySetTest PropertySetTest::VAL01_PRIO50(Property::Touch, QLatin1String("VAL01_PRIO50"));
const PropertySetTest PropertySetTest::VAL03_PRIO10(Property::Gesture, QLatin1String("VAL03_PRIO10"));
const PropertySetTest PropertySetTest::VAL02_PRIO10(Property::AbsWheelUp, QLatin1String("VAL02_PRIO10"));
const PropertySetTest PropertySetTest::VAL01_PRIO10(Property::AbsWheelDown, QLatin1String("VAL01_PRIO10"));
const PropertySetTest PropertySetTest::VAL01_PRIO99(Property::StripLeftDown, QLatin1String("VAL01_PRIO99"));
const PropertySetTest PropertySetTest::VAL02_PRIO99(Property::StripLeftUp, QLatin1String("VAL02_PRIO99"));

/*
 * UNIT TESTS
 */

void TestPropertySet::testCompare()
{
    // make sure the order is the same as the definition order
    PropertySetTest::const_iterator iter = PropertySetTest::begin();
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(iter++->key(), QLatin1String("VAL03_PRIO10"));
    QCOMPARE(iter++->key(), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(iter++->key(), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(iter++->key(), QLatin1String("VAL02_PRIO99"));
}

void TestPropertySet::testConstructor()
{
    // copy constructor
    PropertySetTest test(PropertySetTest::VAL01_PRIO10);
    QVERIFY(test == PropertySetTest::VAL01_PRIO10);
}

void TestPropertySet::testFind()
{
    // find by key
    QVERIFY(PropertySetTest::find(QLatin1String("NON_EXISTANT")) == NULL);

    const PropertySetTest *findKey = PropertySetTest::find(QLatin1String("VAL01_PRIO50"));
    QVERIFY(findKey != NULL);
    QVERIFY(*findKey == PropertySetTest::VAL01_PRIO50);

    const PropertySetTest *findKeyIgnoreCase = PropertySetTest::find(QLatin1String("vAl01_PriO50"));
    QVERIFY(findKeyIgnoreCase != NULL);
    QVERIFY(*findKeyIgnoreCase == PropertySetTest::VAL01_PRIO50);
}

void TestPropertySet::testId()
{
    QVERIFY(PropertySetTest::VAL01_PRIO50.id() == Property::Touch);
    QVERIFY(PropertySetTest::VAL03_PRIO10.id() == Property::Gesture);
    QVERIFY(PropertySetTest::VAL02_PRIO10.id() == Property::AbsWheelUp);
    QVERIFY(PropertySetTest::VAL01_PRIO10.id() == Property::AbsWheelDown);
    QVERIFY(PropertySetTest::VAL01_PRIO99.id() == Property::StripLeftDown);
    QVERIFY(PropertySetTest::VAL02_PRIO99.id() == Property::StripLeftUp);
}

void TestPropertySet::testIds()
{
    QList<Property> ids = PropertySetTest::ids();

    QList<Property>::ConstIterator iter = ids.constBegin();
    QVERIFY(*(iter++) == Property::Touch);
    QVERIFY(*(iter++) == Property::Gesture);
    QVERIFY(*(iter++) == Property::AbsWheelUp);
    QVERIFY(*(iter++) == Property::AbsWheelDown);
    QVERIFY(*(iter++) == Property::StripLeftDown);
    QVERIFY(*(iter++) == Property::StripLeftUp);
}

void TestPropertySet::testIterator()
{
    PropertySetTest::const_iterator begin = PropertySetTest::begin();
    PropertySetTest::const_iterator end = PropertySetTest::end();

    PropertySetTest::const_iterator iter, second;

    // begin(), operator*, operator->
    QCOMPARE(begin->key(), QLatin1String("VAL01_PRIO50"));
    QCOMPARE((*begin).key(), PropertySetTest::VAL01_PRIO50.key());
    QVERIFY(*begin == PropertySetTest::VAL01_PRIO50);
    QVERIFY(&(*begin) == &PropertySetTest::VAL01_PRIO50);

    // operator==
    QVERIFY(begin == PropertySetTest::begin());
    QVERIFY(end == PropertySetTest::end());

    // operator!=
    QVERIFY(begin != PropertySetTest::end());
    QVERIFY(end != PropertySetTest::begin());

    // operator=
    iter = begin;
    QVERIFY(iter == iter);
    QVERIFY(iter == begin);
    QVERIFY(iter == PropertySetTest::begin());

    // operator++
    second = begin;
    ++second;
    QCOMPARE(second->key(), QLatin1String("VAL03_PRIO10"));

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
    QCOMPARE(iter->key(), QLatin1String("VAL02_PRIO99"));
}

void TestPropertySet::testKey()
{
    QCOMPARE(PropertySetTest::VAL01_PRIO50.key(), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(PropertySetTest::VAL03_PRIO10.key(), QLatin1String("VAL03_PRIO10"));
    QCOMPARE(PropertySetTest::VAL02_PRIO10.key(), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(PropertySetTest::VAL01_PRIO10.key(), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(PropertySetTest::VAL01_PRIO99.key(), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(PropertySetTest::VAL02_PRIO99.key(), QLatin1String("VAL02_PRIO99"));
}

void TestPropertySet::testKeys()
{
    QStringList keys = PropertySetTest::keys();

    QStringList::ConstIterator iter = keys.constBegin();
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO50"));
    QCOMPARE(*(iter++), QLatin1String("VAL03_PRIO10"));
    QCOMPARE(*(iter++), QLatin1String("VAL02_PRIO10"));
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO10"));
    QCOMPARE(*(iter++), QLatin1String("VAL01_PRIO99"));
    QCOMPARE(*(iter++), QLatin1String("VAL02_PRIO99"));
}

void TestPropertySet::testList()
{
    QList<PropertySetTest> list = PropertySetTest::list();
    QList<PropertySetTest>::ConstIterator iter = list.constBegin();

    QVERIFY(*(iter++) == PropertySetTest::VAL01_PRIO50);
    QVERIFY(*(iter++) == PropertySetTest::VAL03_PRIO10);
    QVERIFY(*(iter++) == PropertySetTest::VAL02_PRIO10);
    QVERIFY(*(iter++) == PropertySetTest::VAL01_PRIO10);
    QVERIFY(*(iter++) == PropertySetTest::VAL01_PRIO99);
    QVERIFY(*(iter++) == PropertySetTest::VAL02_PRIO99);
}

void TestPropertySet::testMap()
{
    QVERIFY(PropertySetTest::map(Property::Button6) == NULL);
    const PropertySetTest *findId = PropertySetTest::map(Property::Touch);
    QVERIFY(findId != NULL);
    QVERIFY(*findId == PropertySetTest::VAL01_PRIO50);
}

void TestPropertySet::testOperator()
{
    // operator==
    QVERIFY(PropertySetTest::VAL01_PRIO10 == PropertySetTest::VAL01_PRIO10);
    QVERIFY(PropertySetTest::VAL02_PRIO10 == PropertySetTest::VAL02_PRIO10);

    // operator!=
    QVERIFY(PropertySetTest::VAL01_PRIO10 != PropertySetTest::VAL02_PRIO10);

    // operator=
    PropertySetTest test = PropertySetTest::VAL01_PRIO10;
    QVERIFY(test == PropertySetTest::VAL01_PRIO10);
}

void TestPropertySet::testSize()
{
    int count = 6;
    QCOMPARE((int)PropertySetTest::size(), count);
    QCOMPARE(PropertySetTest::keys().size(), count);
    QCOMPARE(PropertySetTest::list().size(), count);
}

#include "testpropertyset.moc"
