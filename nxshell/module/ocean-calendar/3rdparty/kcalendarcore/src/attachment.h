/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002 Michael Brade <brade@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Attachment class.

  @author Michael Brade \<brade@kde.org\>
*/

#ifndef KCALCORE_ATTACHMENT_H
#define KCALCORE_ATTACHMENT_H

#include <QHash>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

namespace KCalendarCore {
/**
  @brief
  Represents information related to an attachment for a Calendar Incidence.
  表示与日历事件的附件相关的信息。

  This is not an email message attachment.

  Calendar Incidence attachments consist of:
  - A <a href="https://en.wikipedia.org/wiki/Uniform_Resource_Identifier">
    Uniform Resource Identifier (URI)</a>
    or a
    <a href="https://en.wikipedia.org/wiki/Base64#MIME">base64 encoded</a>
    binary blob.
  - A <a href="https://en.wikipedia.org/wiki/MIME">
    Multipurpose Internet Mail Extensions (MIME)</a> type.

  This class is used to associate files (local or remote) or other resources
  with a Calendar Incidence.
*/
class Q_CORE_EXPORT Attachment
{
    Q_GADGET
    Q_PROPERTY(bool isEmpty READ isEmpty)
    Q_PROPERTY(QString uri READ uri WRITE setUri)
    Q_PROPERTY(bool isUri READ isUri)
    Q_PROPERTY(bool isBinary READ isBinary)
    Q_PROPERTY(int size READ size)
    Q_PROPERTY(QString mimeType READ mimeType WRITE setMimeType)
    Q_PROPERTY(bool showInline READ showInline WRITE setShowInline)
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(bool isLocal READ isLocal WRITE setLocal)

public:
    /**
      List of attachments.
    */
    typedef QVector<Attachment> List;

    /**
      Constructs an empty attachment.
    */
    explicit Attachment();

    /**
      Constructs an attachment consisting of a @p uri and a @p mime type.

      @param uri is the @acronym URI referred to by this attachment.
      @param mime is the (optional) @acronym MIME type of the @p uri
    */
    explicit Attachment(const QString &uri, const QString &mime = QString());

    /**
      Constructs an attachment consisting of a binary blob of data
      and a @p mime type.

      @param base64 is the binary data in base64 format for the attachment.
      @param mime is the (optional) @acronym MIME type of the attachment
    */
    explicit Attachment(const QByteArray &base64, const QString &mime = QString());

    /**
      Constructs an attachment by copying another attachment.

      @param attachment is the attachment to be copied.
    */
    Attachment(const Attachment &attachment);

    /**
      Destroys the attachment.
    */
    ~Attachment();

    /**
       Returns whether this is an empty or default constructed object.
    */
    bool isEmpty() const;

    /**
      Sets the @acronym URI for this attachment to @p uri.

      @param uri is the @acronym URI to use for the attachment.

      @see uri(), isUri()
    */
    void setUri(const QString &uri);

    /**
      Returns the @acronym URI of the attachment.

      @see setUri(), isUri()
    */
    Q_REQUIRED_RESULT QString uri() const;

    /**
      Returns true if the attachment has a @acronym URI; false otherwise.

      @see uri(), setUri(I), isBinary()
    */
    Q_REQUIRED_RESULT bool isUri() const;

    /**
      Returns true if the attachment has a binary blob; false otherwise.

      @see isUri()
    */
    Q_REQUIRED_RESULT bool isBinary() const;

    /**
      Sets the base64 encoded binary blob data of the attachment.

      @param base64 contains the base64 encoded binary data.

      @see data(), decodedData()
    */
    void setData(const QByteArray &base64);

    /**
      Returns a pointer to a QByteArray containing the base64 encoded
      binary data of the attachment.

      @see setData(), setDecodedData()
    */
    Q_REQUIRED_RESULT QByteArray data() const;

    /**
      Sets the decoded attachment data.

      @param data is the decoded base64 binary data.

      @see decodedData(), data()
    */
    void setDecodedData(const QByteArray &data);

    /**
      Returns a QByteArray containing the decoded base64 binary data of the
      attachment.

      @see setDecodedData(), setData()
    */
    Q_REQUIRED_RESULT QByteArray decodedData() const;

    /**
      Returns the size of the attachment, in bytes.
      If the attachment is binary (i.e, there is no @acronym URI associated
      with the attachment) then a value of 0 is returned.
    */
    uint size() const;

    /**
      Sets the @acronym MIME-type of the attachment to @p mime.

      @param mime is the string to use for the attachment @acronym MIME-type.

      @see mimeType()
    */
    void setMimeType(const QString &mime);

    /**
      Returns the @acronym MIME-type of the attachment.

      @see setMimeType()
    */
    Q_REQUIRED_RESULT QString mimeType() const;

    /**
      Sets the attachment "show in-line" option, which is derived from
      the Calendar Incidence @b X-CONTENT-DISPOSITION parameter.

      @param showinline is the flag to set (true) or unset (false)
      for the attachment "show in-line" option.

      @see showInline()
    */
    void setShowInline(bool showinline);

    /**
      Returns the attachment "show in-line" flag.

      @see setShowInline()
    */
    Q_REQUIRED_RESULT bool showInline() const;

    /**
      Sets the attachment label to @p label, which is derived from
      the Calendar Incidence @b X-LABEL parameter.

      @param label is the string to use for the attachment label.

      @see label()
    */
    void setLabel(const QString &label);

    /**
      Returns the attachment label string.
    */
    Q_REQUIRED_RESULT QString label() const;

    /**
      Sets the attachment "local" option, which is derived from the
      Calendar Incidence @b X-KONTACT-TYPE parameter.

      @param local is the flag to set (true) or unset (false) for the
      attachment "local" option.

      @see local()
    */
    void setLocal(bool local);

    /**
      Returns the attachment "local" flag.
    */
    Q_REQUIRED_RESULT bool isLocal() const;

    /**
      Assignment operator.
      @param attachment is the attachment to assign.
    */
    Attachment &operator=(const Attachment &attachment);

    /**
      Compare this with @p attachment for equality.
      @param attachment is the attachment to compare.
      @return true if the attachments are equal; false otherwise.
     */
    bool operator==(const Attachment &attachment) const;

    /**
      Compare this with @p attachment for inequality.
      @param attachment is the attachment to compare.
      @return true if the attachments are /not/ equal; false otherwise.
     */
    bool operator!=(const Attachment &attachment) const;

private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond

    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &s, const KCalendarCore::Attachment &);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &s, KCalendarCore::Attachment &);
};

/**
 * Attachment serializer.
 *
 * @since 4.12
 */
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &out, const KCalendarCore::Attachment &);

/**
 * Attachment deserializer.
 *
 * @since 4.12
 */
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &in, KCalendarCore::Attachment &);

} // namespace KCalendarCore

//@cond PRIVATE
Q_DECLARE_TYPEINFO(KCalendarCore::Attachment, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KCalendarCore::Attachment)
//@endcond

#endif
