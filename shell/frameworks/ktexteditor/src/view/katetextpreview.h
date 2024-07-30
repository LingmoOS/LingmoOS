/*
    SPDX-FileCopyrightText: 2016 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXT_PREVIEW_H
#define KATE_TEXT_PREVIEW_H

#include <QFrame>

// namespace KTextEditor { class DocumentPrivate; }
namespace KTextEditor
{
class ViewPrivate;
}

/**
 * TODO
 */
class KateTextPreview : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal line READ line WRITE setLine)
    Q_PROPERTY(bool showFoldedLines READ showFoldedLines WRITE setShowFoldedLines)
    Q_PROPERTY(bool centerView READ centerView WRITE setCenterView)
    Q_PROPERTY(qreal scaleFactor READ scaleFactor WRITE setScaleFactor)

public:
    KateTextPreview(KTextEditor::ViewPrivate *view, QWidget *parent);

    KTextEditor::ViewPrivate *view() const;

    /**
     * Sets @p line as preview line.
     */
    void setLine(qreal line);

    /**
     * Returns the line set with setLine().
     */
    qreal line() const;

    /**
     * Enabled/disable centering the view on the line set with setLine().
     * If @p center is false, the first visible line is the once specified in
     * setLine(). If @p center is true, the specified line is vertically
     * centered. By default, centering the preview is set to true.
     */
    void setCenterView(bool center);

    /**
     * Returns whether view centering is enabled.
     */
    bool centerView() const;

    /**
     * Sets the scale factor.
     * The default scale factor is 1.0.
     * For text previews, you may want a scale factor of e.g. 0.75.
     * Negative scale factors are not allowed.
     */
    void setScaleFactor(qreal factor);

    /**
     * Returns the scale factor set with setScale().
     * The default value is 1.0.
     */
    qreal scaleFactor() const;

    /**
     * Sets whether folded lines are hidden or not.
     * By default, folded liens are not visible.
     */
    void setShowFoldedLines(bool on);

    /**
     * Returns whether folded lines are hidden.
     */
    bool showFoldedLines() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    KTextEditor::ViewPrivate *m_view;
    qreal m_line;
    bool m_showFoldedLines;
    bool m_center;
    qreal m_scale;
};

#endif
