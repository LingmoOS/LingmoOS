/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Journal class.

  @brief
  Provides a Journal in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "incidence_p.h"
#include "journal.h"
#include "visitor.h"

#include "kcalendarcore_debug.h"

using namespace KCalendarCore;

//@cond PRIVATE
class KCalendarCore::JournalPrivate : public IncidencePrivate
{
    bool validStatus(Incidence::Status) override;
};

bool JournalPrivate::validStatus(Incidence::Status status)
{
    constexpr unsigned validSet
        = 1u << Incidence::StatusNone
        | 1u << Incidence::StatusDraft
        | 1u << Incidence::StatusFinal
        | 1u << Incidence::StatusCanceled;
    return validSet & (1u << status);
}

//@endcond

Journal::Journal()
    : Incidence(new JournalPrivate)
{
}

Journal::Journal(const Journal &other)
    : Incidence(other, new JournalPrivate(*(other.d_func())))
{
}

Journal::~Journal() = default;

Incidence::IncidenceType Journal::type() const
{
    return TypeJournal;
}

QByteArray Journal::typeStr() const
{
    return QByteArrayLiteral("Journal");
}

Journal *Journal::clone() const
{
    return new Journal(*this);
}

IncidenceBase &Journal::assign(const IncidenceBase &other)
{
    if (&other != this) {
        Incidence::assign(other);
    }
    return *this;
}

bool Journal::equals(const IncidenceBase &journal) const
{
    return Incidence::equals(journal);
}

bool Journal::accept(Visitor &v, const IncidenceBase::Ptr &incidence)
{
    return v.visit(incidence.staticCast<Journal>());
}

QDateTime Journal::dateTime(DateTimeRole role) const
{
    switch (role) {
    case RoleEnd:
    case RoleEndTimeZone:
        return QDateTime();
    case RoleDisplayStart:
    case RoleDisplayEnd:
        return dtStart();
    default:
        return dtStart();
    }
}

void Journal::setDateTime(const QDateTime &dateTime, DateTimeRole role)
{
    switch (role) {
    case RoleDnD: {
        setDtStart(dateTime);
        break;
    }
    default:
        qCDebug(KCALCORE_LOG) << "Unhandled role" << role;
    }
}

void Journal::virtual_hook(VirtualHook id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}

QLatin1String Journal::mimeType() const
{
    return Journal::journalMimeType();
}

/* static */
QLatin1String Journal::journalMimeType()
{
    return QLatin1String("application/x-vnd.akonadi.calendar.journal");
}

QLatin1String Journal::iconName(const QDateTime &) const
{
    return QLatin1String("view-pim-journal");
}

void Journal::serialize(QDataStream &out) const
{
    Incidence::serialize(out);
}

void Journal::deserialize(QDataStream &in)
{
    Incidence::deserialize(in);
}

bool Journal::supportsGroupwareCommunication() const
{
    return false;
}
