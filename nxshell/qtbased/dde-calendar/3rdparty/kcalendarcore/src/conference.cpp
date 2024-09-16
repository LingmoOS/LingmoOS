/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "conference.h"

#include <QDataStream>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Conference::Private : public QSharedData
{
public:
    QString label;
    QString language;
    QStringList features;
    QUrl uri;
    CustomProperties customProperties;
};
//@endcond

Conference::Conference()
    : d(new Conference::Private)
{
}

Conference::Conference(const QUrl &uri, const QString &label, const QStringList &features, const QString &language)
    : d(new Conference::Private)
{
    setUri(uri);
    setLabel(label);
    setFeatures(features);
    setLanguage(language);
}

Conference::Conference(const Conference &) = default;

Conference::~Conference() = default;

bool Conference::isNull() const
{
    // isNull rather than isEmpty, as user code is actually creating empty but non-null conferences...
    return !d->uri.isValid() && d->label.isNull();
}

bool KCalendarCore::Conference::operator==(const Conference &other) const
{
    return std::tie(d->label, d->language, d->features, d->uri) == std::tie(other.d->label, other.d->language, other.d->features, other.d->uri);
}

bool KCalendarCore::Conference::operator!=(const Conference &other) const
{
    return !operator==(other);
}

Conference &KCalendarCore::Conference::operator=(const Conference &) = default;

QUrl Conference::uri() const
{
    return d->uri;
}

void Conference::setUri(const QUrl &uri)
{
    d->uri = uri;
}

QString Conference::label() const
{
    return d->label;
}

void Conference::setLabel(const QString &label)
{
    d->label = label;
}

QStringList Conference::features() const
{
    return d->features;
}

void Conference::addFeature(const QString &feature)
{
    d->features.push_back(feature);
}

void Conference::removeFeature(const QString &feature)
{
    d->features.removeAll(feature);
}

void Conference::setFeatures(const QStringList &features)
{
    d->features = features;
}

QString Conference::language() const
{
    return d->language;
}

void Conference::setLanguage(const QString &language)
{
    d->language = language;
}

void Conference::setCustomProperty(const QByteArray &xname, const QString &xvalue)
{
    d->customProperties.setNonKDECustomProperty(xname, xvalue);
}

CustomProperties &Conference::customProperties()
{
    return d->customProperties;
}

const CustomProperties &Conference::customProperties() const
{
    return d->customProperties;
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::Conference &conference)
{
    return stream << conference.d->uri << conference.d->label << conference.d->features << conference.d->language << conference.d->customProperties;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, KCalendarCore::Conference &conference)
{
    Conference conf;
    stream >> conf.d->uri >> conf.d->label >> conf.d->features >> conf.d->language >> conf.d->customProperties;
    conference = conf;

    return stream;
}
