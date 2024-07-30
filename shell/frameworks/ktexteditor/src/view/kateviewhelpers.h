/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KATE_VIEW_HELPERS_H
#define KATE_VIEW_HELPERS_H

#include <KActionMenu>
#include <KLineEdit>
#include <KSelectAction>

#include <QColor>
#include <QHash>
#include <QLayout>
#include <QPixmap>
#include <QPointer>
#include <QScrollBar>
#include <QTimer>

#include "katetextline.h"
#include <ktexteditor/cursor.h>
#include <ktexteditor/message.h>

namespace KTextEditor
{
class ViewPrivate;
class DocumentPrivate;
class Command;
class AnnotationModel;
class MovingRange;
class AbstractAnnotationItemDelegate;
class StyleOptionAnnotationItem;
}

class KateViewInternal;
class KateTextLayout;

#define MAXFOLDINGCOLORS 16

class KateLineInfo;
class KateTextPreview;

namespace Kate
{
class TextRange;
}

class QTimer;
class QVBoxLayout;
class QStackedWidget;

/**
 * Class to layout KTextEditor::Message%s in KateView. Only the floating
 * positions TopInView, CenterInView, and BottomInView are supported.
 * AboveView and BelowView are not supported and ASSERT.
 */
class KateMessageLayout : public QLayout
{
public:
    explicit KateMessageLayout(QWidget *parent);
    ~KateMessageLayout() override;

    void addWidget(QWidget *widget, KTextEditor::Message::MessagePosition pos);
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

    void add(QLayoutItem *item, KTextEditor::Message::MessagePosition pos);

private:
    void addItem(QLayoutItem *item) override; // never called publically

    struct ItemWrapper {
        ItemWrapper() = default;
        ItemWrapper(QLayoutItem *i, KTextEditor::Message::MessagePosition pos)
            : item(i)
            , position(pos)
        {
        }

        QLayoutItem *item = nullptr;
        KTextEditor::Message::MessagePosition position = KTextEditor::Message::AboveView;
    };

    QList<ItemWrapper> m_items;
};

/**
 * This class is required because QScrollBar's sliderMoved() signal is
 * really supposed to be a sliderDragged() signal... so this way we can capture
 * MMB slider moves as well
 *
 * Also, it adds some useful indicators on the scrollbar.
 */
class KateScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    KateScrollBar(Qt::Orientation orientation, class KateViewInternal *parent);
    ~KateScrollBar() override;
    QSize sizeHint() const override;

    void showEvent(QShowEvent *event) override;

    inline bool showMarks() const
    {
        return m_showMarks;
    }
    inline void setShowMarks(bool b)
    {
        m_showMarks = b;
        update();
    }

    inline bool showMiniMap() const
    {
        return m_showMiniMap;
    }
    void setShowMiniMap(bool b);

    inline bool miniMapAll() const
    {
        return m_miniMapAll;
    }
    inline void setMiniMapAll(bool b)
    {
        m_miniMapAll = b;
        updateGeometry();
        update();
    }

    inline bool miniMapWidth() const
    {
        return m_miniMapWidth;
    }
    inline void setMiniMapWidth(int width)
    {
        m_miniMapWidth = width;
        updateGeometry();
        update();
    }

    inline void queuePixmapUpdate()
    {
        m_updateTimer.start();
    }

Q_SIGNALS:
    void sliderMMBMoved(int value);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
    void sliderChange(SliderChange change) override;

protected Q_SLOTS:
    void sliderMaybeMoved(int value);
    void marksChanged();

public Q_SLOTS:
    void updatePixmap();

private Q_SLOTS:
    void showTextPreview();

