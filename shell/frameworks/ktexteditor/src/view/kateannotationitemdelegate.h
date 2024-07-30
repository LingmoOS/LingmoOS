/*
    SPDX-FileCopyrightText: 2017-18 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_ANNOTATIONITEMDELEGATE_H
#define KATE_ANNOTATIONITEMDELEGATE_H

#include <ktexteditor/abstractannotationitemdelegate.h>

namespace KTextEditor
{
class ViewPrivate;
}
class KateViewInternal;

class KateAnnotationItemDelegate : public KTextEditor::AbstractAnnotationItemDelegate
{
public:
    explicit KateAnnotationItemDelegate(QObject *parent);
    ~KateAnnotationItemDelegate() override;

public:
    void paint(QPainter *painter, const KTextEditor::StyleOptionAnnotationItem &option, KTextEditor::AnnotationModel *model, int line) const override;
    bool helpEvent(QHelpEvent *event,
                   KTextEditor::View *view,
                   const KTextEditor::StyleOptionAnnotationItem &option,
                   KTextEditor::AnnotationModel *model,
                   int line) override;
    void hideTooltip(KTextEditor::View *view) override;
    QSize sizeHint(const KTextEditor::StyleOptionAnnotationItem &option, KTextEditor::AnnotationModel *model, int line) const override;

private:
    mutable qreal m_maxCharWidth = 0.0;
    mutable QFontMetricsF m_cachedDataContentFontMetrics;
};

#endif
