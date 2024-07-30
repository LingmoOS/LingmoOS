/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbihash_p.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QString>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KBiHash<int, QString> biHash;
    biHash.insert(5, "5.5");
    biHash.insert(6, "6.6");
    biHash.insert(7, "7.7");
    biHash.insert(8, "8.8");

    qDebug() << "left to right";
    KBiHash<int, QString>::left_iterator it1 = biHash.leftBegin();
    while (it1 != biHash.leftEnd()) {
        qDebug() << it1.key() << it1.value();
        if (it1.key() == 7) {
            qDebug() << "erase" << it1.key() << ":" << it1.value();
            it1 = biHash.eraseLeft(it1);
        } else {
            ++it1;
        }
    }
    qDebug() << "right to left";
    KBiHash<int, QString>::right_const_iterator it2 = biHash.rightConstBegin();
    const KBiHash<int, QString>::right_const_iterator end2 = biHash.rightConstEnd();
    while (it2 != end2) {
        qDebug() << it2.key() << it2.value();
        ++it2;
    }

    KBiHash<int, QString>::right_iterator it3 = biHash.rightBegin();
    while (it3 != biHash.rightEnd()) {
        if (it3.value() == 5) {
            const int newValue = 55;
            qDebug() << "update" << it3.key() << ":" << it3.value() << "to" << newValue;
            biHash.updateLeft(it3, newValue);
        }
        ++it3;
    }

    KBiHash<int, QString>::left_iterator it4 = biHash.leftBegin();
    while (it4 != biHash.leftEnd()) {
        if (it4.value() == "6.6") {
            const QLatin1String newValue("66.66");
            qDebug() << "update" << it4.key() << ":" << it4.value() << "to" << newValue;
            biHash.updateRight(it4, newValue);
        }
        ++it4;
    }

    KBiHash<int, QString>::right_const_iterator it5 = biHash.constFindRight("5.5");
    qDebug() << "found" << it5.key() << it5.value();

    KBiHash<int, QString>::left_const_iterator it6 = biHash.constFindLeft(6);
    qDebug() << "found" << it6.key() << it6.value();

    qDebug() << "subscript operator" << biHash[8];

    qDebug() << "left to right";
    KBiHash<int, QString>::left_iterator it7 = biHash.leftBegin();
    while (it7 != biHash.leftEnd()) {
        qDebug() << it7.key() << it7.value();
        if (it7.key() == 7) {
            it7 = biHash.eraseLeft(it7);
        } else {
            ++it7;
        }
    }

    KBiHash<int, QString> biHash2;
    biHash2.insert(8, "8.8");
    biHash2.insert(9, "9.9");
    biHash2.insert(10, "10.10");
    biHash2.insert(11, "11.11");

    qDebug() << biHash2;
    qDebug() << biHash;

    KBiHash<int, QString> biHash3 = biHash.unite(biHash2);

    qDebug() << biHash;
    qDebug() << biHash3;

    QByteArray ba;
    QBuffer outBuffer(&ba);
    outBuffer.open(QIODevice::WriteOnly);
    QDataStream out(&outBuffer);
    out << biHash;
    outBuffer.close();

    KBiHash<int, QString> biHash4;

    QBuffer inBuffer(&ba);
    inBuffer.open(QIODevice::ReadOnly);
    QDataStream in(&inBuffer);

    in >> biHash4;

    qDebug() << biHash4;

    qDebug() << (biHash == biHash4) << (biHash != biHash4);

    QHash<int, QString> hash;
    hash.insert(1, "1");
    hash.insert(2, "2");
    hash.insert(3, "3");

    KBiHash<int, QString> biHash5 = KBiHash<int, QString>::fromHash(hash);

    qDebug() << biHash5;

    KBiHash<int, QString>::left_iterator it8 = biHash5.findLeft(1);
    qDebug() << (it8 == biHash.leftEnd());
    qDebug() << it8.key();
    //   qDebug() << it8.key() << it8.value();

    return 0;
}