private:
    void showTextPreviewDelayed();
    void hideTextPreview();

    void redrawMarks();
    void recomputeMarksPositions();

    void miniMapPaintEvent(QPaintEvent *e);
    void normalPaintEvent(QPaintEvent *e);

    int minimapYToStdY(int y);

    struct ColumnRangeWithColor {
        int penIndex = -1;
        int startColumn;
        int endColumn;
    };
    void getCharColorRanges(const QList<Kate::TextLine::Attribute> &attributes,
                            const QList<Kate::TextRange *> &decorations,
                            const QString &text,
                            QList<KateScrollBar::ColumnRangeWithColor> &ranges,
                            QVarLengthArray<std::pair<QRgb, QPen>, 20> &penCache);

    bool m_middleMouseDown;
    bool m_leftMouseDown;

    KTextEditor::ViewPrivate *m_view;
    KTextEditor::DocumentPrivate *m_doc;
    class KateViewInternal *m_viewInternal;
    QPointer<KateTextPreview> m_textPreview;
    QTimer m_delayTextPreviewTimer;

    QHash<int, QColor> m_lines;

    bool m_showMarks;
    bool m_showMiniMap;
    bool m_miniMapAll;
    bool m_needsUpdateOnShow;
    int m_miniMapWidth;

    QPixmap m_pixmap;
    int m_grooveHeight;
    QRect m_stdGroveRect;
    QRect m_mapGroveRect;
    QRect m_sliderRect;
    QTimer m_updateTimer;
    QPoint m_toolTipPos;

    // lists of lines added/removed recently to avoid scrollbar flickering
    QHash<int, int> m_linesAdded;

    static const unsigned char characterOpacity[256];
};

class KateIconBorder : public QWidget
{
    Q_OBJECT

public:
    KateIconBorder(KateViewInternal *internalView, QWidget *parent);
    ~KateIconBorder() override;
    // VERY IMPORTANT ;)
    QSize sizeHint() const override;

    void updateFont();
    int lineNumberWidth() const;

    void setIconBorderOn(bool enable);
    void setLineNumbersOn(bool enable);
    void setRelLineNumbersOn(bool enable);
    void setAnnotationBorderOn(bool enable);
    void setDynWrapIndicators(int state);
    int dynWrapIndicators() const
    {
        return m_dynWrapIndicators;
    }
    bool dynWrapIndicatorsOn() const
    {
        return m_dynWrapIndicatorsOn;
    }
    void setFoldingMarkersOn(bool enable);
    void toggleIconBorder()
    {
        setIconBorderOn(!iconBorderOn());
    }
    void toggleLineNumbers()
    {
        setLineNumbersOn(!lineNumbersOn());
    }
    void toggleFoldingMarkers()
    {
        setFoldingMarkersOn(!foldingMarkersOn());
    }
    inline bool iconBorderOn() const
    {
        return m_iconBorderOn;
    }
    inline bool lineNumbersOn() const
    {
        return m_lineNumbersOn;
    }
    inline bool viRelNumbersOn() const
    {
        return m_relLineNumbersOn;
    }
    inline bool foldingMarkersOn() const
    {
        return m_foldingMarkersOn;
    }
    inline bool annotationBorderOn() const
    {
        return m_annotationBorderOn;
    }

    void updateForCursorLineChange();

    enum BorderArea { None, LineNumbers, IconBorder, FoldingMarkers, AnnotationBorder, ModificationBorder };
    BorderArea positionToArea(const QPoint &) const;

    KTextEditor::AbstractAnnotationItemDelegate *annotationItemDelegate() const;
    void setAnnotationItemDelegate(KTextEditor::AbstractAnnotationItemDelegate *delegate);
    inline bool uniformAnnotationItemSizes() const
    {
        return m_hasUniformAnnotationItemSizes;
    }
    inline void setAnnotationUniformItemSizes(bool enable)
    {
        m_hasUniformAnnotationItemSizes = enable;
    }

public Q_SLOTS:
    void updateAnnotationBorderWidth();
    void updateAnnotationLine(int line);
    void annotationModelChanged(KTextEditor::AnnotationModel *oldmodel, KTextEditor::AnnotationModel *newmodel);
    void displayRangeChanged();

private:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void paintBorder(int x, int y, int width, int height);

    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

    void enterEvent(QEnterEvent *e) override;

    void showMarkMenu(uint line, const QPoint &pos);

    void hideAnnotationTooltip();
    void removeAnnotationHovering();
    void showAnnotationMenu(int line, const QPoint &pos);
    void calcAnnotationBorderWidth();

