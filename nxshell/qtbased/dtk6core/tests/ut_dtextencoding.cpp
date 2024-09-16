// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtextencoding.h"

#include <gtest/gtest.h>

#include <QLibrary>
#include <QFile>
#include <QTemporaryFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

DCORE_USE_NAMESPACE

class ut_DTextEncoding : public testing::Test
{
public:
    static void SetUpTestCase();

    bool rewriteTempFile(const QByteArray &data);
    void removeTempFile();

    static bool canLoadUchardet;
    static bool canLoadICU;
    static QString tmpFileName;

    static QByteArray dataGB18030;
    static QByteArray dataEUC_JP;
    static QByteArray dataKOI8_R;

protected:
    void TearDown() override;
};

bool ut_DTextEncoding::canLoadUchardet = false;
bool ut_DTextEncoding::canLoadICU = false;
QString ut_DTextEncoding::tmpFileName;
// Utf8 Chinese Text(Hex): 中文测试一二三四123456789abcdefgh
QByteArray ut_DTextEncoding::dataGB18030 = "\xd6\xd0\xce\xc4\xb2\xe2\xca\xd4\xd2\xbb\xb6\xfe\xc8\xfd\xcb\xc4\x31\x32\x33\x34\x35"
                                           "\x36\x37\x38\x39\x61\x62\x63\x64\x65\x66\x67\x68";
// Utf8 Japanese Text(Hex): 日本語のテスト ワン ツー スリー フォー 123456789abcdefgh
QByteArray ut_DTextEncoding::dataEUC_JP =
    "\xc6\xfc\xcb\xdc\xb8\xec\xa4\xce\xa5\xc6\xa5\xb9\xa5\xc8\x20\xa5\xef\xa5\xf3\x20\xa5\xc4\xa1\xbc\x20\xa5\xb9\xa5\xea\xa1\xbc"
    "\x20\xa5\xd5\xa5\xa9\xa1\xbc\x20\x31\x32\x33\x34\x35\x36\x37\x38\x39\x61\x62\x63\x64\x65\x66\x67\x68";
// Utf8 Russian Text(Hex): Русский тест раз два три четыре 123456789abcdefgh
QByteArray ut_DTextEncoding::dataKOI8_R =
    "\xf2\xd5\xd3\xd3\xcb\xc9\xca\x20\xd4\xc5\xd3\xd4\x20\xd2\xc1\xda\x20\xc4\xd7\xc1\x20\xd4\xd2\xc9\x20\xde\xc5\xd4\xd9\xd2\xc5"
    "\x20\x31\x32\x33\x34\x35\x36\x37\x38\x39\x61\x62\x63\x64\x65\x66\x67\x68";

void ut_DTextEncoding::SetUpTestCase()
{
    QLibrary uchardet("libuchardet", "0");
    if (!uchardet.isLoaded()) {
        canLoadUchardet = uchardet.load();
        if (canLoadUchardet) {
            uchardet.unload();
        }
    }

    QLibrary icuuc("libicuuc");
    if (icuuc.isLoaded()) {
        canLoadICU = icuuc.load();
        if (canLoadICU) {
            icuuc.unload();
        }
    }
}

bool ut_DTextEncoding::rewriteTempFile(const QByteArray &data)
{
    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        return false;
    }
    tmpFileName = tmpFile.fileName();
    tmpFile.setAutoRemove(false);

    tmpFile.write(data);
    tmpFile.close();
    return true;
}

void ut_DTextEncoding::removeTempFile()
{
    if (QFile::exists(tmpFileName)) {
        QFile::remove(tmpFileName);
    }
}

void ut_DTextEncoding::TearDown()
{
    removeTempFile();
}

TEST_F(ut_DTextEncoding, testDetectTextEncode)
{
    // Default encoding is utf-8.
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding(""));
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding("12345678ABCDEFG"));

    ASSERT_EQ("GB18030", DTextEncoding::detectTextEncoding(dataGB18030));
    ASSERT_EQ("EUC-JP", DTextEncoding::detectTextEncoding(dataEUC_JP));
    ASSERT_EQ("KOI8-R", DTextEncoding::detectTextEncoding(dataKOI8_R));
}

TEST_F(ut_DTextEncoding, testDetectTextEncodeWithUchardet)
{
    if (canLoadUchardet) {
        QByteArray uchardetEncoding("EUC-TW");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto encode = QStringConverter::encodingForName(uchardetEncoding);
        ASSERT_FALSE(encode);
#else
        // QTextCodec not suppotted EUC-TW.
        QTextCodec *codec = QTextCodec::codecForName(uchardetEncoding);
        ASSERT_EQ(codec, nullptr);
#endif

        // Utf8 text: 繁體中文測試一二三四
        QByteArray dataZhTraditional("\u7e41\u9ad4\u4e2d\u6587\u6e2c\u8a66\u4e00\u4e8c\u4e09\u56db");
        QByteArray dataEUC_TW;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataZhTraditional, dataEUC_TW, uchardetEncoding));
        ASSERT_EQ(uchardetEncoding, DTextEncoding::detectTextEncoding(dataEUC_TW));

        QByteArray convertGB18030;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataEUC_TW, convertGB18030, "GB18030"));
        QByteArray convertUTF8;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(convertGB18030, convertUTF8, "UTF-8"));
        ASSERT_EQ(dataZhTraditional, convertUTF8);
    }
}

