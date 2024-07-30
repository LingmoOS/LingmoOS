/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "mockdecoration.h"
#include "mockbridge.h"

#include <QMap>
#include <QVariantMap>
#include <utility>

MockDecoration::MockDecoration(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
}

#ifdef _MSC_VER
QMap<QString, QVariant> makeMap(const QString &key, const QVariant &value)
{
    QMap<QString, QVariant> ret;
    ret.insert(key, value);
    return ret;
}
MockDecoration::MockDecoration(MockBridge *bridge)
    : MockDecoration(nullptr, QVariantList({makeMap(QStringLiteral("bridge"), QVariant::fromValue(bridge))}))
#else
MockDecoration::MockDecoration(MockBridge *bridge)
    : MockDecoration(nullptr, QVariantList({QVariantMap({{QStringLiteral("bridge"), QVariant::fromValue(bridge)}})}))
#endif
{
}

bool MockDecoration::init()
{
    return true;
}

void MockDecoration::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(painter)
    Q_UNUSED(repaintRegion)
}

void MockDecoration::setOpaque(bool set)
{
    Decoration::setOpaque(set);
}

void MockDecoration::setBorders(const QMargins &m)
{
    Decoration::setBorders(m);
}

void MockDecoration::setTitleBar(const QRect &rect)
{
    Decoration::setTitleBar(rect);
}

#include "moc_mockdecoration.cpp"