    void initStyleOption(KTextEditor::StyleOptionAnnotationItem *styleOption) const;
    void setStyleOptionLineData(KTextEditor::StyleOptionAnnotationItem *styleOption,
                                int y,
                                int realLine,
                                const KTextEditor::AnnotationModel *model,
                                const QString &annotationGroupIdentifier) const;
    QRect annotationLineRectInView(int line) const;

private:
    KTextEditor::ViewPrivate *m_view;
    KTextEditor::DocumentPrivate *m_doc;
    KateViewInternal *m_viewInternal;

    bool m_iconBorderOn : 1;
    bool m_lineNumbersOn : 1;
    bool m_relLineNumbersOn : 1;
    bool m_updateRelLineNumbers : 1;
    bool m_foldingMarkersOn : 1;
    bool m_dynWrapIndicatorsOn : 1;
    bool m_annotationBorderOn : 1;
    bool m_updatePositionToArea : 1;
    bool m_mouseOver = false;

    typedef QPair<int, KateIconBorder::BorderArea> AreaPosition;
    std::vector<AreaPosition> m_positionToArea;

    const int m_separatorWidth = 2;
    const int m_modAreaWidth = 3;
    qreal m_maxCharWidth = 0.0;
    int m_lineNumberAreaWidth = 0;
    int m_iconAreaWidth = 0;
    int m_foldingAreaWidth = 0;
    int m_annotationAreaWidth = 0;
    const QChar m_dynWrapIndicatorChar = QChar(0x21AA);
    int m_dynWrapIndicators = 0;
    int m_lastClickedLine = -1;

    KTextEditor::AbstractAnnotationItemDelegate *m_annotationItemDelegate;
    bool m_hasUniformAnnotationItemSizes = false;
    bool m_isDefaultAnnotationItemDelegate = true;

    QPointer<KateTextPreview> m_foldingPreview;
    KTextEditor::MovingRange *m_foldingRange = nullptr;
    int m_currentLine = -1;
    QTimer m_antiFlickerTimer;
    void highlightFoldingDelayed(int line);
    void hideFolding();

private Q_SLOTS:
    void highlightFolding();
    void handleDestroyedAnnotationItemDelegate();
    void delayedUpdateOfSizeWithRepaint();

private:
    QString m_hoveredAnnotationGroupIdentifier;
};

class KateViewEncodingAction : public KSelectAction
{
public:
    KateViewEncodingAction(KTextEditor::DocumentPrivate *_doc, KTextEditor::ViewPrivate *_view, const QString &text, QObject *parent, bool saveAsMode = false);

    bool setCurrentCodec(const QString &codec);

private:
    void init();
    void subActionTriggered(QAction *);

    KTextEditor::DocumentPrivate *doc;
    KTextEditor::ViewPrivate *view;
    QAction *currentSubAction;
    const bool m_saveAsMode;

private:
    void setEncoding(const QString &e);
    void slotAboutToShow();
};

class KateViewBar;

class KateViewBarWidget : public QWidget
{
    Q_OBJECT
    friend class KateViewBar;

public:
    explicit KateViewBarWidget(bool addCloseButton, QWidget *parent = nullptr);

    virtual void closed()
    {
    }

    /// returns the currently associated KateViewBar and 0, if it is not associated
    KateViewBar *viewBar()
    {
        return m_viewBar;
    }

protected:
    /**
     * @return widget that should be used to add controls to bar widget
     */
    QWidget *centralWidget()
    {
        return m_centralWidget;
    }

    /**
     * @return close button, if there
     */
    QToolButton *closeButton()
    {
        return m_closeButton;
    }

Q_SIGNALS:
    void hideMe();

    // for friend class KateViewBar
private:
    void setAssociatedViewBar(KateViewBar *bar)
    {
        m_viewBar = bar;
    }

private:
    QWidget *m_centralWidget = nullptr;
    KateViewBar *m_viewBar = nullptr; // 0-pointer, if not added to a view bar
    QToolButton *m_closeButton = nullptr;
};

class KateViewBar : public QWidget
{
    Q_OBJECT

public:
    KateViewBar(bool external, QWidget *parent, KTextEditor::ViewPrivate *view);

    /**
     * Adds a widget to this viewbar.
     * Widget is initially invisible, you should call showBarWidget, to show it.
     * Several widgets can be added to the bar, but only one can be visible
     */
    void addBarWidget(KateViewBarWidget *newBarWidget);

