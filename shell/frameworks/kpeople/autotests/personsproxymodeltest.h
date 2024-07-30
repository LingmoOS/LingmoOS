/*
    SPDX-FileCopyrightText: 2015 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONSPROXYMODELTEST_H
#define PERSONSPROXYMODELTEST_H

#include <QObject>
#include <QTemporaryFile>

class FakeContactSource;
namespace KPeople
{
class PersonsModel;
}

class PersonsProxyModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testFiltering();

private:
    FakeContactSource *m_source;
    KPeople::PersonsModel *m_model;
    QTemporaryFile m_database;
};

#endif // PERSONSMODELTEST_H
