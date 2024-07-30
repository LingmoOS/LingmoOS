/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
 */
// Almost completely copy/pasted from qtquickcontrols2 so that I don't need to depend on private headers

#include "qquickicon_p.h"

namespace Ocean
{

class QQuickIconPrivate : public QSharedData
{
public:
    QString name;
    QUrl source;
    QUrl resolvedSource; // UNUSED: needed to keep compatibility with Qt's QQuickIcon
    int width = 0;
    int height = 0;
    QColor color = Qt::transparent;
    bool cache = true;

    enum ResolveProperties {
        NameResolved = 0x0001,
        SourceResolved = 0x0002,
        WidthResolved = 0x0004,
        HeightResolved = 0x0008,
        ColorResolved = 0x0010,
        CacheResolved = 0x0020,
        AllPropertiesResolved = 0x1ffff,
    };

    // This is based on QFont's resolve_mask.
    int resolveMask = 0;
};

QQuickIcon::QQuickIcon()
    : d(new QQuickIconPrivate)
{
}

QQuickIcon::QQuickIcon(const QQuickIcon &other)
    : d(other.d)
{
}

QQuickIcon::~QQuickIcon()
{
}

QQuickIcon &QQuickIcon::operator=(const QQuickIcon &other)
{
    d = other.d;
    return *this;
}

bool QQuickIcon::operator==(const QQuickIcon &other) const
{
    return d == other.d //
        || (d->name == other.d->name //
            && d->source == other.d->source //
            && d->width == other.d->width //
            && d->height == other.d->height //
            && d->color == other.d->color //
            && d->cache == other.d->cache);
}

bool QQuickIcon::operator!=(const QQuickIcon &other) const
{
    return !(*this == other);
}

bool QQuickIcon::isEmpty() const
{
    return d->name.isEmpty() && d->source.isEmpty();
}

QString QQuickIcon::name() const
{
    return d->name;
}

void QQuickIcon::setName(const QString &name)
{
    if ((d->resolveMask & QQuickIconPrivate::NameResolved) && d->name == name)
        return;

    d.detach();
    d->name = name;
    d->resolveMask |= QQuickIconPrivate::NameResolved;
}

void QQuickIcon::resetName()
{
    d.detach();
    d->name = QString();
    d->resolveMask &= ~QQuickIconPrivate::NameResolved;
}

QUrl QQuickIcon::source() const
{
    return d->source;
}

void QQuickIcon::setSource(const QUrl &source)
{
    if ((d->resolveMask & QQuickIconPrivate::SourceResolved) && d->source == source)
        return;

    d.detach();
    d->source = source;
    d->resolveMask |= QQuickIconPrivate::SourceResolved;
}

void QQuickIcon::resetSource()
{
    d.detach();
    d->source = QUrl();
    d->resolveMask &= ~QQuickIconPrivate::SourceResolved;
}

QString QQuickIcon::nameOrSource() const
{
    if (!d->name.isEmpty()) {
        return d->name;
    } else if (d->source.isValid()) {
        return d->source.toString();
    }
    return QString();
}

int QQuickIcon::width() const
{
    return d->width;
}

void QQuickIcon::setWidth(int width)
{
    if ((d->resolveMask & QQuickIconPrivate::WidthResolved) && d->width == width)
        return;

    d.detach();
    d->width = width;
    d->resolveMask |= QQuickIconPrivate::WidthResolved;
}

void QQuickIcon::resetWidth()
{
    d.detach();
    d->width = 0;
    d->resolveMask &= ~QQuickIconPrivate::WidthResolved;
}

int QQuickIcon::height() const
{
    return d->height;
}

void QQuickIcon::setHeight(int height)
{
    if ((d->resolveMask & QQuickIconPrivate::HeightResolved) && d->height == height)
        return;

    d.detach();
    d->height = height;
    d->resolveMask |= QQuickIconPrivate::HeightResolved;
}

void QQuickIcon::resetHeight()
{
    d.detach();
    d->height = 0;
    d->resolveMask &= ~QQuickIconPrivate::HeightResolved;
}

QColor QQuickIcon::color() const
{
    return d->color;
}

void QQuickIcon::setColor(const QColor &color)
{
    if ((d->resolveMask & QQuickIconPrivate::ColorResolved) && d->color == color)
        return;

    d.detach();
    d->color = color;
    d->resolveMask |= QQuickIconPrivate::ColorResolved;
}

void QQuickIcon::resetColor()
{
    d.detach();
    d->color = Qt::transparent;
    d->resolveMask &= ~QQuickIconPrivate::ColorResolved;
}

bool QQuickIcon::cache() const
{
    return d->cache;
}

void QQuickIcon::setCache(bool cache)
{
    if ((d->resolveMask & QQuickIconPrivate::CacheResolved) && d->cache == cache)
        return;

    d.detach();
    d->cache = cache;
    d->resolveMask |= QQuickIconPrivate::CacheResolved;
}

void QQuickIcon::resetCache()
{
    d.detach();
    d->cache = true;
    d->resolveMask &= ~QQuickIconPrivate::CacheResolved;
}

QQuickIcon QQuickIcon::resolve(const QQuickIcon &other) const
{
    QQuickIcon resolved = *this;
    resolved.d.detach();

    if (!(d->resolveMask & QQuickIconPrivate::NameResolved))
        resolved.d->name = other.d->name;

    if (!(d->resolveMask & QQuickIconPrivate::SourceResolved))
        resolved.d->source = other.d->source;

    if (!(d->resolveMask & QQuickIconPrivate::WidthResolved))
        resolved.d->width = other.d->width;

    if (!(d->resolveMask & QQuickIconPrivate::HeightResolved))
        resolved.d->height = other.d->height;

    if (!(d->resolveMask & QQuickIconPrivate::ColorResolved))
        resolved.d->color = other.d->color;

    if (!(d->resolveMask & QQuickIconPrivate::CacheResolved))
        resolved.d->cache = other.d->cache;

    return resolved;
}

}

#include "moc_qquickicon_p.cpp"