    /**
     * Removes a widget from this viewbar.
     * Removing a widget makes sense if it takes a lot of space vertically,
     * because we use a QStackedWidget to maintain the same height for all
     * widgets in the viewbar.
     */
    void removeBarWidget(KateViewBarWidget *barWidget);

    /**
     * @return if viewbar has widget @p barWidget
     */
    bool hasBarWidget(KateViewBarWidget *barWidget) const;

    /**
     * Shows barWidget that was previously added with addBarWidget.
     * @see hideCurrentBarWidget
     */
    void showBarWidget(KateViewBarWidget *barWidget);

    /**
     * Adds widget that will be always shown in the viewbar.
     * After adding permanent widget viewbar is immediately shown.
     * ViewBar with permanent widget won't hide itself
     * until permanent widget is removed.
     * OTOH showing/hiding regular barWidgets will work as usual
     * (they will be shown above permanent widget)
     *
     * If permanent widget already exists, asserts!
     */
    void addPermanentBarWidget(KateViewBarWidget *barWidget);

    /**
     * Removes permanent bar widget from viewbar.
     * If no other viewbar widgets are shown, viewbar gets hidden.
     *
     * barWidget is not deleted, caller must do it if it wishes
     */
    void removePermanentBarWidget(KateViewBarWidget *barWidget);

    /**
     * @return true if the a KateViewBar is visible*/
    bool barWidgetVisible() const;

public Q_SLOTS:
    /**
     * Hides currently shown bar widget
     */
    void hideCurrentBarWidget();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    /**
     * Shows or hides whole viewbar
     */
    void setViewBarVisible(bool visible);

    bool m_external;

private:
    KTextEditor::ViewPrivate *m_view;
    QStackedWidget *m_stack;
    KateViewBarWidget *m_permanentBarWidget;
    QVBoxLayout *m_layout;
};

class KateCommandLineBar : public KateViewBarWidget
{
public:
    explicit KateCommandLineBar(KTextEditor::ViewPrivate *view, QWidget *parent = nullptr);
    ~KateCommandLineBar() override;

    void setText(const QString &text, bool selected = true);
    void execute(const QString &text);

public:
    static void showHelpPage();

private:
    class KateCmdLineEdit *m_lineEdit;
};

class KateCmdLineEdit : public KLineEdit
{
    Q_OBJECT

public:
    KateCmdLineEdit(KateCommandLineBar *bar, KTextEditor::ViewPrivate *view);
    bool event(QEvent *e) override;

    void hideEvent(QHideEvent *e) override;

Q_SIGNALS:
    void hideRequested();

public Q_SLOTS:
    void slotReturnPressed(const QString &cmd);

private Q_SLOTS:
    void hideLineEdit();

protected:
    void focusInEvent(QFocusEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

private:
    /**
     * Parse an expression denoting a position in the document.
     * Return the position as an integer.
     * Examples of expressions are "10" (the 10th line),
     * "$" (the last line), "." (the current line),
     * "'a" (the mark 'a), "/foo/" (a forwards search for "foo"),
     * and "?bar?" (a backwards search for "bar").
     * @param string the expression to parse
     * @return the position, an integer
     */
    void fromHistory(bool up);
    QString helptext(const QPoint &) const;

    KTextEditor::ViewPrivate *m_view;
    KateCommandLineBar *m_bar;
    bool m_msgMode;
    QString m_oldText;
    uint m_histpos; ///< position in the history
    uint m_cmdend; ///< the point where a command ends in the text, if we have a valid one.
    KTextEditor::Command *m_command; ///< For completing flags/args and interactiveness
    class KateCmdLnWhatsThis *m_help;

    QTimer *m_hideTimer;
};

class KateViewSchemaAction : public KActionMenu
{
public:
    KateViewSchemaAction(const QString &text, QObject *parent)
        : KActionMenu(text, parent)
    {
        init();
        setPopupMode(QToolButton::InstantPopup);
    }

    void updateMenu(KTextEditor::ViewPrivate *view);

private:
    void init();

    QPointer<KTextEditor::ViewPrivate> m_view;
    QStringList names;
    QActionGroup *m_group;
    int last;

public:
    void slotAboutToShow();

private:
    void setSchema();
};

#endif
