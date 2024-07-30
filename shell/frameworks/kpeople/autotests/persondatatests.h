/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONDATATESTS_H
#define PERSONDATATESTS_H

#include <QObject>
#include <QTemporaryFile>

class FakeContactSource;

class PersonDataTests : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void loadContact();
    void loadPerson();
    void contactChanged();

    void nullPerson();
    void removeContact();

private:
    FakeContactSource *m_source;
    QTemporaryFile m_database;
};

#endif // PERSONDATATESTS_H
