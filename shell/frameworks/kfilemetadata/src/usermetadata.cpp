/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "usermetadata.h"
#include "xattr_p.h"


using namespace KFileMetaData;

class KFileMetaData::UserMetaDataPrivate
{
public:
    QString filePath;
};

UserMetaData::UserMetaData(const QString& filePath)
    : d(new UserMetaDataPrivate)
{
    d->filePath = filePath;
}

UserMetaData::UserMetaData(const UserMetaData& rhs)
    : d(new UserMetaDataPrivate(*rhs.d))
{
}

UserMetaData::~UserMetaData() = default;

const UserMetaData& UserMetaData::operator=(const UserMetaData& rhs)
{
    d->filePath = rhs.d->filePath;
    return *this;
}

QString UserMetaData::filePath() const
{
    return d->filePath;
}

UserMetaData::Error UserMetaData::setTags(const QStringList& tags)
{
    return setAttribute(QStringLiteral("xdg.tags"), !tags.empty() ? tags.join(QLatin1Char(',')) : QString());
}

QStringList UserMetaData::tags() const
{
    QString value;

    k_getxattr(d->filePath, QStringLiteral("xdg.tags"), &value);
    return value.split(QLatin1Char(','), Qt::SkipEmptyParts);
}

int UserMetaData::rating() const
{
    return attribute(QStringLiteral("baloo.rating")).toInt();
}

UserMetaData::Error UserMetaData::setRating(int rating)
{
    return setAttribute(QStringLiteral("baloo.rating"), rating ? QString::number(rating) : QString());
}

QString UserMetaData::userComment() const
{
    return attribute(QStringLiteral("xdg.comment"));
}

UserMetaData::Error UserMetaData::setUserComment(const QString& userComment)
{
    return setAttribute(QStringLiteral("xdg.comment"), userComment);
}

QUrl UserMetaData::originUrl() const
{
    return QUrl(attribute(QStringLiteral("xdg.origin.url")));
}

UserMetaData::Error UserMetaData::setOriginUrl(const QUrl &originUrl)
{
    return setAttribute(QStringLiteral("xdg.origin.url"), !originUrl.isEmpty() ? originUrl.toString(): QString());
}

QString UserMetaData::originEmailSubject() const
{
    return attribute(QStringLiteral("xdg.origin.email.subject"));
}

UserMetaData::Error UserMetaData::setOriginEmailSubject(const QString &originEmailSubject)
{
    return setAttribute(QStringLiteral("xdg.origin.email.subject"), originEmailSubject);
}

QString UserMetaData::originEmailSender() const
{
    return attribute(QStringLiteral("xdg.origin.email.sender"));
}

UserMetaData::Error UserMetaData::setOriginEmailSender(const QString &originEmailSender)
{
    return setAttribute(QStringLiteral("xdg.origin.email.sender"), originEmailSender);
}

QString UserMetaData::originEmailMessageId() const
{
    return attribute(QStringLiteral("xdg.origin.email.message-id"));
}

UserMetaData::Error UserMetaData::setOriginEmailMessageId(const QString &originEmailMessageId)
{
    return setAttribute(QStringLiteral("xdg.origin.email.message-id"), originEmailMessageId);
}

UserMetaData::Error UserMetaData::setAttribute(const QString& key, const QString& value)
{
    int result;
    if (!value.isEmpty()) {
        result = k_setxattr(d->filePath, key, value);
    } else {
        result = k_removexattr(d->filePath, key);
    }

    if (result != 0) {
        switch (result) {
#ifdef Q_OS_UNIX
        case EDQUOT:
#endif
        case ENOSPC:
            return NoSpace;
        case ENOTSUP:
            return NotSupported;
        case EACCES:
        case EPERM:
            return MissingPermission;
        case ENAMETOOLONG:
        case ERANGE:
            return NameToolong;
        case E2BIG:
            return ValueTooBig;
        default:
            return UnknownError;
        }
    }
    return NoError;
}

#if KFILEMETADATA_BUILD_DEPRECATED_SINCE(6, 2)
bool UserMetaData::hasAttribute(const QString& key)
{
    return std::as_const(*this).hasAttribute(key);
}
#endif

bool UserMetaData::hasAttribute(const QString &key) const
{
    return k_hasAttribute(d->filePath, key);
}

#if KFILEMETADATA_BUILD_DEPRECATED_SINCE(6, 2)
QString UserMetaData::attribute(const QString &key)
{
    return std::as_const(*this).attribute(key);
}
#endif

QString UserMetaData::attribute(const QString& key) const
{
    QString value;
    k_getxattr(d->filePath, key, &value);

    return value;
}

bool UserMetaData::isSupported() const
{
    return k_isSupported(d->filePath);
}

UserMetaData::Attributes UserMetaData::queryAttributes(UserMetaData::Attributes attributes) const
{
    return k_queryAttributes(d->filePath, attributes);
}
