/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALTERNATIVESMODELTEST_H
#define ALTERNATIVESMODELTEST_H

#include <QObject>
#include <QTemporaryDir>

class AlternativesModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void runJobTest();
    void bigBufferTest();
    void disablePluginTest();
    void blacklistTest();

private:
    QTemporaryDir m_tempDir;
};

#endif
