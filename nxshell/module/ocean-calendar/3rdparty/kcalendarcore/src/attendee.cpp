/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Attendee class.

  @brief
  Represents information related to an attendee of an Calendar Incidence.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "attendee.h"
#include "person.h"
#include "person_p.h"

#include <QDataStream>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Attendee::Private : public QSharedData
{
public:
    void setCuType(CuType cuType);
    void setCuType(const QString &cuType);
    CuType cuType() const;
    QString cuTypeStr() const;

    bool mRSVP = false;
    Role mRole;
    PartStat mStatus;
    mutable QString mUid;
    QString mDelegate;
    QString mDelegator;
    CustomProperties mCustomProperties;
    QString mName;
    QString mEmail;

private:
    QString sCuType;
    CuType mCuType;
};
//@endcond

void KCalendarCore::Attendee::Private::setCuType(Attendee::CuType cuType)
{
    mCuType = cuType;
    sCuType.clear();
}

void KCalendarCore::Attendee::Private::setCuType(const QString &cuType)
{
    const QString upper = cuType.toUpper();
    if (upper == QLatin1String("INDIVIDUAL")) {
        setCuType(Attendee::Individual);
    } else if (upper == QLatin1String("GROUP")) {
        setCuType(Attendee::Group);
    } else if (upper == QLatin1String("RESOURCE")) {
        setCuType(Attendee::Resource);
    } else if (upper == QLatin1String("ROOM")) {
        setCuType(Attendee::Room);
    } else {
        setCuType(Attendee::Unknown);
        if (upper.startsWith(QLatin1String("X-")) || upper.startsWith(QLatin1String("IANA-"))) {
            sCuType = upper;
        }
    }
}

Attendee::CuType KCalendarCore::Attendee::Private::cuType() const
{
    return mCuType;
}

QString KCalendarCore::Attendee::Private::cuTypeStr() const
{
    switch (mCuType) {
    case Attendee::Individual:
        return QStringLiteral("INDIVIDUAL");
    case Attendee::Group:
        return QStringLiteral("GROUP");
    case Attendee::Resource:
        return QStringLiteral("RESOURCE");
    case Attendee::Room:
        return QStringLiteral("ROOM");
    case Attendee::Unknown:
        if (sCuType.isEmpty()) {
            return QStringLiteral("UNKNOWN");
        } else {
            return sCuType;
        }
    }
    return QStringLiteral("UNKNOWN");
}

Attendee::Attendee()
    : d(new Attendee::Private)
{
}

Attendee::Attendee(const QString &name, const QString &email, bool rsvp, Attendee::PartStat status, Attendee::Role role, const QString &uid)
    : d(new Attendee::Private)
{
    setName(name);
    setEmail(email);
    d->mRSVP = rsvp;
    d->mStatus = status;
    d->mRole = role;
    d->mUid = uid;
    d->setCuType(Attendee::Individual);
}

Attendee::Attendee(const Attendee &attendee)
    : d(attendee.d)
{
}

Attendee::~Attendee() = default;

bool Attendee::isNull() const
{
    // isNull rather than isEmpty, as user code is actually creating empty but non-null attendees...
    return d->mName.isNull() && d->mEmail.isNull();
}

bool KCalendarCore::Attendee::operator==(const Attendee &attendee) const
{
    return d->mUid == attendee.d->mUid && d->mRSVP == attendee.d->mRSVP && d->mRole == attendee.d->mRole && d->mStatus == attendee.d->mStatus
           && d->mDelegate == attendee.d->mDelegate && d->mDelegator == attendee.d->mDelegator && d->cuTypeStr() == attendee.d->cuTypeStr()
           && d->mName == attendee.d->mName && d->mEmail == attendee.d->mEmail;
}

bool KCalendarCore::Attendee::operator!=(const Attendee &attendee) const
{
    return !operator==(attendee);
}

Attendee &KCalendarCore::Attendee::operator=(const Attendee &attendee)
{
    // check for self assignment
    if (&attendee == this) {
        return *this;
    }

    d = attendee.d;
    return *this;
}

