/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2016 Arne Spiegelhauer <gm2.asp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <krandom.h>
#include <stdlib.h>

#include <QTest>
#include <QThread>

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QVarLengthArray>

#include <algorithm>
#include <iostream>

typedef QVarLengthArray<int> intSequenceType;

static const char *binpath;

class KRandomTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_randomString();
    void test_randomStringThreaded();
    void test_shuffle();
};

void KRandomTest::test_randomString()
{
    const int desiredLength = 12;
    const QString testString = KRandom::randomString(desiredLength);
    const QRegularExpression outputFormat(QRegularExpression::anchoredPattern(QStringLiteral("[A-Za-z0-9]+")));
    const QRegularExpressionMatch match = outputFormat.match(testString);

    QCOMPARE(testString.length(), desiredLength);
    QVERIFY(match.hasMatch());
}

void KRandomTest::test_shuffle()
{
    {
        QRandomGenerator rg(1);
        QList<int> list = {1, 2, 3, 4, 5};
        const QList<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(list, &rg);
        QCOMPARE(list, shuffled);
    }

    {
        QRandomGenerator rg(1);
        QList<int> vector = {1, 2, 3, 4, 5};
        const QList<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(vector, &rg);
        QCOMPARE(vector, shuffled);
    }

    {
        QRandomGenerator rg(1);
        std::vector<int> std_vector = {1, 2, 3, 4, 5};
        const std::vector<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(std_vector, &rg);
        QCOMPARE(std_vector, shuffled);
    }
}

class KRandomTestThread : public QThread
{
protected:
    void run() override
    {
        result = KRandom::randomString(32);
    };

public:
    QString result;
};

void KRandomTest::test_randomStringThreaded()
{
    static const int size = 5;
    KRandomTestThread *threads[size];
    for (int i = 0; i < size; ++i) {
        threads[i] = new KRandomTestThread();
        threads[i]->start();
    }
    QSet<QString> results;
    for (int i = 0; i < size; ++i) {
        threads[i]->wait(2000);
        results.insert(threads[i]->result);
    }
    // each thread should have returned a unique result
    QCOMPARE(results.size(), size);
    for (int i = 0; i < size; ++i) {
        delete threads[i];
    }
}

// Manually implemented to dispatch to child process if needed to support
// subtests
int main([[maybe_unused]] int argc, char *argv[])
{
    binpath = argv[0];
    KRandomTest randomTest;
    return QTest::qExec(&randomTest);
}

#include "krandomtest.moc"
