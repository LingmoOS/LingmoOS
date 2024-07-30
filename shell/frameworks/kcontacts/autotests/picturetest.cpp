/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "picturetest.h"
#include "kcontacts/picture.h"
#include <QBuffer>
#include <QPainter>
#include <QTest>

QTEST_MAIN(PictureTest)

static QImage testImage()
{
    static QImage image;

    if (image.isNull()) {
        image = QImage(200, 200, QImage::Format_RGB32);
        QPainter p(&image);
        p.drawRect(10, 10, 50, 50);
    }

    return image;
}

static QByteArray testImageRawPNG()
{
    static QByteArray raw;

    if (raw.isNull()) {
        QBuffer buffer(&raw);
        buffer.open(QIODevice::WriteOnly);
        testImage().save(&buffer, "PNG");
    }

    return raw;
}

static QByteArray testImageRawJPEG()
{
    static QByteArray raw;

    if (raw.isNull()) {
        QBuffer buffer(&raw);
        buffer.open(QIODevice::WriteOnly);
        testImage().save(&buffer, "JPEG");
    }

    return raw;
}

void PictureTest::emptyTest()
{
    KContacts::Picture picture;

    QVERIFY(picture.isEmpty());
}

void PictureTest::storeTestInternImage()
{
    KContacts::Picture picture;

    picture.setData(testImage());

    QVERIFY(picture.isEmpty() == false);
    QVERIFY(picture.isIntern() == true);
    QVERIFY(picture.type() == QLatin1String("jpeg"));
    QVERIFY(picture.data() == testImage());
    QVERIFY(picture.rawData() == testImageRawJPEG());
}

void PictureTest::storeTestInternRawData()
{
    KContacts::Picture picture;

    picture.setRawData(testImageRawPNG(), QStringLiteral("png"));

    QVERIFY(picture.isEmpty() == false);
    QVERIFY(picture.isIntern() == true);
    QVERIFY(picture.type() == QLatin1String("png"));
    QVERIFY(picture.rawData() == testImageRawPNG());
    QVERIFY(picture.data() == testImage());
}

void PictureTest::storeTestExtern()
{
    KContacts::Picture picture;

    picture.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    QVERIFY(picture.isEmpty() == false);
    QVERIFY(picture.isIntern() == false);
    QVERIFY(picture.type() == QLatin1String("png"));
    QVERIFY(picture.url() == QLatin1String("http://myhomepage.com/foto.png"));
}

void PictureTest::equalsTestInternImage()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setData(testImage());

    picture2.setData(testImage());

    QVERIFY(picture1 == picture2);

    // access rawData() so a QByteArray is created
    QVERIFY(!picture1.rawData().isNull());
    QVERIFY(picture1 == picture2);
}

void PictureTest::equalsTestInternRawData()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setRawData(testImageRawPNG(), QStringLiteral("png"));

    picture2.setRawData(testImageRawPNG(), QStringLiteral("png"));

    QVERIFY(picture1 == picture2);

    // access data() so a QImage is created
    QVERIFY(!picture1.data().isNull());
    QVERIFY(picture1 == picture2);
}

void PictureTest::equalsTestInternImageAndRawData()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setData(testImage());

    picture2.setRawData(testImageRawJPEG(), QStringLiteral("jpeg"));

    QVERIFY(picture1.rawData() == picture2.rawData());
}

void PictureTest::equalsTestExtern()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    picture2.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    QVERIFY(picture1 == picture2);
}

void PictureTest::differsTest()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    picture2.setData(testImage());

    QVERIFY(picture1 != picture2);
}

void PictureTest::differsTestInternRawData()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setRawData(testImageRawJPEG(), QStringLiteral("jpeg"));

    picture2.setRawData(testImageRawPNG(), QStringLiteral("png"));

    QVERIFY(picture1 != picture2);
    QVERIFY(picture1.rawData() != picture2.rawData());
}

void PictureTest::differsTestExtern()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto2.png"), QStringLiteral("png"));

    QVERIFY(picture1 != picture2);
}

void PictureTest::assignmentTestIntern()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setData(testImage());

    picture2 = picture1;

    QVERIFY(picture1 == picture2);
}

void PictureTest::assignmentTestExtern()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    picture2 = picture1;

    QVERIFY(picture1 == picture2);
}

void PictureTest::serializeTestInternImage()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setData(testImage());

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << picture1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> picture2;

    QVERIFY(picture1 == picture2);
}

void PictureTest::serializeTestInternRawData()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setRawData(testImageRawPNG(), QStringLiteral("png"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << picture1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> picture2;

    QVERIFY(picture1 == picture2);
}

void PictureTest::serializeTestExtern()
{
    KContacts::Picture picture1;
    KContacts::Picture picture2;

    picture1.setUrl(QStringLiteral("http://myhomepage.com/foto.png"), QStringLiteral("png"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << picture1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> picture2;

    QVERIFY(picture1 == picture2);
}

#include "moc_picturetest.cpp"
