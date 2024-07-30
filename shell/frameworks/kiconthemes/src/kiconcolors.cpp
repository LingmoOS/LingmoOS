/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kiconcolors.h"
#include <KColorScheme>

static QString STYLESHEET_TEMPLATE()
{
    /* clang-format off */
    return QStringLiteral(".ColorScheme-Text { color:%1; }\
.ColorScheme-Background{ color:%2; }\
.ColorScheme-Highlight{ color:%3; }\
.ColorScheme-HighlightedText{ color:%4; }\
.ColorScheme-PositiveText{ color:%5; }\
.ColorScheme-NeutralText{ color:%6; }\
.ColorScheme-NegativeText{ color:%7; }\
.ColorScheme-ActiveText{ color:%8; }\
.ColorScheme-Complement{ color:%9; }\
.ColorScheme-Contrast{ color:%10; }\
.ColorScheme-Accent{ color:%11; }\
");
    /* clang-format on */
}

class KIconColorsPrivate : public QSharedData
{
public:
    KIconColorsPrivate()
    {
    }
    KIconColorsPrivate(const KIconColorsPrivate &other)
        : QSharedData(other)
        , text(other.text)
        , background(other.background)
        , highlight(other.highlight)
        , highlightedText(other.highlightedText)
        , accent(other.accent)
        , positiveText(other.positiveText)
        , neutralText(other.neutralText)
        , negativeText(other.negativeText)
    {
    }
    ~KIconColorsPrivate()
    {
    }

    QColor text;
    QColor background;
    QColor highlight;
    QColor highlightedText;
    QColor accent;
    QColor positiveText;
    QColor neutralText;
    QColor negativeText;
    QColor activeText;
    static std::optional<QPalette> lastPalette;
    static std::optional<KColorScheme> lastColorScheme;
};

std::optional<QPalette> KIconColorsPrivate::lastPalette;
std::optional<KColorScheme> KIconColorsPrivate::lastColorScheme;

KIconColors::KIconColors()
    : KIconColors(QPalette())
{
}

KIconColors::KIconColors(const KIconColors &other)
    : d_ptr(other.d_ptr)
{
}

KIconColors KIconColors::operator=(const KIconColors &other)
{
    if (d_ptr != other.d_ptr) {
        d_ptr = other.d_ptr;
    }
    return *this;
}

KIconColors::KIconColors(const QColor &colors)
    : d_ptr(new KIconColorsPrivate)
{
    Q_D(KIconColors);
    d->text = colors;
    d->background = colors;
    d->highlight = colors;
    d->highlightedText = colors;
    d->positiveText = colors;
    d->neutralText = colors;
    d->negativeText = colors;
    d->accent = colors;
}

KIconColors::KIconColors(const QPalette &palette)
    : d_ptr(new KIconColorsPrivate)
{
    Q_D(KIconColors);
    d->text = palette.windowText().color();
    d->background = palette.window().color();
    d->highlight = palette.highlight().color();
    d->highlightedText = palette.highlightedText().color();
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    d->accent = palette.accent().color();
#else
    d->accent = palette.highlight().color();
#endif

    if (!d->lastColorScheme || !d->lastPalette || palette != d->lastPalette) {
        d->lastPalette = palette;
        d->lastColorScheme = KColorScheme(QPalette::Active, KColorScheme::Window);
    }

    d->positiveText = d->lastColorScheme->foreground(KColorScheme::PositiveText).color().name();
    d->neutralText = d->lastColorScheme->foreground(KColorScheme::NeutralText).color().name();
    d->negativeText = d->lastColorScheme->foreground(KColorScheme::NegativeText).color().name();
    d->activeText = d->lastColorScheme->foreground(KColorScheme::ActiveText).color().name();
}

KIconColors::~KIconColors()
{
}

qreal luma(const QColor &color) {
    return (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
}

QString KIconColors::stylesheet(KIconLoader::States state) const
{
    Q_D(const KIconColors);

    const QColor complement =
        luma(d->background) > 0.5 ? Qt::white : Qt::black;

    const QColor contrast =
        luma(d->background) > 0.5 ? Qt::black : Qt::white;

    QColor accentColor = d->accent;
    // When selected, tint the accent color with a small portion of highlighted text color,
    // because since the accent color used to be the same as the highlight color, it might cause
    // icons, especially folders to "disappear" against the background
    if (state == KIconLoader::SelectedState) {
        const qreal tintRatio = 0.85;
        const qreal r = accentColor.redF() * tintRatio + d->highlightedText.redF() * (1.0 - tintRatio);
        const qreal g = accentColor.greenF() * tintRatio + d->highlightedText.greenF() * (1.0 - tintRatio);
        const qreal b = accentColor.blueF() * tintRatio + d->highlightedText.blueF() * (1.0 - tintRatio);
        accentColor.setRgbF(r, g, b, accentColor.alphaF());
    }

    return STYLESHEET_TEMPLATE()
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->text.name())
        .arg(state == KIconLoader::SelectedState ? d->highlight.name() : d->background.name())
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->highlight.name())
        .arg(state == KIconLoader::SelectedState ? d->highlight.name() : d->highlightedText.name())
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->positiveText.name())
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->neutralText.name())
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->negativeText.name())
        .arg(state == KIconLoader::SelectedState ? d->highlightedText.name() : d->activeText.name())
        .arg(complement.name())
        .arg(contrast.name())
        .arg(accentColor.name());
}

QColor KIconColors::highlight() const
{
    Q_D(const KIconColors);
    return d->highlight;
}

QColor KIconColors::highlightedText() const
{
    Q_D(const KIconColors);
    return d->highlightedText;
}

QColor KIconColors::accent() const
{
    Q_D(const KIconColors);
    return d->accent;
}

QColor KIconColors::background() const
{
    Q_D(const KIconColors);
    return d->background;
}

QColor KIconColors::text() const
{
    Q_D(const KIconColors);
    return d->text;
}

QColor KIconColors::negativeText() const
{
    Q_D(const KIconColors);
    return d->negativeText;
}

QColor KIconColors::positiveText() const
{
    Q_D(const KIconColors);
    return d->positiveText;
}

QColor KIconColors::neutralText() const
{
    Q_D(const KIconColors);
    return d->neutralText;
}

QColor KIconColors::activeText() const
{
    Q_D(const KIconColors);
    return d->activeText;
}

void KIconColors::setText(const QColor &color)
{
    Q_D(KIconColors);
    d->text = color;
}

void KIconColors::setBackground(const QColor &color)
{
    Q_D(KIconColors);
    d->background = color;
}

void KIconColors::setHighlight(const QColor &color)
{
    Q_D(KIconColors);
    d->highlight = color;
}

void KIconColors::setHighlightedText(const QColor &color)
{
    Q_D(KIconColors);
    d->highlightedText = color;
}

void KIconColors::setAccent(const QColor &color)
{
    Q_D(KIconColors);
    d->accent = color;
}

void KIconColors::setNegativeText(const QColor &color)
{
    Q_D(KIconColors);
    d->negativeText = color;
}

void KIconColors::setNeutralText(const QColor &color)
{
    Q_D(KIconColors);
    d->neutralText = color;
}

void KIconColors::setPositiveText(const QColor &color)
{
    Q_D(KIconColors);
    d->positiveText = color;
}

void KIconColors::setActiveText(const QColor &color)
{
    Q_D(KIconColors);
    d->activeText = color;
}
