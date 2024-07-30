/*
    SPDX-FileCopyrightText: 2004, 2010 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_TEMPLATE_HANDLER_H_
#define _KATE_TEMPLATE_HANDLER_H_

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <katescript.h>
#include <ktexteditor/cursor.h>

class KateUndoManager;

namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
class MovingCursor;
class MovingRange;
class View;
}

/**
 * \brief Inserts a template and offers advanced snippet features, like navigation and mirroring.
 *
 * For each template inserted a new KateTemplateHandler will be created.
 *
 * The handler has the following features:
 *
 * \li It inserts the template string into the document at the requested position.
 * \li When the template contains at least one variable, the cursor will be placed
 *     at the start of the first variable and its range gets selected.
 * \li When more than one variable exists,TAB and SHIFT TAB can be used to navigate
 *     to the next/previous variable.
 * \li When a variable occurs more than once in the template, edits to any of the
 *     occurrences will be mirroed to the other ones.
 * \li When ESC is pressed, the template handler closes.
 * \li When ALT + RETURN is pressed and a \c ${cursor} variable
 *     exists in the template,the cursor will be placed there. Else the cursor will
 *     be placed at the end of the template.
 *
 * \author Milian Wolff <mail@milianw.de>
 */

class KateTemplateHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * Setup the template handler, insert the template string.
     *
     * NOTE: The handler deletes itself when required, you do not need to
     *       keep track of it.
     */
    KateTemplateHandler(KTextEditor::ViewPrivate *view,
                        KTextEditor::Cursor position,
                        const QString &templateString,
                        const QString &script,
                        KateUndoManager *undoManager);

    ~KateTemplateHandler() override;

protected:
    /**
     * \brief Provide keyboard interaction for the template handler.
     *
     * The event filter handles the following shortcuts:
     *
     * TAB: jump to next editable (i.e. not mirrored) range.
     *      NOTE: this prevents indenting via TAB.
     * SHIFT + TAB: jump to previous editable (i.e. not mirrored) range.
     *      NOTE: this prevents un-indenting via SHIFT + TAB.
     * ESC: terminate template handler (only when no completion is active).
     * ALT + RETURN: accept template and jump to the end-cursor.
     *               if %{cursor} was given in the template, that will be the
     *               end-cursor.
     *               else just jump to the end of the inserted text.
     */
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    /**
     * Inserts the @p text template at @p position and performs
     * all necessary initializations, such as populating default values
     * and placing the cursor.
     */
    void initializeTemplate();

    /**
     * Parse @p templateText and populate m_fields.
     */
    void parseFields(const QString &templateText);

    /**
     * Set necessary attributes (esp. background colour) on all moving
     * ranges for the fields in m_fields.
     */
    void setupFieldRanges();

    /**
     * Evaluate default values for all fields in m_fields and
     * store them in the fields. This updates the @property defaultValue property
     * of the TemplateField instances in m_fields from the raw, user-entered
     * default value to its evaluated equivalent (e.g. "func()" -> result of function call)
     *
     * @sa TemplateField
     */
    void setupDefaultValues();

    /**
     * Install an event filter on the filter proxy of \p view for
     * navigation between the ranges and terminating the KateTemplateHandler.
     *
     * \see eventFilter()
     */
    void setupEventHandler(KTextEditor::View *view);

    /**
     * Jumps to the previous editable range. If there is none, wrap and jump to the first range.
     *
     * \see jumpToNextRange()
     */
    void jumpToPreviousRange();

    /**
     * Jumps to the next editable range. If there is none, wrap and jump to the last range.
     *
     * \see jumpToPreviousRange()
     */
    void jumpToNextRange();

    /**
     * Helper function for jumpTo{Next,Previous}
     * if initial is set to true, assumes the cursor is before the snippet
     * and selects the first field
     */
    void jump(int by, bool initial = false);

    /**
     * Jumps to the final cursor position. This is either \p m_finalCursorPosition, or
     * if that is not set, the end of \p m_templateRange.
     */
    void jumpToFinalCursorPosition();

    /**
     * Go through all template fields and decide if their moving ranges expand
     * when edited at the corners. Expansion is turned off if two fields are
     * directly adjacent to avoid overlaps when characters are inserted between
     * them.
     */
    void updateRangeBehaviours();

    /**
     * Sort all template fields in m_fields by tab order, which means,
     * by range; except for ${cursor} which is always sorted last.
     */
    void sortFields();

private Q_SLOTS:
    /**
     * Saves the range of the inserted template. This is required since
     * tabs could get expanded on insert. While we are at it, we can
     * use it to auto-indent the code after insert.
     */
    void slotTemplateInserted(KTextEditor::Document *document, KTextEditor::Range range);

    /**
     * Install event filter on new views.
     */
    void slotViewCreated(KTextEditor::Document *document, KTextEditor::View *view);

    /**
     * Update content of all dependent fields, i.e. mirror or script fields.
     */
    void updateDependentFields(KTextEditor::Document *document, KTextEditor::Range oldRange);

public:
    KTextEditor::ViewPrivate *view() const;
    KTextEditor::DocumentPrivate *doc() const;

private:
    /// The view we operate on
    KTextEditor::ViewPrivate *m_view;
    /// The undo manager associated with our document
    KateUndoManager *const m_undoManager;

    // Describes a single template field, e.g. ${foo}.
    struct TemplateField {
        // up-to-date range for the field
        std::shared_ptr<KTextEditor::MovingRange> range;
        // contents of the field, i.e. identifier or function to call
        QString identifier;
        // default value, if applicable; else empty
        QString defaultValue;
        enum Kind {
            Invalid, // not an actual field
            Editable, // normal, user-editable field (green by default) [non-dependent field]
            Mirror, // field mirroring contents of another field [dependent field]
            FunctionCall, // field containing the up-to-date result of a function call [dependent field]
            FinalCursorPosition // field marking the final cursor position
        };
        Kind kind = Invalid;
        // true if this field was edited by the user before
        bool touched = false;
        bool operator==(const TemplateField &other) const
        {
            return range == other.range;
        }
    };
    // List of all template fields in the inserted snippet. @see sortFields()
    QList<TemplateField> m_fields;

    // Get the template field which contains @p range.
    const TemplateField fieldForRange(KTextEditor::Range range) const;

    /// Construct a map of master fields and their current value, for use in scripts.
    KateScript::FieldMap fieldMap() const;

    /// A range that occupies the whole range of the inserted template.
    /// When the an edit happens outside it, the template handler gets closed.
    std::shared_ptr<KTextEditor::MovingRange> m_wholeTemplateRange;

    /// Set to true when currently updating dependent fields, to prevent recursion.
    bool m_internalEdit;

    /// template script (i.e. javascript stuff), which can be used by the current template
    KateScript m_templateScript;
};

#endif
