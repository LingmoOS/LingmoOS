/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwordwrap.h"

#include <QList>
#include <QPainter>

class KWordWrapPrivate : public QSharedData
{
public:
    QRect m_constrainingRect;
    QList<int> m_breakPositions;
    QList<int> m_lineWidths;
    QRect m_boundingRect;
    QString m_text;
};

KWordWrap::KWordWrap(const QRect &r)
    : d(new KWordWrapPrivate)
{
    d->m_constrainingRect = r;
}

KWordWrap KWordWrap::formatText(QFontMetrics &fm, const QRect &r, int /*flags*/, const QString &str, int len)
{
    KWordWrap kw(r);
    // The wordwrap algorithm
    // The variable names and the global shape of the algorithm are inspired
    // from QTextFormatterBreakWords::format().
    // qDebug() << "KWordWrap::formatText " << str << " r=" << r.x() << "," << r.y() << " " << r.width() << "x" << r.height();
    int height = fm.height();
    if (len == -1) {
        kw.d->m_text = str;
    } else {
        kw.d->m_text = str.left(len);
    }
    if (len == -1) {
        len = str.length();
    }
    int lastBreak = -1;
    int lineWidth = 0;
    int x = 0;
    int y = 0;
    int w = r.width();
    int textwidth = 0;
    bool isBreakable = false;
    bool wasBreakable = false; // value of isBreakable for last char (i-1)
    bool isParens = false; // true if one of ({[
    bool wasParens = false; // value of isParens for last char (i-1)
    QString inputString = str;

    for (int i = 0; i < len; ++i) {
        const QChar c = inputString.at(i);
        const int ww = fm.horizontalAdvance(c);

        isParens = (c == QLatin1Char('(') //
                    || c == QLatin1Char('[') //
                    || c == QLatin1Char('{'));
        // isBreakable is true when we can break _after_ this character.
        isBreakable = (c.isSpace() || c.isPunct() || c.isSymbol()) & !isParens;

        // Special case for '(', '[' and '{': we want to break before them
        if (!isBreakable && i < len - 1) {
            const QChar nextc = inputString.at(i + 1); // look at next char
            isBreakable = (nextc == QLatin1Char('(') //
                           || nextc == QLatin1Char('[') //
                           || nextc == QLatin1Char('{'));
        }
        // Special case for '/': after normal chars it's breakable (e.g. inside a path),
        // but after another breakable char it's not (e.g. "mounted at /foo")
        // Same thing after a parenthesis (e.g. "dfaure [/fool]")
        if (c == QLatin1Char('/') && (wasBreakable || wasParens)) {
            isBreakable = false;
        }

        /*qDebug() << "c='" << QString(c) << "' i=" << i << "/" << len
                  << " x=" << x << " ww=" << ww << " w=" << w
                  << " lastBreak=" << lastBreak << " isBreakable=" << isBreakable << endl;*/
        int breakAt = -1;
        if (x + ww > w && lastBreak != -1) { // time to break and we know where
            breakAt = lastBreak;
        }
        if (x + ww > w - 4 && lastBreak == -1) { // time to break but found nowhere [-> break here]
            breakAt = i;
        }
        if (i == len - 2 && x + ww + fm.horizontalAdvance(inputString.at(i + 1)) > w) { // don't leave the last char alone
            breakAt = lastBreak == -1 ? i - 1 : lastBreak;
        }
        if (c == QLatin1Char('\n')) { // Forced break here
            if (breakAt == -1 && lastBreak != -1) { // only break if not already breaking
                breakAt = i - 1;
                lastBreak = -1;
            }
            // remove the line feed from the string
            kw.d->m_text.remove(i, 1);
            inputString.remove(i, 1);
            len--;
        }
        if (breakAt != -1) {
            // qDebug() << "KWordWrap::formatText breaking after " << breakAt;
            kw.d->m_breakPositions.append(breakAt);
            int thisLineWidth = lastBreak == -1 ? x + ww : lineWidth;
            kw.d->m_lineWidths.append(thisLineWidth);
            textwidth = qMax(textwidth, thisLineWidth);
            x = 0;
            y += height;
            wasBreakable = true;
            wasParens = false;
            if (lastBreak != -1) {
                // Breakable char was found, restart from there
                i = lastBreak;
                lastBreak = -1;
                continue;
            }
        } else if (isBreakable) {
            lastBreak = i;
            lineWidth = x + ww;
        }
        x += ww;
        wasBreakable = isBreakable;
        wasParens = isParens;
    }
    textwidth = qMax(textwidth, x);
    kw.d->m_lineWidths.append(x);
    y += height;
    // qDebug() << "KWordWrap::formatText boundingRect:" << r.x() << "," << r.y() << " " << textwidth << "x" << y;
    if (r.height() >= 0 && y > r.height()) {
        textwidth = r.width();
    }
    int realY = y;
    if (r.height() >= 0) {
        while (realY > r.height()) {
            realY -= height;
        }
        realY = qMax(realY, 0);
    }
    kw.d->m_boundingRect.setRect(0, 0, textwidth, realY);
    return kw;
}

KWordWrap::~KWordWrap()
{
}

KWordWrap::KWordWrap(const KWordWrap &other)
    : d(other.d)
{
}

KWordWrap &KWordWrap::operator=(const KWordWrap &other)
{
    d = other.d;
    return *this;
}

