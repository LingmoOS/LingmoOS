/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include <KCodecs>

class Base45Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBase45Decode_data()
    {
        QTest::addColumn<QByteArray>("in");
        QTest::addColumn<QByteArray>("out");

        QTest::newRow("empty") << QByteArray() << QByteArray();

        // examples from the RFC - https://datatracker.ietf.org/doc/draft-faltstrom-base45/
        QTest::newRow("hello") << QByteArray("%69 VD92EX0") << QByteArray("Hello!!");
        QTest::newRow("base-45") << QByteArray("UJCLQE7W581") << QByteArray("base-45");
        QTest::newRow("ietf") << QByteArray("QED8WEX0") << QByteArray("ietf!");

        // from EU DCG test data - https://github.com/eu-digital-green-certificates/dgc-testdata
        QTest::newRow("eu-dcg")
            << QByteArray(
                   "6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 "
                   "N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%67092L4WJCT3EHS8XJC$+"
                   "DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA "
                   "96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D "
                   "JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 C5IA5N9$PC5$CUZCY$5Y$527B+A4KZNQG5TKOWWD9FL%I8U$F7O2IBM85CWOC%LEZU4R/BXHDAHN "
                   "11$CA5MRI:AONFN7091K9FKIGIY%VWSSSU9%01FO2*FTPQ3C3F")
            << QByteArray::fromHex(
                   "789c0163019cfed28443a10126a104480c4b15512be9140159010da401624445061a60b29429041a61f39fa9390103a101a4617681aa626369782f55524e3a555643493a303"
                   "144452f495a3132333435412f3543574c553132524e4f4239525853454f5036464738235762636f62444562646e026264746a323032312d30352d323962697374526f626572"
                   "74204b6f63682d496e737469747574626d616d4f52472d313030303331313834626d706c45552f312f32302f3135303762736402627467693834303533393030366276706a3"
                   "131313933343930303763646f626a313936342d30382d3132636e616da462666e6a4d75737465726d616e6e62676e654572696b6163666e746a4d55535445524d414e4e6367"
                   "6e74654552494b416376657265312e302e305840218ebc2a2a77c1796c95a8c942987d461411b0075fd563447295250d5ead69f3b8f6083a515bd97656e87aca01529e6aa0e"
                   "09144fc07e2884c93080f1419e82f1c66773a");
    }

    void testBase45Decode()
    {
        QFETCH(QByteArray, in);
        QFETCH(QByteArray, out);

        QCOMPARE(KCodecs::base45Decode(in), out);
    }

    void testBase45DecodeInvalid_data()
    {
        QTest::addColumn<QByteArray>("in");
        QTest::newRow("1 byte") << QByteArray("X");
        QTest::newRow("invalid chars") << QByteArray("%69 vD92Ex0");
    }

    void testBase45DecodeInvalid()
    {
        QFETCH(QByteArray, in);

        // undefined return value, but must not crash or produce ASAN errors
        KCodecs::base45Decode(in);
    }
};

QTEST_APPLESS_MAIN(Base45Test)

#include "base45test.moc"
