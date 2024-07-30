// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#include "kcountryflagemojiiconengine.h"

#include <QDebug>
#include <QFont>
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>

using namespace Qt::Literals::StringLiterals;

namespace
{

Q_GLOBAL_STATIC(QFont, s_globalDefaultFont, "emoji"_L1)

QString makeCountryEmoji(const QString &country)
{
    // The way this was set up by unicode is actually pretty smart. Country flags are based on their two character
    // country codes within a given range of code points. And even better, the offset inside the range is the same
    // as the offset inside ASCII. Meaning the offset of 'A' from 0 is the same as the offset of 🇦 in the
    // flag codepoint range. The way a flag is then denoted is e.g. <SURROGATEPAIR>🇦<SURROGATEPAIR>🇹 resulting in
    // the Austrian flag.
    // https://en.wikipedia.org/wiki/Regional_indicator_symbol

    static constexpr auto surrogatePairCodePoint = 0xD83C; // U+D83C
    static constexpr auto flagCodePointStart = 0xDDE6; // U+1F1E6 (🇦) - NB: we are in UTF-16
    static constexpr auto offsetCodePointA = 'A'_L1.unicode(); // offset from 0, the flag code points have the same offsets
    static constexpr auto basePoint = flagCodePointStart - offsetCodePointA;

    QString emoji;
    emoji.reserve(2 * country.size());
    for (const auto &c : country) {
        emoji.append(QChar(surrogatePairCodePoint));
        emoji.append(QChar(basePoint + c.toUpper().unicode()));
    }

    return emoji;
}

QString makeRegionEmoji(const QString &region)
{
    // Region flags work much the same as country flags but with a slightly different format in a slightly different
    // code point region. Specifically they use ISO 3166-2 as input (e.g. GB-SCT for Scotland). It all happens in
    // the Unicode Block “Tags” (starting at U+E0000) wherein it functions the same as the country codes do in their
    // block. The offsets inside the block are the same as the ascii offsets and the emoji is constructed by combining
    // the off set code points of the incoming region tag. They are prefixed with U+1F3F4 🏴 WAVING BLACK FLAG
    // and suffixed with U+E007F CANCEL TAG.
    // https://en.wikipedia.org/wiki/Regional_indicator_symbol

    auto hyphenlessRegion = region;
    hyphenlessRegion.remove('-'_L1);

    static constexpr auto surrogatePairCodePoint = 0xdb40; // U+DB40
    static constexpr auto flagCodePointStart = 0xDC41; // U+E0041 (Tag Latin Capital Letter A) - NB: we are in UTF-16
    static constexpr auto offsetCodePointA = 'A'_L1.unicode(); // offset from 0, the flag code points have the same offsets
    static constexpr auto basePoint = flagCodePointStart - offsetCodePointA;

    auto emoji = u"🏴"_s;
    emoji.reserve(emoji.size() + 2 * hyphenlessRegion.size() + 2);
    for (const auto &c : hyphenlessRegion) {
        emoji.append(QChar(surrogatePairCodePoint));
        emoji.append(QChar(basePoint + c.toLower().unicode()));
    }
    static const auto cancelTag = QString().append(QChar(surrogatePairCodePoint)).append(QChar(0xDC7F));
    return emoji.append(cancelTag);
}

} // namespace

class Q_DECL_HIDDEN KCountryFlagEmojiIconEnginePrivate
{
public:
    explicit KCountryFlagEmojiIconEnginePrivate(const QString &regionOrCountry)
        : m_country(regionOrCountry)
        , m_emoji(regionOrCountry.contains("-"_L1) ? makeRegionEmoji(regionOrCountry) : makeCountryEmoji(regionOrCountry))
    {
    }

    const QString m_country;
    const QString m_emoji;
};

KCountryFlagEmojiIconEngine::KCountryFlagEmojiIconEngine(const QString &country)
    : d(std::make_unique<KCountryFlagEmojiIconEnginePrivate>(country))
{
}

KCountryFlagEmojiIconEngine::~KCountryFlagEmojiIconEngine() = default;

QIconEngine *KCountryFlagEmojiIconEngine::clone() const
{
    return new KCountryFlagEmojiIconEngine(d->m_country);
}

QString KCountryFlagEmojiIconEngine::key() const
{
    return u"org.kde.KCountryFlagEmojiIconEngine"_s;
}

void KCountryFlagEmojiIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    // Not supported
    Q_UNUSED(mode);
    Q_UNUSED(state);

    QFont font(*s_globalDefaultFont, painter->device());
    font.setPixelSize(qMax(rect.width(), rect.height()));
    font.setFixedPitch(true);

    QFontMetricsF metrics(font, painter->device());
    QRectF tightRect = metrics.tightBoundingRect(d->m_emoji);
    while (tightRect.width() > rect.width() || tightRect.height() > rect.height()) {
        const auto widthDelta = std::floor(tightRect.width() - rect.width());
        const auto heightDelta = std::floor(tightRect.height() - rect.height());
        auto delta = std::max(std::max(1.0, widthDelta), std::max(1.0, heightDelta));
        if (delta >= font.pixelSize()) {
            // when the delta is too large we'll chop the pixel size in half and hope the delta comes within a more reasonable range the next loop run
            static constexpr auto halfSize = 2;
            delta = std::floor(font.pixelSize() / halfSize);
        }
        font.setPixelSize(std::floor(font.pixelSize() - delta));
        metrics = QFontMetricsF(font, painter->device());
        tightRect = metrics.tightBoundingRect(d->m_emoji);
    }

    const QRectF flagBoundingRect = metrics.boundingRect(rect, Qt::AlignCenter, d->m_emoji);

    painter->setPen(qGuiApp->palette().color(QPalette::WindowText)); // in case we render the letters in absence of a flag
    // Confusingly the pixelSize for drawing must actually be without DPR but the rect calculation above
    // seems to be correct even with DPR in the pixelSize.
    font.setPixelSize(std::floor(font.pixelSize() / painter->device()->devicePixelRatioF()));
    painter->setFont(font);
    painter->drawText(flagBoundingRect, d->m_emoji);
}

QPixmap KCountryFlagEmojiIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap KCountryFlagEmojiIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale)
{
    QPixmap pixmap(size);
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);
    {
        QPainter p(&pixmap);
        paint(&p, QRect(QPoint(0, 0), size), mode, state);
    }
    return pixmap;
}

bool KCountryFlagEmojiIconEngine::isNull()
{
    return d->m_emoji.isEmpty();
}

void KCountryFlagEmojiIconEngine::setGlobalDefaultFont(const QFont &font)
{
    QFont swapable(font);
    s_globalDefaultFont->swap(swapable);
}
