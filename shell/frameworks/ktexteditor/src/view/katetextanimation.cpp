/*
    SPDX-FileCopyrightText: 2013-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katetextanimation.h"

#include "katerenderer.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include <ktexteditor/document.h>

#include <QPainter>
#include <QPointF>
#include <QRect>
#include <QSizeF>
#include <QTimeLine>

KateTextAnimation::KateTextAnimation(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute, KateViewInternal *view)
    : QObject(view)
    , m_range(range)
    , m_text(view->view()->document()->text(range))
    , m_attribute(std::move(attribute))
    , m_doc(view->view()->doc())
    , m_view(view)
    , m_timeLine(new QTimeLine(250, this))
    , m_value(0.0)
{
    connect(m_timeLine, &QTimeLine::valueChanged, this, &KateTextAnimation::nextFrame);
    connect(m_timeLine, &QTimeLine::finished, this, &KateTextAnimation::deleteLater);

    m_timeLine->setEasingCurve(QEasingCurve::SineCurve);
    m_timeLine->start();

    QObject::connect(view, &KTextEditor::View::destroyed, m_timeLine, &QTimeLine::stop);
}

KateTextAnimation::~KateTextAnimation()
{
    // if still running, we need to update the view a last time to avoid artifacts
    if (m_timeLine->state() == QTimeLine::Running) {
        m_timeLine->stop();
        nextFrame(0.0);
    }
}

QRectF KateTextAnimation::rectForText()
{
    const QFontMetricsF fm = m_view->view()->renderer()->currentFontMetrics();
    const int lineHeight = m_view->view()->renderer()->lineHeight();
    QPoint pixelPos = m_view->cursorToCoordinate(m_range.start(), /*bool realCursor*/ true, /*bool includeBorder*/ false);

    if (pixelPos.x() == -1 || pixelPos.y() == -1) {
        return QRectF();
    } else {
        QRectF rect(pixelPos.x(), pixelPos.y(), fm.boundingRect(m_view->view()->document()->text(m_range)).width(), lineHeight);
        const QPointF center = rect.center();
        const qreal factor = 1.0 + 0.5 * m_value;
        rect.setWidth(rect.width() * factor);
        rect.setHeight(rect.height() * factor);
        rect.moveCenter(center);
        return rect;
    }
}

void KateTextAnimation::draw(QPainter &painter)
{
    // could happen in corner cases: timeLine emitted finished(), but this object
    // is not yet deleted. Therefore, draw() might be called in paintEvent().
    if (m_timeLine->state() == QTimeLine::NotRunning) {
        return;
    }

    // get current rect and fill background
    QRectF rect = rectForText();
    painter.fillRect(rect, m_attribute->background());

    // scale font with animation
    QFont f = m_view->view()->renderer()->currentFont();
    f.setBold(m_attribute->fontBold());
    f.setPointSizeF(f.pointSizeF() * (1.0 + 0.5 * m_value));
    painter.setFont(f);

    painter.setPen(m_attribute->foreground().color());
    // finally draw contents on the view
    painter.drawText(rect, m_text);
}

void KateTextAnimation::nextFrame(qreal value)
{
    // cache previous rect for update
    const QRectF prevRect = rectForText();

    m_value = value;

    // next rect is used to draw the text
    const QRectF nextRect = rectForText();

    // due to rounding errors, increase the rect 1px to avoid artifacts
    const QRect updateRect = nextRect.united(prevRect).adjusted(-1, -1, 1, 1).toRect();

    // request repaint
    m_view->update(updateRect);
}
