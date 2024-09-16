/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Journal class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/
#ifndef KCALCORE_JOURNAL_H
#define KCALCORE_JOURNAL_H

#include "incidence.h"

namespace KCalendarCore {
/**
  @brief
  Provides a Journal in the sense of RFC2445.
*/
class Q_CORE_EXPORT Journal : public Incidence
{
public:
    /**
      A shared pointer to a Journal object.
    */
    typedef QSharedPointer<Journal> Ptr;

    /**
      List of journals.
    */
    typedef QVector<Ptr> List;

    ///@cond PRIVATE
    // needed for Akonadi polymorphic payload support
    typedef Incidence SuperClass;
    ///@endcond

    /**
      Constructs an empty journal.
    */
    Journal();

    /** Copy a journey object. */
    Journal(const Journal &);

    /**
      Destroys a journal.
    */
    ~Journal() override;

    /**
      @copydoc
      IncidenceBase::type()
    */
    Q_REQUIRED_RESULT IncidenceType type() const override;

    /**
      @copydoc
      IncidenceBase::typeStr()
    */
    Q_REQUIRED_RESULT QByteArray typeStr() const override;

    /**
      Returns an exact copy of this journal. The returned object is owned
      by the caller.
    */
    Journal *clone() const override;

    /**
      @copydoc
      IncidenceBase::dateTime(DateTimeRole)const
    */
    Q_REQUIRED_RESULT QDateTime dateTime(DateTimeRole role) const override;

    /**
      @copydoc
      IncidenceBase::setDateTime(const QDateTime &, DateTimeRole )
    */
    void setDateTime(const QDateTime &dateTime, DateTimeRole role) override;

    /**
       @copydoc
       IncidenceBase::mimeType()
    */
    Q_REQUIRED_RESULT QLatin1String mimeType() const override;

    /**
       @copydoc
       Incidence::iconName()
    */
    Q_REQUIRED_RESULT QLatin1String iconName(const QDateTime &recurrenceId = {}) const override;

    /**
       @copydoc
       Incidence::supportsGroupwareCommunication()
    */
    Q_REQUIRED_RESULT bool supportsGroupwareCommunication() const override;

    /**
       Returns the Akonadi specific sub MIME type of a KCalendarCore::Journal.
    */
    Q_REQUIRED_RESULT static QLatin1String journalMimeType();

protected:
    /**
      Compare this with @p journal for equality.

      @param journal is the journal to compare.
    */
    bool equals(const IncidenceBase &journal) const override;

    /**
      @copydoc
      IncidenceBase::assign()
    */
    IncidenceBase &assign(const IncidenceBase &other) override;

    /**
      @copydoc
      IncidenceBase::virtual_hook()
    */
    void virtual_hook(VirtualHook id, void *data) override;

private:
    /**
      @copydoc
      IncidenceBase::accept(Visitor &, const IncidenceBase::Ptr &)
    */
    bool accept(Visitor &v, const IncidenceBase::Ptr &incidence) override;

    /**
      Disabled, otherwise could be dangerous if you subclass Journal.
      Use IncidenceBase::operator= which is safe because it calls
      virtual function assign().
      @param other is another Journal object to assign to this one.
     */
    Journal &operator=(const Journal &other) = delete;

    // For polymorfic serialization
    void serialize(QDataStream &out) const override;
    void deserialize(QDataStream &in) override;

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Journal::Ptr, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Journal::Ptr)
Q_DECLARE_METATYPE(KCalendarCore::Journal *)
//@endcond

#endif
