/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Person class.

  @brief
  Represents a person, by name and email address.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "person.h"
#include "person_p.h"

#include <QDataStream>
#include <QRegularExpression>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Person::Private : public QSharedData
{
public:
    QString mName; // person name
    QString mEmail; // person email address
};
//@endcond

Person::Person()
    : d(new KCalendarCore::Person::Private)
{
}

Person::Person(const QString &name, const QString &email)
    : d(new KCalendarCore::Person::Private)
{
    d->mName = name;
    d->mEmail = email;
}

Person::Person(const Person &person)
    : d(person.d)
{
}

Person::~Person() = default;

bool KCalendarCore::Person::operator==(const Person &person) const
{
    return d->mName == person.d->mName && d->mEmail == person.d->mEmail;
}

bool KCalendarCore::Person::operator!=(const Person &person) const
{
    return !(*this == person);
}

Person &KCalendarCore::Person::operator=(const Person &person)
{
    // check for self assignment
    if (&person == this) {
        return *this;
    }

    d = person.d;
    return *this;
}

QString KCalendarCore::fullNameHelper(const QString &name, const QString &email)
{
    if (name.isEmpty()) {
        return email;
    }
    if (email.isEmpty()) {
        return name;
    }
    // Taken from KContacts::Addressee::fullEmail
    QString fullName = name;
    const QRegularExpression needQuotes(QStringLiteral("[^ 0-9A-Za-z\\x{0080}-\\x{FFFF}]"));
    bool weNeedToQuote = name.indexOf(needQuotes) != -1;
    if (weNeedToQuote) {
        if (fullName[0] != QLatin1Char('"')) {
            fullName.prepend(QLatin1Char('"'));
        }
        if (fullName[fullName.length() - 1] != QLatin1Char('"')) {
            fullName.append(QLatin1Char('"'));
        }
    }
    return fullName + QStringLiteral(" <") + email + QLatin1Char('>');
}

QString Person::fullName() const
{
    return fullNameHelper(d->mName, d->mEmail);
}

QString Person::name() const
{
    return d->mName;
}

QString Person::email() const
{
    return d->mEmail;
}

bool Person::isEmpty() const
{
    return d->mEmail.isEmpty() && d->mName.isEmpty();
}

void Person::setName(const QString &name)
{
    d->mName = name;
}

void Person::setEmail(const QString &email)
{
    if (email.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        d->mEmail = email.mid(7);
    } else {
        d->mEmail = email;
    }
}

bool Person::isValidEmail(const QString &email)
{
    const int pos = email.lastIndexOf(QLatin1Char('@'));
    return (pos > 0) && (email.lastIndexOf(QLatin1Char('.')) > pos) && ((email.length() - pos) > 4);
}

uint KCalendarCore::qHash(const KCalendarCore::Person &key)
{
    return qHash(key.fullName());
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::Person &person)
{
    return stream << person.d->mName << person.d->mEmail << (int)(0);
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, Person &person)
{
    int count;
    stream >> person.d->mName >> person.d->mEmail >> count;
    return stream;
}

