/*
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUG_H
#define BUG_H

#include <QObject>
#include <QPointer>

#include <QVariant>

#include "comment.h"

namespace Bugzilla
{
// Models a bugzilla bug.
class Bug : public QObject
{
public:
    enum class Status {
        Unknown, // First value is default if QMetaEnum can't map the key.
        UNCONFIRMED,
        CONFIRMED,
        ASSIGNED,
        REOPENED,
        RESOLVED,
        NEEDSINFO,
        VERIFIED,
        CLOSED,
    };
    Q_ENUM(Status)

    enum class Resolution {
        Unknown, // First value is default if QMetaEnum can't map the key.
        NONE, // Fake value, expresses unresolved. On the REST side this is an empty string.
        FIXED,
        INVALID,
        WONTFIX,
        LATER,
        REMIND,
        DUPLICATE,
        WORKSFORME,
        MOVED,
        UPSTREAM,
        DOWNSTREAM,
        WAITINGFORINFO,
        BACKTRACE,
        UNMAINTAINED,
    };
    Q_ENUM(Resolution)

private:
    Q_OBJECT
    Q_PROPERTY(qint64 id READ id MEMBER m_id NOTIFY changed)
    Q_PROPERTY(QString product READ product MEMBER m_product NOTIFY changed)
    Q_PROPERTY(QString component READ component MEMBER m_component NOTIFY changed)
    Q_PROPERTY(QString summary READ summary MEMBER m_summary NOTIFY changed)
    Q_PROPERTY(QString version READ version MEMBER m_version NOTIFY changed)
    Q_PROPERTY(bool is_open READ is_open MEMBER m_is_open NOTIFY changed)
    // maybe should be camel mapped, who knows
    Q_PROPERTY(QString op_sys READ op_sys MEMBER m_op_sys NOTIFY changed)
    Q_PROPERTY(QString priority READ priority MEMBER m_priority NOTIFY changed)
    Q_PROPERTY(QString severity READ severity MEMBER m_severity NOTIFY changed)
    Q_PROPERTY(Status status READ status MEMBER m_status NOTIFY changed)
    Q_PROPERTY(Resolution resolution READ resolution MEMBER m_resolution NOTIFY changed)
    Q_PROPERTY(qint64 dupe_of READ dupe_of MEMBER m_dupe_of NOTIFY changed)

    // Custom fields (versionfixedin etc) are only available via customField().

public:
    using Ptr = QPointer<Bug>;

    explicit Bug(const QVariantHash &object, QObject *parent = nullptr);

    qint64 id() const;
    void setId(qint64 id);

    QVariant customField(const char *key);

    Status status() const;
    Resolution resolution() const;
    QString summary() const;
    QString version() const;
    QString product() const;
    QString component() const;
    QString op_sys() const;
    QString priority() const;
    QString severity() const;
    bool is_open() const;
    qint64 dupe_of() const;

Q_SIGNALS:
    void commentsChanged();
    void changed();

private:
    qint64 m_id = -1;
    QString m_product;
    QString m_component;
    QString m_summary;
    QString m_version;
    bool m_is_open = false;
    QString m_op_sys;
    QString m_priority;
    QString m_severity;
    Status m_status = Status::Unknown;
    Resolution m_resolution = Resolution::Unknown;
    qint64 m_dupe_of = -1;
};

} // namespace Bugzilla

Q_DECLARE_METATYPE(Bugzilla::Bug *);

#endif // BUG_H
