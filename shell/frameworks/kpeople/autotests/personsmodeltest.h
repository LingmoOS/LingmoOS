/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONSMODELTEST_H
#define PERSONSMODELTEST_H

#include <QObject>
#include <QTemporaryFile>

class FakeContactSource;
namespace KPeople
{
class PersonsModel;
}

class PersonsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void loadModel();
    void mergeContacts();
    void gettersTests();
    void unmergeContacts();

private:
    FakeContactSource *m_source;
    KPeople::PersonsModel *m_model;
    QTemporaryFile m_database;
};

#endif // PERSONSMODELTEST_H