QString KWordWrap::wrappedString() const
{
    const QStringView strView(d->m_text);
    // We use the calculated break positions to insert '\n' into the string
    QString ws;
    int start = 0;
    for (int i = 0; i < d->m_breakPositions.count(); ++i) {
        int end = d->m_breakPositions.at(i);
        ws += strView.mid(start, end - start + 1);
        ws += QLatin1Char('\n');
        start = end + 1;
    }
    ws += strView.mid(start);
    return ws;
}

QString KWordWrap::truncatedString(bool dots) const
{
    if (d->m_breakPositions.isEmpty()) {
        return d->m_text;
    }

    QString ts = d->m_text.left(d->m_breakPositions.first() + 1);
    if (dots) {
        ts += QLatin1String("...");
    }
    return ts;
}

static QColor mixColors(double p1, QColor c1, QColor c2)
{
    return QColor(int(c1.red() * p1 + c2.red() * (1.0 - p1)), //
                  int(c1.green() * p1 + c2.green() * (1.0 - p1)), //
                  int(c1.blue() * p1 + c2.blue() * (1.0 - p1)));
}

void KWordWrap::drawFadeoutText(QPainter *p, int x, int y, int maxW, const QString &t)
{
    QFontMetrics fm = p->fontMetrics();
    QColor bgColor = p->background().color();
    QColor textColor = p->pen().color();

    if ((fm.boundingRect(t).width() > maxW) && (t.length() > 1)) {
        int tl = 0;
        int w = 0;
        while (tl < t.length()) {
            w += fm.horizontalAdvance(t.at(tl));
            if (w >= maxW) {
                break;
            }
            tl++;
        }

        int n = qMin(tl, 3);
        if (t.isRightToLeft()) {
            x += maxW; // start from the right side for RTL string
            if (tl > 3) {
                x -= fm.horizontalAdvance(t.left(tl - 3));
                p->drawText(x, y, t.left(tl - 3));
            }
            for (int i = 0; i < n; i++) {
                p->setPen(mixColors(0.70 - i * 0.25, textColor, bgColor));
                QString s(t.at(tl - n + i));
                x -= fm.horizontalAdvance(s);
                p->drawText(x, y, s);
            }
        } else {
            if (tl > 3) {
                p->drawText(x, y, t.left(tl - 3));
                x += fm.horizontalAdvance(t.left(tl - 3));
            }
            for (int i = 0; i < n; i++) {
                p->setPen(mixColors(0.70 - i * 0.25, textColor, bgColor));
                QString s(t.at(tl - n + i));
                p->drawText(x, y, s);
                x += fm.horizontalAdvance(s);
            }
        }
    } else {
        p->drawText(x, y, t);
    }
}

void KWordWrap::drawTruncateText(QPainter *p, int x, int y, int maxW, const QString &t)
{
    QString tmpText = p->fontMetrics().elidedText(t, Qt::ElideRight, maxW);
    p->drawText(x, y, tmpText);
}

void KWordWrap::drawText(QPainter *painter, int textX, int textY, int flags) const
{
    // qDebug() << "KWordWrap::drawText text=" << wrappedString() << " x=" << textX << " y=" << textY;
    // We use the calculated break positions to draw the text line by line using QPainter
    int start = 0;
    int y = 0;
    QFontMetrics fm = painter->fontMetrics();
    int height = fm.height(); // line height
    int ascent = fm.ascent();
    int maxwidth = d->m_boundingRect.width();
    int i;
    int lwidth = 0;
    int end = 0;
    for (i = 0; i < d->m_breakPositions.count(); ++i) {
        // if this is the last line, leave the loop
        if (d->m_constrainingRect.height() >= 0 //
            && ((y + 2 * height) > d->m_constrainingRect.height())) {
            break;
        }
        end = d->m_breakPositions.at(i);
        lwidth = d->m_lineWidths.at(i);
        int x = textX;
        if (flags & Qt::AlignHCenter) {
            x += (maxwidth - lwidth) / 2;
        } else if (flags & Qt::AlignRight) {
            x += maxwidth - lwidth;
        }
        painter->drawText(x, textY + y + ascent, d->m_text.mid(start, end - start + 1));
        y += height;
        start = end + 1;
    }

    // Draw the last line
    lwidth = d->m_lineWidths.last();
    int x = textX;
    if (flags & Qt::AlignHCenter) {
        x += (maxwidth - lwidth) / 2;
    } else if (flags & Qt::AlignRight) {
        x += maxwidth - lwidth;
    }
    if ((d->m_constrainingRect.height() < 0) || ((y + height) <= d->m_constrainingRect.height())) {
        if (i == d->m_breakPositions.count()) {
            painter->drawText(x, textY + y + ascent, d->m_text.mid(start));
        } else if (flags & FadeOut) {
            drawFadeoutText(painter, textX, textY + y + ascent, d->m_constrainingRect.width(), d->m_text.mid(start));
        } else if (flags & Truncate) {
            drawTruncateText(painter, textX, textY + y + ascent, d->m_constrainingRect.width(), d->m_text.mid(start));
        } else {
            painter->drawText(x, textY + y + ascent, d->m_text.mid(start));
        }
    }
}

QRect KWordWrap::boundingRect() const
{
    return d->m_boundingRect;
}
