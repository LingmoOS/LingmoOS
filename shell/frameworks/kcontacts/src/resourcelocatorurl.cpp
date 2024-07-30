/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcelocatorurl.h"

#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN ResourceLocatorUrl::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        url = other.url;
    }

    ParameterMap mParamMap;
    QUrl url;
};

ResourceLocatorUrl::ResourceLocatorUrl()
    : d(new Private)
{
}

ResourceLocatorUrl::ResourceLocatorUrl(const ResourceLocatorUrl &other)
    : d(other.d)
{
}

ResourceLocatorUrl::~ResourceLocatorUrl()
{
}

struct url_type_name {
    const char *name;
    ResourceLocatorUrl::Type type;
};

static const url_type_name url_type_names[] = {
    {"HOME", ResourceLocatorUrl::Home},
    {"WORK", ResourceLocatorUrl::Work},
    {"OTHER", ResourceLocatorUrl::Other},
    {"PROFILE", ResourceLocatorUrl::Profile},
    {"FTP", ResourceLocatorUrl::Ftp},
    {"RESERVATION", ResourceLocatorUrl::Reservation},
    {"APPINSTALLPAGE", ResourceLocatorUrl::AppInstallPage},
};

ResourceLocatorUrl::Type ResourceLocatorUrl::type() const
{
    const auto it = d->mParamMap.findParam(QLatin1String("type"));
    if (it == d->mParamMap.cend()) {
        return Unknown;
    }

    Type type = Unknown;
    for (const QString &s : it->paramValues) {
        const auto typeIt = std::find_if(std::begin(url_type_names), std::end(url_type_names), [&s](const url_type_name &t) {
            return QLatin1String(t.name) == s;
        });
        if (typeIt != std::end(url_type_names)) {
            type |= (*typeIt).type;
        }
    }
    return type;
}

void ResourceLocatorUrl::setType(ResourceLocatorUrl::Type type)
{
    const auto oldType = this->type();

    const QString paramName(QStringLiteral("type"));

    auto it = d->mParamMap.findParam(paramName);
    if (it == d->mParamMap.end()) {
        it = d->mParamMap.insertParam({QLatin1String("type"), {}});
    }

    for (const auto &t : url_type_names) {
        if (((type ^ oldType) & t.type) == 0) {
            continue; // no change
        }

        const QLatin1String str(t.name);
        if (type & t.type) {
            it->paramValues.push_back(str);
        } else {
            it->paramValues.removeAll(str);
        }
    }
}

bool ResourceLocatorUrl::isPreferred() const
{
    auto it = d->mParamMap.findParam(QLatin1String("pref"));
    if (it != d->mParamMap.cend()) {
        return !it->paramValues.isEmpty() && it->paramValues.at(0) == QLatin1Char('1');
    }

    it = d->mParamMap.findParam(QLatin1String("type"));
    if (it != d->mParamMap.cend()) {
        return it->paramValues.contains(QLatin1String("PREF"), Qt::CaseInsensitive);
    }

    return false;
}

void ResourceLocatorUrl::setPreferred(bool preferred)
{
    if (preferred == isPreferred()) {
        return;
    }

    const QString paramName(QStringLiteral("type"));

    auto it = d->mParamMap.findParam(paramName);
    QStringList types = it != d->mParamMap.end() ? it->paramValues : QStringList{};

    const QLatin1String PREF_Str("PREF");
    if (!preferred) {
        auto prefIt = d->mParamMap.findParam(QLatin1String("pref"));
        if (prefIt != d->mParamMap.end()) {
            d->mParamMap.erase(prefIt);
        }

        types.removeAll(PREF_Str);
    } else {
        types.push_back(PREF_Str);
    }

    // The above erase() call could have invalidated "it"
    it = d->mParamMap.findParam(paramName);
    if (it != d->mParamMap.end()) {
        it->paramValues = types;
    } else {
        d->mParamMap.insertParam({QLatin1String("type"), types});
    }
}

bool ResourceLocatorUrl::operator==(const ResourceLocatorUrl &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->url == other.url());
}

bool ResourceLocatorUrl::operator!=(const ResourceLocatorUrl &other) const
{
    return !(other == *this);
}

ResourceLocatorUrl &ResourceLocatorUrl::operator=(const ResourceLocatorUrl &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString ResourceLocatorUrl::toString() const
{
    QString str = QLatin1String("ResourceLocatorUrl {\n");
    str += QStringLiteral("    url: %1\n").arg(d->url.toString());
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

void ResourceLocatorUrl::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap ResourceLocatorUrl::params() const
{
    return d->mParamMap;
}

bool ResourceLocatorUrl::isValid() const
{
    return d->url.isValid();
}

void ResourceLocatorUrl::setUrl(const QUrl &url)
{
    d->url = url;
}

QUrl ResourceLocatorUrl::url() const
{
    return d->url;
}

QDataStream &KContacts::operator<<(QDataStream &s, const ResourceLocatorUrl &calUrl)
{
    return s << calUrl.d->mParamMap << calUrl.d->url;
}

QDataStream &KContacts::operator>>(QDataStream &s, ResourceLocatorUrl &calUrl)
{
    s >> calUrl.d->mParamMap >> calUrl.d->url;
    return s;
}

#include "moc_resourcelocatorurl.cpp"