TEST_F(ut_DTextEncoding, testDetectFileEncode)
{
    bool isOk = false;
    ASSERT_EQ("", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_FALSE(isOk);

    ASSERT_TRUE(rewriteTempFile(""));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_EQ("GB18030", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataEUC_JP));
    ASSERT_EQ("EUC-JP", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataKOI8_R));
    ASSERT_EQ("KOI8-R", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);
}

TEST_F(ut_DTextEncoding, testConvertTextEncoding)
{
    QByteArray dataUTF_8;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "UTF-8"));
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding(dataUTF_8));

    // QStringConverter not support GB18030.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    ASSERT_EQ(codec->toUnicode(dataGB18030).toUtf8(), dataUTF_8);

    QByteArray convertedGB18030;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, convertedGB18030, "GB18030"));
    ASSERT_EQ(dataGB18030, convertedGB18030);
#endif

    // Convert with multi bytes encoding.
    QByteArray dataUTF_16;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, dataUTF_16, "UTF-16"));
    ASSERT_EQ("UTF-16", DTextEncoding::detectTextEncoding(dataUTF_16));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 using utf-16 string by default, not utf-8.
    auto fromUtf16 = QStringDecoder(QStringDecoder::Utf16);
    QString strUtf16 = fromUtf16.decode(dataUTF_16);
    ASSERT_EQ(strUtf16, QString::fromUtf8(dataUTF_8));
#else
    codec = QTextCodec::codecForName("UTF-16");
    ASSERT_EQ(codec->toUnicode(dataUTF_16).toUtf8(), dataUTF_8);
#endif

    QByteArray convertedUTF8;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_16, convertedUTF8, "UTF-8"));
    ASSERT_EQ(dataUTF_8, convertedUTF8);

    QByteArray dataUTF_32;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, dataUTF_32, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectTextEncoding(dataUTF_32));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto fromUtf32 = QStringDecoder(QStringDecoder::Utf32);
    QString strUtf32 = fromUtf32.decode(dataUTF_32);
    ASSERT_EQ(strUtf32, QString::fromUtf8(dataUTF_8));
#else
    codec = QTextCodec::codecForName("UTF-32");
    ASSERT_EQ(codec->toUnicode(dataUTF_32).toUtf8(), dataUTF_8);
#endif

    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_32, convertedUTF8, "UTF-8"));
    ASSERT_EQ(dataUTF_8, convertedUTF8);
}

TEST_F(ut_DTextEncoding, testConvertTextEncodingWithError)
{
    QByteArray dataUTF_8;
    ASSERT_FALSE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "ERROR"));
    ASSERT_FALSE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "KOI8-R"));

    QByteArray tmpUTF_8Error = "\x31\x32\x33\xFF\xFF\xFF\x31\x32\x33";
    QByteArray tmpUTF_16;
    QString error;
    int converted = 0;
    bool ret = DTextEncoding::convertTextEncodingEx(tmpUTF_8Error, tmpUTF_16, "UTF-16", "UTF-8", &error, &converted);
    ASSERT_FALSE(ret);
    ASSERT_FALSE(error.isEmpty());
    ASSERT_EQ(converted, 3);

    QByteArray tmpUTF_8Error2 = "\xFF\xFF";
    converted = 3;
    ret = DTextEncoding::convertTextEncodingEx(tmpUTF_8Error2, tmpUTF_16, "UTF-16", "UTF-8", &error, &converted);
    ASSERT_FALSE(ret);
    ASSERT_EQ(converted, 0);
}

TEST_F(ut_DTextEncoding, testConvertFileEncoding)
{
    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_TRUE(DTextEncoding::convertFileEncoding(tmpFileName, "UTF-8"));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncoding(tmpFileName, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectFileEncoding(tmpFileName));

    ASSERT_FALSE(DTextEncoding::convertFileEncoding("", "UTF-32"));
}

TEST_F(ut_DTextEncoding, testConvertFileEncodingTo)
{
    QString tmpConvertFileName("/tmp/ut_DTextEncoding_temp_testConvertFileEncodingTo.txt");
    if (QFile::exists(tmpConvertFileName)) {
        ASSERT_TRUE(QFile::remove(tmpConvertFileName));
    }

    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "GB18030"));
    ASSERT_TRUE(QFile::exists(tmpConvertFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "UTF-8"));
    ASSERT_TRUE(QFile::exists(tmpConvertFileName));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpConvertFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectFileEncoding(tmpConvertFileName));

    ASSERT_TRUE(QFile::remove(tmpConvertFileName));
}

TEST_F(ut_DTextEncoding, testConvertFileEncodingToWithError)
{
    QString tmpConvertFileName("/tmp/ut_DTextEncoding_temp_testConvertFileEncodingToWithError.txt");
    if (QFile::exists(tmpConvertFileName)) {
        ASSERT_TRUE(QFile::remove(tmpConvertFileName));
    }

    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo("", tmpConvertFileName, "UTF-32"));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, "", "UTF-32"));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "ERROR"));
    ASSERT_FALSE(QFile::exists(tmpConvertFileName));

    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "EUC-JP"));
    ASSERT_FALSE(QFile::exists(tmpConvertFileName));
}
