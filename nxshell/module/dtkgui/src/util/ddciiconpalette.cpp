// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddciiconpalette.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QPalette>

DGUI_BEGIN_NAMESPACE

DDciIconPalette::DDciIconPalette(QColor foreground, QColor background, QColor highlight, QColor highlightForeground)
{
    colors.reserve(PaletteCount);
    colors.insert(Foreground, foreground);
    colors.insert(Background, background);
    colors.insert(HighlightForeground, highlightForeground);
    colors.insert(Highlight, highlight);
}

bool DDciIconPalette::operator==(const DDciIconPalette &other) const
{
    for (int i = Foreground; i < PaletteCount; ++i) {
        if (colors.at(i) != other.colors.at(i))
            return false;
    }

    return true;
}

bool DDciIconPalette::operator!=(const DDciIconPalette &other) const
{
    return !(*this == other);
}

QColor DDciIconPalette::foreground() const
{
    return colors.at(Foreground);
}

void DDciIconPalette::setForeground(const QColor &foreground)
{
    colors[Foreground] = foreground;
}

QColor DDciIconPalette::background() const
{
    return colors[Background];
}

void DDciIconPalette::setBackground(const QColor &background)
{
    colors[Background] = background;
}

QColor DDciIconPalette::highlightForeground() const
{
    return colors[HighlightForeground];
}

void DDciIconPalette::setHighlightForeground(const QColor &highlightForeground)
{
    colors[HighlightForeground] = highlightForeground;
}

QColor DDciIconPalette::highlight() const
{
    return colors[Highlight];
}

void DDciIconPalette::setHighlight(const QColor &highlight)
{
    colors[Highlight] = highlight;
}

static QString _d_dciIconPaletteHost()
{
    return QLatin1String("dtk.dci.palette");
}

QString DDciIconPalette::convertToString(const DDciIconPalette &palette)
{
    QUrl url;
    url.setHost(_d_dciIconPaletteHost());
    QUrlQuery query;
    if (palette.foreground().isValid())
        query.addQueryItem(QLatin1String("foreground"), palette.foreground().name(QColor::HexArgb));
    if (palette.background().isValid())
        query.addQueryItem(QLatin1String("background"), palette.background().name(QColor::HexArgb));
    if (palette.highlight().isValid())
        query.addQueryItem(QLatin1String("highlight"), palette.highlight().name(QColor::HexArgb));
    if (palette.highlightForeground().isValid())
        query.addQueryItem(QLatin1String("highlightForeground"), palette.highlightForeground().name(QColor::HexArgb));

    url.setQuery(query);
    return url.toString();
}

DDciIconPalette DDciIconPalette::convertFromString(const QString &data)
{
    QUrl url(data);
    if (url.host() != _d_dciIconPaletteHost())
        return DDciIconPalette();

    QUrlQuery query(url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).mid(1));
    QColor foreground;
    if (query.hasQueryItem(QLatin1String("foreground")))
        foreground = query.queryItemValue(QLatin1String("foreground"));
    QColor background;
    if (query.hasQueryItem(QLatin1String("background")))
        background = query.queryItemValue(QLatin1String("background"));
    QColor highlight;
    if (query.hasQueryItem(QLatin1String("highlight")))
        highlight = query.queryItemValue(QLatin1String("highlight"));
    QColor highlightForeground;
    if (query.hasQueryItem(QLatin1String("highlightForeground")))
        highlightForeground = query.queryItemValue(QLatin1String("highlightForeground"));
    return DDciIconPalette(foreground, background, highlight, highlightForeground);
}

DDciIconPalette DDciIconPalette::fromQPalette(const QPalette &pa)
{
    return DDciIconPalette(pa.windowText().color(), pa.window().color(),
                           pa.highlight().color(), pa.highlightedText().color());
}

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
DGUI_USE_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM

static inline QString getColorName(QColor color) {
    if (color.isValid())
        return color.name(QColor::HexArgb);
    return QLatin1String("InValid");
};

QDebug operator<<(QDebug dbg, const DDciIconPalette &pal)
{
    QDebugStateSaver saver(dbg);
    QDebug nospace = dbg.nospace();
    nospace << "DDciIconPalette(foreground: " << getColorName(pal.foreground())
            << ",background: " << getColorName(pal.background())
            << ",highlight: " << getColorName(pal.highlight())
            << ",highlightForeground: " << getColorName(pal.highlightForeground())
            << ")";
    return dbg;
}
#endif
QT_END_NAMESPACE
