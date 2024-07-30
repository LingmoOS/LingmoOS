/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATIVESMODELTEST_H
#define ALTERNATIVESMODELTEST_H

#include <QObject>
#include <QTemporaryDir>

class MenuTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void runJobTest();

private:
    QTemporaryDir m_tempDir;
};

#endif
