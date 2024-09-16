// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TESTDISOMASTER_H
#define TESTDISOMASTER_H

#include <QObject>
#include <QtTest/QtTest>
#include "../libdisomaster/disomaster.h"

class TestDISOMaster;

class TestSignalReceiver: public QObject
{
    Q_OBJECT
public:
    explicit TestSignalReceiver(DISOMasterNS::DISOMaster *_d, TestDISOMaster *parent = nullptr);
public Q_SLOTS:
    void updateJobStatus(DISOMasterNS::DISOMaster::JobStatus status, int progress);
private:
    TestDISOMaster *p;
    DISOMasterNS::DISOMaster *d;
};

class TestDISOMaster : public QObject
{
    Q_OBJECT
public:
    explicit TestDISOMaster(QObject *parent = nullptr);
    DISOMasterNS::DISOMaster::JobStatus st;
    int p;

private Q_SLOTS:
    void test_getDevice();
    void test_writeFiles();
    void test_erase();
    void test_isoWrite();
    void test_checkMedia();
    void test_dumpISO();

};

#endif // TESTDISOMASTER_H
