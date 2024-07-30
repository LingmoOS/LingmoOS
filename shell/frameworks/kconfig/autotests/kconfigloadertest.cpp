/*
    SPDX-FileCopyrightText: 2010 Martin Blumenstingl <darklight.xdarklight@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kconfigloadertest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kconfigloader.h>
#include <kconfigskeleton.h>

Q_DECLARE_METATYPE(QList<int>)

static const QString s_testName(QStringLiteral("kconfigloadertest")); // clazy:exclude=non-pod-global-static

#define GET_CONFIG_ITEM_VALUE(type, configName)                                                                                                                \
    KConfigSkeletonItem *item = cl->findItem(s_testName, configName);                                                                                          \
    /* Check if we got back a valid item. */                                                                                                                   \
    QVERIFY(item != nullptr);                                                                                                                                  \
    /* Cast the item to the given type. */                                                                                                                     \
    type typeItem = dynamic_cast<type>(item);                                                                                                                  \
    /* Make sure the cast was successful. */                                                                                                                   \
    QVERIFY(typeItem != nullptr);

void ConfigLoaderTest::init()
{
    QString fileName = s_testName + QLatin1String(".xml");
    configFile = new QFile(QFINDTESTDATA(QString::fromLatin1("/") + fileName));
    cl = new KConfigLoader(configFile->fileName(), configFile);
}

void ConfigLoaderTest::cleanup()
{
    delete cl;
    delete configFile;
}

void ConfigLoaderTest::boolDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemBool *, QStringLiteral("DefaultBoolItem"));

    QVERIFY(typeItem->isEqual(true));
}

void ConfigLoaderTest::colorDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemColor *, QStringLiteral("DefaultColorItem"));

    QVERIFY(typeItem->isEqual(QColor("#00FF00")));
}

void ConfigLoaderTest::dateTimeDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemDateTime *, QStringLiteral("DefaultDateTimeItem"));

    QVERIFY(typeItem->isEqual(QDateTime::fromString(QStringLiteral("Thu Sep 09 2010"))));
}

void ConfigLoaderTest::enumDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemEnum *, QStringLiteral("DefaultEnumItem"));

    QVERIFY(typeItem->isEqual(3));
}

void ConfigLoaderTest::enumDefaultValueString()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemEnum *, QStringLiteral("DefaultEnumItemString"));

    QVERIFY(typeItem->isEqual(2));
}

void ConfigLoaderTest::fontDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemFont *, QStringLiteral("DefaultFontItem"));

    QVERIFY(typeItem->isEqual(QFont(QStringLiteral("DejaVu Sans"))));
}

void ConfigLoaderTest::intDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemInt *, QStringLiteral("DefaultIntItem"));

    QVERIFY(typeItem->isEqual(27));
}

void ConfigLoaderTest::passwordDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemPassword *, QStringLiteral("DefaultPasswordItem"));

    QVERIFY(typeItem->isEqual(QString::fromLatin1("h4x.")));
}

void ConfigLoaderTest::pathDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemPath *, QStringLiteral("DefaultPathItem"));

    QVERIFY(typeItem->isEqual(QString::fromLatin1("/dev/null")));
}

void ConfigLoaderTest::stringDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemString *, QStringLiteral("DefaultStringItem"));

    QVERIFY(typeItem->isEqual(QString::fromLatin1("TestString")));
}

void ConfigLoaderTest::stringListDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemStringList *, QStringLiteral("DefaultStringListItem"));

    // Create a string list with the expected values.
    QStringList expected;
    expected.append(QStringLiteral("One"));
    expected.append(QStringLiteral("Two"));
    expected.append(QStringLiteral("Three"));
    expected.append(QStringLiteral("Four"));
    expected.append(QStringLiteral("Five"));

    QVERIFY(typeItem->isEqual(expected));
}

void ConfigLoaderTest::uintDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemUInt *, QStringLiteral("DefaultUIntItem"));

    QVERIFY(typeItem->isEqual(7U));
}

void ConfigLoaderTest::urlDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemUrl *, QStringLiteral("DefaultUrlItem"));

    QVERIFY(typeItem->isEqual(QUrl(QStringLiteral("http://kde.org"))));
}

void ConfigLoaderTest::doubleDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemDouble *, QStringLiteral("DefaultDoubleItem"));

    QVERIFY(typeItem->isEqual(13.37));
}

void ConfigLoaderTest::intListDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemIntList *, QStringLiteral("DefaultIntListItem"));

    // Create a int list with the expected values.
    QList<int> expected;
    expected.append(1);
    expected.append(1);
    expected.append(2);
    expected.append(3);
    expected.append(5);
    expected.append(8);

    QVERIFY(typeItem->isEqual(QVariant::fromValue(expected)));
}

void ConfigLoaderTest::intListEmptyDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemIntList *, QStringLiteral("EmptyDefaultIntListItem"));

    QVERIFY(typeItem->value().isEmpty());
}

void ConfigLoaderTest::longLongDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemLongLong *, QStringLiteral("DefaultLongLongItem"));

    QVERIFY(typeItem->isEqual(Q_INT64_C(-9211372036854775808)));
}

void ConfigLoaderTest::pointDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemPoint *, QStringLiteral("DefaultPointItem"));

    QVERIFY(typeItem->isEqual(QPoint(185, 857)));
}

void ConfigLoaderTest::pointFDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemPointF *, QStringLiteral("DefaultPointFItem"));

    QVERIFY(typeItem->isEqual(QPointF(185.5, 857.5)));
}

void ConfigLoaderTest::rectDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemRect *, QStringLiteral("DefaultRectItem"));

    // Create a new QRect with the expected value.
    QRect expected;
    expected.setCoords(3, 7, 951, 358);

    QVERIFY(typeItem->isEqual(expected));
}

void ConfigLoaderTest::rectFDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemRectF *, QStringLiteral("DefaultRectFItem"));

    // Create a new QRectF with the expected value.
    QRectF expected;
    expected.setCoords(3.5, 7.5, 951.5, 358.5);

    QVERIFY(typeItem->isEqual(expected));
}

void ConfigLoaderTest::sizeDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemSize *, QStringLiteral("DefaultSizeItem"));

    QVERIFY(typeItem->isEqual(QSize(640, 480)));
}

void ConfigLoaderTest::sizeFDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemSizeF *, QStringLiteral("DefaultSizeFItem"));

    QVERIFY(typeItem->isEqual(QSizeF(640.5, 480.5)));
}

void ConfigLoaderTest::ulongLongDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemULongLong *, QStringLiteral("DefaultULongLongItem"));

    QVERIFY(typeItem->isEqual(Q_UINT64_C(9223372036854775806)));
}

QTEST_MAIN(ConfigLoaderTest)

#include "moc_kconfigloadertest.cpp"
