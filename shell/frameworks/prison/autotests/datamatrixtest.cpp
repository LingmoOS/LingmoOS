/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <Prison/Barcode>

#include <QObject>
#include <QTest>

using namespace Prison;

class DataMatrixTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRender_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QString>("refName");

        QTest::newRow("text") << QByteArray("KF5::Prison") << "datamatrix-text.png";
        QTest::newRow("binary") << QByteArray("KDE\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9kde", 16) << "datamatrix-binary.png";
    }

    void testRender()
    {
        QFETCH(QByteArray, input);
        QFETCH(QString, refName);

        {
            auto code = Prison::Barcode::create(Prison::DataMatrix);
            QVERIFY(code);
            code->setData(QString::fromLatin1(input.constData(), input.size()));
            const auto img = code->toImage(code->preferredSize(1));
            img.save(refName);

            QImage ref(QStringLiteral(":/datamatrix/") + refName);
            ref = ref.convertToFormat(img.format());
            QCOMPARE(img, ref);
        }
        {
            auto code = Prison::Barcode::create(Prison::DataMatrix);
            QVERIFY(code);
            code->setData(input);
            const auto img = code->toImage(code->preferredSize(1));
            img.save(refName);

            QImage ref(QStringLiteral(":/datamatrix/") + refName);
            ref = ref.convertToFormat(img.format());
            QCOMPARE(img, ref);
        }
    }
};

QTEST_APPLESS_MAIN(DataMatrixTest)

#include "datamatrixtest.moc"
