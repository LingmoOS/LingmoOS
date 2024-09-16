/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002, 2006, 2010 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CustomProperties class.

  @brief
  A class to manage custom calendar properties.

  @author David Jarvie \<djarvie@kde.org\>
*/

#include "customproperties.h"

#include <QDataStream>
#include <QDebug>

using namespace KCalendarCore;

//@cond PRIVATE
static bool checkName(const QByteArray &name);

class Q_DECL_HIDDEN CustomProperties::Private
{
public:
    bool operator==(const Private &other) const;
    QMap<QByteArray, QString> mProperties; // custom calendar properties
    QMap<QByteArray, QString> mPropertyParameters;

    // Volatile properties are not written back to the serialized format and are not compared in operator==
    // They are only used for runtime purposes and are not part of the payload.
    QMap<QByteArray, QString> mVolatileProperties;

    bool isVolatileProperty(const QString &name) const
    {
        return name.startsWith(QLatin1String("X-KDE-VOLATILE"));
    }
};

bool CustomProperties::Private::operator==(const CustomProperties::Private &other) const
{
    if (mProperties.count() != other.mProperties.count()) {
        // qDebug() << "Property count is different:" << mProperties << other.mProperties;
        return false;
    }
    for (QMap<QByteArray, QString>::ConstIterator it = mProperties.begin(); it != mProperties.end(); ++it) {
        QMap<QByteArray, QString>::ConstIterator itOther = other.mProperties.find(it.key());
        if (itOther == other.mProperties.end() || itOther.value() != it.value()) {
            return false;
        }
    }
    for (QMap<QByteArray, QString>::ConstIterator it = mPropertyParameters.begin(); it != mPropertyParameters.end(); ++it) {
        QMap<QByteArray, QString>::ConstIterator itOther = other.mPropertyParameters.find(it.key());
        if (itOther == other.mPropertyParameters.end() || itOther.value() != it.value()) {
            return false;
        }
    }
    return true;
}
//@endcond

CustomProperties::CustomProperties()
    : d(new Private)
{
}

CustomProperties::CustomProperties(const CustomProperties &cp)
    : d(new Private(*cp.d))
{
}

CustomProperties &CustomProperties::operator=(const CustomProperties &other)
{
    // check for self assignment
    if (&other == this) {
        return *this;
    }

    *d = *other.d;
    return *this;
}

CustomProperties::~CustomProperties()
{
    delete d;
}

bool CustomProperties::operator==(const CustomProperties &other) const
{
    return *d == *other.d;
}

void CustomProperties::setCustomProperty(const QByteArray &app, const QByteArray &key, const QString &value)
{
    if (value.isNull() || key.isEmpty() || app.isEmpty()) {
        return;
    }
    QByteArray property = "X-KDE-" + app + '-' + key;
    if (!checkName(property)) {
        return;
    }
    customPropertyUpdate();

    if (d->isVolatileProperty(QLatin1String(property))) {
        d->mVolatileProperties[property] = value;
    } else {
        d->mProperties[property] = value;
    }

    customPropertyUpdated();
}

void CustomProperties::removeCustomProperty(const QByteArray &app, const QByteArray &key)
{
    removeNonKDECustomProperty(QByteArray("X-KDE-" + app + '-' + key));
}

QString CustomProperties::customProperty(const QByteArray &app, const QByteArray &key) const
{
    return nonKDECustomProperty(QByteArray("X-KDE-" + app + '-' + key));
}

QByteArray CustomProperties::customPropertyName(const QByteArray &app, const QByteArray &key)
{
    QByteArray property("X-KDE-" + app + '-' + key);
    if (!checkName(property)) {
        return QByteArray();
    }
    return property;
}

void CustomProperties::setNonKDECustomProperty(const QByteArray &name, const QString &value, const QString &parameters)
{
    if (value.isNull() || !checkName(name)) {
        return;
    }
    customPropertyUpdate();
    if (d->isVolatileProperty(QLatin1String(name))) {
        d->mVolatileProperties[name] = value;
    } else {
        d->mProperties[name] = value;
        d->mPropertyParameters[name] = parameters;
    }
    customPropertyUpdated();
}
void CustomProperties::removeNonKDECustomProperty(const QByteArray &name)
{
    if (d->mProperties.contains(name)) {
        customPropertyUpdate();
        d->mProperties.remove(name);
        d->mPropertyParameters.remove(name);
        customPropertyUpdated();
    } else if (d->mVolatileProperties.contains(name)) {
        customPropertyUpdate();
        d->mVolatileProperties.remove(name);
        customPropertyUpdated();
    }
}

QString CustomProperties::nonKDECustomProperty(const QByteArray &name) const
{
    return d->isVolatileProperty(QLatin1String(name)) ? d->mVolatileProperties.value(name) : d->mProperties.value(name);
}

QString CustomProperties::nonKDECustomPropertyParameters(const QByteArray &name) const
{
    return d->mPropertyParameters.value(name);
}

void CustomProperties::setCustomProperties(const QMap<QByteArray, QString> &properties)
{
    bool changed = false;
    for (QMap<QByteArray, QString>::ConstIterator it = properties.begin(); it != properties.end(); ++it) {
        // Validate the property name and convert any null string to empty string
        if (checkName(it.key())) {
            if (d->isVolatileProperty(QLatin1String(it.key()))) {
                d->mVolatileProperties[it.key()] = it.value().isNull() ? QLatin1String("") : it.value();
            } else {
                d->mProperties[it.key()] = it.value().isNull() ? QLatin1String("") : it.value();
            }
            if (!changed) {
                customPropertyUpdate();
            }
            changed = true;
        }
    }
    if (changed) {
        customPropertyUpdated();
    }
}

QMap<QByteArray, QString> CustomProperties::customProperties() const
{
    QMap<QByteArray, QString> result = d->mProperties;
    for (auto it = d->mVolatileProperties.begin(), end = d->mVolatileProperties.end(); it != end; ++it) {
        result.insert(it.key(), it.value());
    }
    return result;
}

void CustomProperties::customPropertyUpdate()
{
}

void CustomProperties::customPropertyUpdated()
{
}

//@cond PRIVATE
bool checkName(const QByteArray &name)
{
    // Check that the property name starts with 'X-' and contains
    //检查属性名称是否以“X-”开头，并包含
    // only the permitted characters
    //只有允许的字符
    const char *n = name.constData();
    int len = name.length();
    if (len < 2 || n[0] != 'X' || n[1] != '-') {
        return false;
    }
    for (int i = 2; i < len; ++i) {
        char ch = n[i];
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-') {
            continue;
        }
        return false; // invalid character found
    }
    return true;
}
//@endcond

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::CustomProperties &properties)
{
    return stream << properties.d->mProperties << properties.d->mPropertyParameters;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, KCalendarCore::CustomProperties &properties)
{
    properties.d->mVolatileProperties.clear();
    return stream >> properties.d->mProperties >> properties.d->mPropertyParameters;
}
