/*
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_ABSTRACTANNOTATIONITEMDELEGATE_H
#define KTEXTEDITOR_ABSTRACTANNOTATIONITEMDELEGATE_H

#include <ktexteditor_export.h>

#include <QObject>
#include <QStyleOption>

class QHelpEvent;
class QPoint;

namespace KTextEditor
{
class AnnotationModel;
class View;

/**
 * \brief The style option set for an annotation item, as painted by AbstractAnnotationItemDelegate
 *
 * \since 5.53
 * \see KTextEditor::AbstractAnnotationItemDelegate
 */
class KTEXTEDITOR_EXPORT StyleOptionAnnotationItem : public QStyleOption
{
public:
    // TODO: not sure what SO_Default implies, but no clue how to maintain a user type registry?
    enum StyleOptionType { Type = SO_Default };
    enum StyleOptionVersion { Version = 1 };

    /**
     * Index of the displayed line in the wrapped lines for the given real line
     */
    int wrappedLine = 0;
    /**
     * Number of wrapped lines for the given real line
     *
     * A value of 1 means no wrapping has happened and the real line is displayed as one line.
     */
    int wrappedLineCount = 1;
    /**
     * Index of the displayed line in the displayed lines for the same group
     */
    int visibleWrappedLineInGroup = 0;

    /**
     * The view where the annotation is shown
     *
     * There is always a view set.
     */
    KTextEditor::View *view = nullptr;
    /**
     * Recommended size for icons or other symbols that will be rendered by the delegate
     *
     * The default value is QSize(-1, -1).
     */
    QSize decorationSize;
    /**
     * The metrics of the font used for rendering the text document
     */
    QFontMetricsF contentFontMetrics;

    /**
     * Enum for describing the relative position of a real line in the row of consecutive
     * displayed lines which belong to the same group of annotation items
     * @see AnnotationItemGroupPositions
     */
    enum AnnotationItemGroupPosition {
        InvalidGroupPosition = 0, ///< Position not specified or not belonging to a group
        InGroup = 0x1 << 0, ///< Real line belongs to a group
        GroupBegin = 0x1 << 1, ///< Real line is first of consecutive lines from same group
        GroupEnd = 0x1 << 2, ///< Real line is last of consecutive lines from same group
    };
    /// Stores a combination of #AnnotationItemGroupPosition values.
    Q_DECLARE_FLAGS(AnnotationItemGroupPositions, AnnotationItemGroupPosition)

    /**
     * Relative position of the real line in the row of consecutive displayed lines
     * which belong to the same group of annotation items
     */
    AnnotationItemGroupPositions annotationItemGroupingPosition = InvalidGroupPosition;

public:
    StyleOptionAnnotationItem();
    StyleOptionAnnotationItem(const StyleOptionAnnotationItem &other);
    StyleOptionAnnotationItem &operator=(const StyleOptionAnnotationItem &) = default;

protected:
    explicit StyleOptionAnnotationItem(int version);
};

/**
 * \class AbstractAnnotationItemDelegate abstractannotationitemdelegate.h <KTextEditor/AbstractAnnotationItemDelegate>
 *
 * \brief A delegate for rendering line annotation information and handling events
 *
 * \section annodelegate_intro Introduction
 *
 * AbstractAnnotationItemDelegate is a base class that can be reimplemented
 * to customize the rendering of annotation information for each line in a document.
 * It provides also the hooks to define handling of help events like tooltip or of
 * the request for a context menu.
 *
 * \section annodelegate_impl Implementing an AbstractAnnotationItemDelegate
 *
 * The public interface of this class is loosely based on the QAbstractItemDelegate
 * interfaces. It has five methods to implement.
 *
 * \since 5.53
 * \see KTextEditor::AnnotationModel, KTextEditor::AnnotationViewInterface
 */
class KTEXTEDITOR_EXPORT AbstractAnnotationItemDelegate : public QObject
{
    Q_OBJECT

protected:
    explicit AbstractAnnotationItemDelegate(QObject *parent = nullptr);

public:
    ~AbstractAnnotationItemDelegate() override;

public:
    /**
     * This pure abstract function must be reimplemented to provide custom rendering.
     * Use the painter and style option to render the annotation information for the line
     * specified by the arguments @p model and @p line.
     * @param painter the painter object
     * @param option the option object with the info needed for styling
     * @param model the annotation model providing the annotation information
     * @param line index of the real line the annotation information should be painted for
     *
     * Reimplement this in line with sizeHint().
     */
    virtual void paint(QPainter *painter, const KTextEditor::StyleOptionAnnotationItem &option, KTextEditor::AnnotationModel *model, int line) const = 0;
    /**
     * This pure abstract function must be reimplemented to provide custom rendering.
     * Use the style option to calculate the best size for the annotation information
     * for the line specified by the arguments @p model and @p line.
     * This should be the size for the display for a single displayed content line,
     * i.e. with no line wrapping or consecutive multiple annotation item of the same group assumed.
     *
     * @note If AnnotationViewInterface::uniformAnnotationItemSizes() is @c true for the view
     * this delegate is used by, it is assumed that the returned value is the same for
     * any line.
     *
     * @param option the option object with the info needed for styling
     * @param model the annotation model providing the annotation information
     * @param line index of the real line the annotation information should be painted for
     * @return best size for the annotation information
     *
     * Reimplement this in line with paint().
     */
    virtual QSize sizeHint(const KTextEditor::StyleOptionAnnotationItem &option, KTextEditor::AnnotationModel *model, int line) const = 0;
    /**
     * Whenever a help event occurs, this function is called with the event view option
     * and @p model and @p line specifying the item where the event occurs.
     * This pure abstract function must be reimplemented to provide custom tooltips.
     * @param event the help event
     * @param view the view for which the help event is requested
     * @param option the style option object with the info needed for styling, including the rect of the annotation
     * @param model the annotation model providing the annotation information
     * @param line index of the real line the annotation information should be painted for
     * @return @c true if the event could be handled (implies that the data obtained from the model had the required role), @c false otherwise
     *
     * Reimplement this in line with hideTooltip().
     */
    virtual bool helpEvent(QHelpEvent *event,
                           KTextEditor::View *view,
                           const KTextEditor::StyleOptionAnnotationItem &option,
                           KTextEditor::AnnotationModel *model,
                           int line) = 0;
    /**
     * This pure abstract function must be reimplemented to provide custom tooltips.
     * It is called whenever a possible still shown tooltip no longer is valid,
     * e.g. if the annotations have been hidden.
     * @param view the view for which the tooltip was requested
     *
     * Reimplement this in line with helpEvent().
     */
    virtual void hideTooltip(KTextEditor::View *view) = 0;

Q_SIGNALS:
    /**
     * This signal must be emitted when the sizeHint() for @p model and @p line changed.
     * The view automatically connects to this signal and relayouts as necessary.
     * If AnnotationViewInterface::uniformAnnotationItemSizes is set on the view,
     * it is sufficient to emit sizeHintChanged only for one line.
     * @param model the annotation model providing the annotation information
     * @param line index of the real line the annotation information should be painted for
     */
    void sizeHintChanged(KTextEditor::AnnotationModel *model, int line);
};

}

#endif
