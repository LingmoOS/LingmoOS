/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testpersonimpl.h"
#include "person.h"
#include "personimpl.h"
#include "tools.h"
#include <QList>
#include <QString>
#include <QStringList>

using Syndication::Person;
using Syndication::PersonImpl;
using Syndication::PersonPtr;

void TestPersonImpl::fromString()
{
    QStringList s;
    QList<PersonPtr> p;

    s.append(QString());
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));

    s.append(QLatin1String(""));
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));

    s.append(QStringLiteral("foo@bar.com"));
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QStringLiteral("foo@bar.com"))));

    s.append(QStringLiteral("<foo@bar.com>"));
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QStringLiteral("foo@bar.com"))));

    s.append(QStringLiteral("Foo"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("Foo"), QString(), QString())));

    s.append(QStringLiteral("Foo M. Bar"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("Foo M. Bar"), QString(), QString())));

    s.append(QStringLiteral("Foo <foo@bar.com>"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("Foo"), QString(), QStringLiteral("foo@bar.com"))));

    s.append(QStringLiteral("Foo Bar <foo@bar.com>"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("Foo Bar"), QString(), QStringLiteral("foo@bar.com"))));

    s.append(QStringLiteral("John Doe (President) <john@doe.com>"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("John Doe (President)"), QString(), QStringLiteral("john@doe.com"))));

    s.append(QStringLiteral("John Doe (President)"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("John Doe (President)"), QString(), QString())));

    s.append(QStringLiteral("John Doe (President)) <john@doe.com>"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("John Doe (President))"), QString(), QStringLiteral("john@doe.com"))));

    s.append(QStringLiteral("(President) John Doe <john@doe.com>"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("(President) John Doe"), QString(), QStringLiteral("john@doe.com"))));

    s.append(QStringLiteral("<foo@bar.com> (Foo Bar)"));
    p.append(PersonPtr(new PersonImpl(QStringLiteral("Foo Bar"), QString(), QStringLiteral("foo@bar.com"))));

    // s.append("OnAhlmann(mailto:&amp;#111;&amp;#110;&amp;#97;&amp;#104;&amp;#108;&amp;#109;&amp;#97;&amp;#110;&amp;#110;&amp;#64;&amp;#103;&amp;#109;&amp;#97;&amp;#105;&amp;#108;&amp;#46;&amp;#99;&amp;#111;&amp;#109;)");
    // p.append(PersonPtr(new PersonImpl("OnAhlmann", QString(), "onahlmann@gmail.com")));

    QList<PersonPtr> q;

    QStringList::ConstIterator it = s.constBegin();
    QStringList::ConstIterator end = s.constEnd();
    QList<PersonPtr>::ConstIterator pit = p.constBegin();

    while (it != end) {
        PersonPtr q(Syndication::personFromString(*it));
        QCOMPARE(q->name(), (*pit)->name());
        QCOMPARE(q->email(), (*pit)->email());
        QCOMPARE(q->uri(), (*pit)->uri());

        ++it;
        ++pit;
    }
}

QTEST_MAIN(TestPersonImpl)

#include "moc_testpersonimpl.cpp"
