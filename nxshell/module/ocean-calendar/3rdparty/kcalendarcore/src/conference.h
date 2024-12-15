/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALCORE_CONFERENCE_H
#define KCALCORE_CONFERENCE_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QUrl>

#include "customproperties.h"

namespace KCalendarCore {
/**
  @brief
  Represents information related to a conference information of an Calendar
  Incidence, typically a meeting or task (to-do).
  表示与会议相关的信息日历事件的信息，通常是会议或任务（待办事项）。

  Conference contains information needed to join a remote conference system
  (e.g. phone call, audio/video meeting etc.)

  @since 5.77
*/
class Q_CORE_EXPORT Conference
{
    Q_GADGET
    Q_PROPERTY(bool isNull READ isNull)
    Q_PROPERTY(QStringList features READ features WRITE setFeatures)
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(QUrl uri READ uri WRITE setUri)
    Q_PROPERTY(QString language READ language WRITE setLanguage)

public:
    using List = QVector<Conference>;

    /** Create a null Conference. */
    explicit Conference();

    /**
      Constructs a conference consisting of a @p uri, description of
      the URI (@p label), list of features of the conference (@p features)
      and @p language.

      @param uri Uri to join the conference.
      @param label Label of the URI.
      @param features Features of this particular conference method.
      @param language Language of the information present in other fields.
    */
    Conference(const QUrl &uri, const QString &label, const QStringList &features = {}, const QString &language = {});

    /**
      Constructs a conference by copying another conference.

      @param conference is the conference to be copied.
    */
    Conference(const Conference &conference);

    /**
      Destroys the conference.
    */
    ~Conference();

    /**
      Compares this with @p conference for equality.

      @param conference the conference to compare.
    */
    bool operator==(const Conference &conference) const;

    /**
      Compares this with @p conference for inequality.

      @param conference the conference to compare.
    */
    bool operator!=(const Conference &other) const;

    /**
     * Returns @c true if this is a default-constructed Conference instance.
     */
    Q_REQUIRED_RESULT bool isNull() const;

    /**
     * Returns URI to join the conference, with access code included.
     */
    Q_REQUIRED_RESULT QUrl uri() const;

    /**
     * Sets the URI to @uri.
     */
    void setUri(const QUrl &uri);

    /**
     * Returns label with additional details regarding further use of the URI.
     */
    Q_REQUIRED_RESULT QString label() const;

    /**
     * Sets the URI label to @p label.
     */
    void setLabel(const QString &label);

    /**
     * Returns the list of features of the conferencing system at given URI.
     *
     * This can be e.g. CHAT, AUDIO, VIDEO, PHONE, etc.
     */
    Q_REQUIRED_RESULT QStringList features() const;

    /**
     * Adds @p feature to the list of features.
     *
     * @param feature Feature to add.
     */
    void addFeature(const QString &feature);

    /**
     * Removes @p feature from the list of features.
     *
     * @param feature Feature to remove.
     */
    void removeFeature(const QString &feature);

    /**
     * Sets the list of features to @p features.
     */
    void setFeatures(const QStringList &features);

    /**
     * Returns the language of the text present in other properties of this object.
     */
    Q_REQUIRED_RESULT QString language() const;

    /**
     * Sets the language to @p language.
     */
    void setLanguage(const QString &language);

    /**
      Sets this conference equal to @p conference.

      @param conference is the conference to copy.
    */
    Conference &operator=(const Conference &conference);

    /**
      Adds a custom property. If the property already exists it will be overwritten.
      @param xname is the name of the property.
      @param xvalue is its value.
    */
    void setCustomProperty(const QByteArray &xname, const QString &xvalue);

    /**
      Returns a reference to the CustomProperties object
    */
    Q_REQUIRED_RESULT CustomProperties &customProperties();

    /**
      Returns a const reference to the CustomProperties object
    */
    const CustomProperties &customProperties() const;

private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const KCalendarCore::Conference &);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, KCalendarCore::Conference &);
};

/**
  Serializes a Conference object into a data stream.
  @param stream is a QDataStream.
  @param conference is a reference to a Conference object to be serialized.
*/
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KCalendarCore::Conference &conference);

/**
  Initializes a Conference object from a data stream.
  @param stream is a QDataStream.
  @param conference is a reference to a Conference object to be initialized.
*/
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, KCalendarCore::Conference &conference);

} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Conference, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Conference)
//@endcond

#endif
