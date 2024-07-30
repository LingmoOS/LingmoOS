/*
    SPDX-FileCopyrightText: 2013-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXT_ANIMATION_H
#define KATE_TEXT_ANIMATION_H

#include <QObject>
#include <QRectF>
#include <QString>

#include <ktexteditor/attribute.h>
#include <ktexteditor/range.h>

namespace KTextEditor
{
class DocumentPrivate;
}
class KateViewInternal;
class QTimeLine;
class QPainter;

/**
 * This class is used to flash text in the text view.
 * The duration of the flash animation is about 250 milliseconds.
 * When the animation is finished, it deletes itself.
 */
class KateTextAnimation : public QObject
{
public:
    KateTextAnimation(KTextEditor::Range range, KTextEditor::Attribute::Ptr attribute, KateViewInternal *view);
    ~KateTextAnimation() override;

    // draw the text to highlight, given the current animation progress
    void draw(QPainter &painter);

public:
    // request repaint from view of the respective region
    void nextFrame(qreal value);

private:
    // calculate rect for the text to highlight, given the current animation progress
    QRectF rectForText();

private:
    KTextEditor::Range m_range;
    QString m_text;
    KTextEditor::Attribute::Ptr m_attribute;

    KTextEditor::DocumentPrivate *m_doc;
    KateViewInternal *m_view;
    QTimeLine *m_timeLine;
    qreal m_value;
};

#endif // KATE_TEXT_ANIMATION_H