QString Attendee::name() const
{
    return d->mName;
}

void Attendee::setName(const QString &name)
{
    if (name.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        d->mName = name.mid(7);
    } else {
        d->mName = name;
    }
}

QString Attendee::fullName() const
{
    return fullNameHelper(d->mName, d->mEmail);
}

QString Attendee::email() const
{
    return d->mEmail;
}

void Attendee::setEmail(const QString &email)
{
    if (email.startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
        d->mEmail = email.mid(7);
    } else {
        d->mEmail = email;
    }
}

void Attendee::setRSVP(bool r)
{
    d->mRSVP = r;
}

bool Attendee::RSVP() const
{
    return d->mRSVP;
}

void Attendee::setStatus(Attendee::PartStat status)
{
    d->mStatus = status;
}

Attendee::PartStat Attendee::status() const
{
    return d->mStatus;
}

void Attendee::setCuType(Attendee::CuType cuType)
{
    d->setCuType(cuType);
}

void Attendee::setCuType(const QString &cuType)
{
    d->setCuType(cuType);
}

Attendee::CuType Attendee::cuType() const
{
    return d->cuType();
}

QString Attendee::cuTypeStr() const
{
    return d->cuTypeStr();
}

void Attendee::setRole(Attendee::Role role)
{
    d->mRole = role;
}

Attendee::Role Attendee::role() const
{
    return d->mRole;
}

void Attendee::setUid(const QString &uid)
{
    d->mUid = uid;
}

QString Attendee::uid() const
{
    /* If Uid is empty, just use the pointer to Attendee (encoded to
     * string) as Uid. Only thing that matters is that the Uid is unique
     * insofar IncidenceBase is concerned, and this does that (albeit
     * not very nicely). If these are ever saved to disk, should use
     * (considerably more expensive) CalFormat::createUniqueId(). As Uid
     * is not part of Attendee in iCal std, it's fairly safe bet that
     * these will never hit disc though so faster generation speed is
     * more important than actually being forever unique.*/
    if (d->mUid.isEmpty()) {
        d->mUid = QString::number((qlonglong)d.constData());
    }

    return d->mUid;
}

void Attendee::setDelegate(const QString &delegate)
{
    d->mDelegate = delegate;
}

QString Attendee::delegate() const
{
    return d->mDelegate;
}

void Attendee::setDelegator(const QString &delegator)
{
    d->mDelegator = delegator;
}

QString Attendee::delegator() const
{
    return d->mDelegator;
}

void Attendee::setCustomProperty(const QByteArray &xname, const QString &xvalue)
{
    d->mCustomProperties.setNonKDECustomProperty(xname, xvalue);
}

CustomProperties &Attendee::customProperties()
{
    return d->mCustomProperties;
}

const CustomProperties &Attendee::customProperties() const
{
    return d->mCustomProperties;
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::Attendee &attendee)
{
    KCalendarCore::Person p(attendee.name(), attendee.email());
    stream << p;
    return stream << attendee.d->mRSVP << int(attendee.d->mRole) << int(attendee.d->mStatus) << attendee.d->mUid << attendee.d->mDelegate
                  << attendee.d->mDelegator << attendee.d->cuTypeStr() << attendee.d->mCustomProperties;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, KCalendarCore::Attendee &attendee)
{
    bool RSVP;
    Attendee::Role role;
    Attendee::PartStat status;
    QString uid;
    QString delegate;
    QString delegator;
    QString cuType;
    CustomProperties customProperties;
    uint role_int;
    uint status_int;

    KCalendarCore::Person person;
    stream >> person;
    stream >> RSVP >> role_int >> status_int >> uid >> delegate >> delegator >> cuType >> customProperties;

    role = Attendee::Role(role_int);
    status = Attendee::PartStat(status_int);

    attendee = Attendee(person.name(), person.email(), RSVP, status, role, uid);
    attendee.setDelegate(delegate);
    attendee.setDelegator(delegator);
    attendee.setCuType(cuType);
    attendee.d->mCustomProperties = customProperties;
    return stream;
}
