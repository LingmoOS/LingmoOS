/*
    This file is part of the Kate project.
    SPDX-FileCopyrightText: 2021 Jan Paul Batrina <jpmbatrina01@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "encodingtest.h"
#include "katetextbuffer.h"

QTEST_MAIN(KateEncodingTest)

void KateEncodingTest::utfBomTest()
{
    // setup stuff
    Kate::TextBuffer buffer(nullptr);
    buffer.setFallbackTextCodec(QStringLiteral("UTF-8"));
    bool encodingErrors;
    bool tooLongLinesWrapped;
    bool success;
    int longestLineLoaded;
    QString prefixText;

    // utf8 tests
    buffer.setTextCodec(QStringLiteral("UTF-8"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf8.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(!buffer.generateByteOrderMark()); // file has no bom
    // since the utf8 bom is 3 bytes, the first 3 chars should not be the bom
    prefixText = buffer.text().left(3);
    QCOMPARE(prefixText, QLatin1String("Tes"));

    buffer.setTextCodec(QStringLiteral("UTF-8"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf8-bom-only.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(buffer.generateByteOrderMark());
    // aside from bom, file is empty so there should be no text
    prefixText = buffer.text();
    QVERIFY(prefixText.isEmpty());

    // utf16 tests, we need to allow encoding detection as if the byteorder is not ok, QStringDecoder will not show errors
    buffer.setTextCodec(QStringLiteral("UTF-16"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf16.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, false);
    QVERIFY(success && !encodingErrors);
    QVERIFY(buffer.generateByteOrderMark());
    // since the utf16 bom is 2 bytes, the first 2 chars should not be the bom
    prefixText = buffer.text().left(2);
    QCOMPARE(prefixText, QLatin1String("Te"));

    buffer.setTextCodec(QStringLiteral("UTF-16"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf16be.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, false);
    QVERIFY(success && !encodingErrors);
    QVERIFY(buffer.generateByteOrderMark());
    // since the utf16 bom is 2 bytes, the first 2 chars should not be the bom
    prefixText = buffer.text().left(2);
    QCOMPARE(prefixText, QLatin1String("Te"));

    // utf32 tests
    buffer.setTextCodec(QStringLiteral("UTF-32"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf32.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(buffer.generateByteOrderMark());
    // since the utf16 bom is 4 bytes, the first 4 chars should not be the bom
    prefixText = buffer.text().left(4);
    QCOMPARE(prefixText, QLatin1String("Test"));

    buffer.setTextCodec(QStringLiteral("UTF-32"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf32be.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(buffer.generateByteOrderMark());
    // since the utf16 bom is 4 bytes, the first 4 chars should not be the bom
    prefixText = buffer.text().left(4);
    QCOMPARE(prefixText, QLatin1String("Test"));

    // Ensure that a mismatching bom is not processed (e.g. utf8 bom should not be used for utf16)
    buffer.setTextCodec(QStringLiteral("UTF-16"));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/utf8-bom-only.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    // even though the file does not have a bom, Kate::TextBuffer::setTextCodec always enables bom generation
    // for utf16 and utf32 since the byte order is useful and relevant for reading the file
    QVERIFY(buffer.generateByteOrderMark());
    prefixText = buffer.text();
    // 0xFFBBEF is processed as a single char 0xBBEF, which is a hangul character
    QCOMPARE(prefixText.front(), QChar(0xBBEF));
}

void KateEncodingTest::nonUtfNoBomTest()
{
    // setup stuff
    Kate::TextBuffer buffer(nullptr);
    buffer.setFallbackTextCodec(QStringLiteral("UTF-8"));
    bool encodingErrors;
    bool tooLongLinesWrapped;
    bool success;
    int longestLineLoaded;
    QString prefixText;

    // latin15, should not contain any bom
    buffer.setTextCodec(QString::fromUtf8(QStringConverter::nameForEncoding(QStringConverter::Latin1)));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/latin15.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(!buffer.generateByteOrderMark());
    prefixText = buffer.text().left(4);
    QCOMPARE(prefixText, QLatin1String("Test"));

    // Even if a bom is somehow found, it should be processed normally as text for non-UTF char sets
    buffer.setTextCodec(QString::fromUtf8(QStringConverter::nameForEncoding(QStringConverter::Latin1)));
    success = buffer.load(QLatin1String(TEST_DATA_DIR "encoding/latin15-with-utf8-bom.txt"), encodingErrors, tooLongLinesWrapped, longestLineLoaded, true);
    QVERIFY(success && !encodingErrors);
    QVERIFY(!buffer.generateByteOrderMark()); // utf8 bom shouldn't be processed
    // the utf8 bom is 0xEFBBBF, which is "ï»¿" in Latin15
    prefixText = buffer.text().left(3);
    QCOMPARE(prefixText, QStringLiteral("ï»¿"));
}

#include "moc_encodingtest.cpp"