// The following function was lifted directly from KPIMUtils
// in order to eliminate the dependency on that library.
// Any changes made here should be ported there, and vice versa.
static bool extractEmailAddressAndName(const QString &aStr, QString &mail, QString &name)
{
    name.clear();
    mail.clear();

    const int len = aStr.length();
    const char cQuotes = '"';

    bool bInComment = false;
    bool bInQuotesOutsideOfEmail = false;
    int i = 0, iAd = 0, iMailStart = 0, iMailEnd = 0;
    QChar c;
    unsigned int commentstack = 0;

    // Find the '@' of the email address
    // skipping all '@' inside "(...)" comments:
    while (i < len) {
        c = aStr[i];
        if (QLatin1Char('(') == c) {
            commentstack++;
        }
        if (QLatin1Char(')') == c) {
            commentstack--;
        }
        bInComment = commentstack != 0;
        if (QLatin1Char('"') == c && !bInComment) {
            bInQuotesOutsideOfEmail = !bInQuotesOutsideOfEmail;
        }

        if (!bInComment && !bInQuotesOutsideOfEmail) {
            if (QLatin1Char('@') == c) {
                iAd = i;
                break; // found it
            }
        }
        ++i;
    }

    if (!iAd) {
        // We suppose the user is typing the string manually and just
        // has not finished typing the mail address part.
        // So we take everything that's left of the '<' as name and the rest as mail
        for (i = 0; len > i; ++i) {
            c = aStr[i];
            if (QLatin1Char('<') != c) {
                name.append(c);
            } else {
                break;
            }
        }
        mail = aStr.mid(i + 1);
        if (mail.endsWith(QLatin1Char('>'))) {
            mail.chop(1);
        }

    } else {
        // Loop backwards until we find the start of the string
        // or a ',' that is outside of a comment
        //          and outside of quoted text before the leading '<'.
        bInComment = false;
        bInQuotesOutsideOfEmail = false;
        for (i = iAd - 1; 0 <= i; --i) {
            c = aStr[i];
            if (bInComment) {
                if (QLatin1Char('(') == c) {
                    if (!name.isEmpty()) {
                        name.prepend(QLatin1Char(' '));
                    }
                    bInComment = false;
                } else {
                    name.prepend(c); // all comment stuff is part of the name
                }
            } else if (bInQuotesOutsideOfEmail) {
                if (QLatin1Char(cQuotes) == c) {
                    bInQuotesOutsideOfEmail = false;
                } else if (c != QLatin1Char('\\')) {
                    name.prepend(c);
                }
            } else {
                // found the start of this addressee ?
                if (QLatin1Char(',') == c) {
                    break;
                }
                // stuff is before the leading '<' ?
                if (iMailStart) {
                    if (QLatin1Char(cQuotes) == c) {
                        bInQuotesOutsideOfEmail = true; // end of quoted text found
                    } else {
                        name.prepend(c);
                    }
                } else {
                    switch (c.toLatin1()) {
                    case '<':
                        iMailStart = i;
                        break;
                    case ')':
                        if (!name.isEmpty()) {
                            name.prepend(QLatin1Char(' '));
                        }
                        bInComment = true;
                        break;
                    default:
                        if (QLatin1Char(' ') != c) {
                            mail.prepend(c);
                        }
                    }
                }
            }
        }

        name = name.simplified();
        mail = mail.simplified();

        if (mail.isEmpty()) {
            return false;
        }

        mail.append(QLatin1Char('@'));

        // Loop forward until we find the end of the string
        // or a ',' that is outside of a comment
        //          and outside of quoted text behind the trailing '>'.
        bInComment = false;
        bInQuotesOutsideOfEmail = false;
        int parenthesesNesting = 0;
        for (i = iAd + 1; len > i; ++i) {
            c = aStr[i];
            if (bInComment) {
                if (QLatin1Char(')') == c) {
                    if (--parenthesesNesting == 0) {
                        bInComment = false;
                        if (!name.isEmpty()) {
                            name.append(QLatin1Char(' '));
                        }
                    } else {
                        // nested ")", add it
                        name.append(QLatin1Char(')')); // name can't be empty here
                    }
                } else {
                    if (QLatin1Char('(') == c) {
                        // nested "("
                        ++parenthesesNesting;
                    }
                    name.append(c); // all comment stuff is part of the name
                }
            } else if (bInQuotesOutsideOfEmail) {
                if (QLatin1Char(cQuotes) == c) {
                    bInQuotesOutsideOfEmail = false;
                } else if (c != QLatin1Char('\\')) {
                    name.append(c);
                }
            } else {
                // found the end of this addressee ?
                if (QLatin1Char(',') == c) {
                    break;
                }
                // stuff is behind the trailing '>' ?
                if (iMailEnd) {
                    if (QLatin1Char(cQuotes) == c) {
                        bInQuotesOutsideOfEmail = true; // start of quoted text found
                    } else {
                        name.append(c);
                    }
                } else {
                    switch (c.toLatin1()) {
                    case '>':
                        iMailEnd = i;
                        break;
                    case '(':
                        if (!name.isEmpty()) {
                            name.append(QLatin1Char(' '));
                        }
                        if (++parenthesesNesting > 0) {
                            bInComment = true;
                        }
                        break;
                    default:
                        if (QLatin1Char(' ') != c) {
                            mail.append(c);
                        }
                    }
                }
            }
        }
    }

    name = name.simplified();
    mail = mail.simplified();

    return !(name.isEmpty() || mail.isEmpty());
}

Person Person::fromFullName(const QString &fullName)
{
    QString email, name;
    extractEmailAddressAndName(fullName, email, name);
    return Person(name, email);
}
