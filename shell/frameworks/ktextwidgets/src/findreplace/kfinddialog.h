/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFINDDIALOG_H
#define KFINDDIALOG_H

#include "ktextwidgets_export.h"

#include <QDialog>
#include <memory>

class KFindDialogPrivate;

/**
 * @class KFindDialog kfinddialog.h <KFindDialog>
 *
 * @brief A generic "find" dialog.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * \b Detail:
 *
 * This widget inherits from KDialog and implements
 * the following additional functionalities:  a find string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * \b Example:
 *
 * To use the basic modal find dialog, and then run the search:
 *
 * \code
 *  KFindDialog dlg(....)
 *  if (dlg.exec() != QDialog::Accepted)
 *      return;
 *
 *  // proceed with KFind from here
 * \endcode
 *
 * To create a non-modal find dialog:
 * \code
 *   if (m_findDialog) {
 *     m_findDialog->activateWindow();
 *   } else {
 *     m_findDialog = new KFindDialog(...);
 *     connect(m_findDialog, &KFindDialog::okClicked, this, [this] {
 *         m_findDialog->close();
 *         delete m_find;
 *         m_find = new KFind(m_findDialog->pattern(), m_findDialog->options(), this);
 *         // ... see KFind documentation for what else should be done here
 *     });
 *   }
 * \endcode
 * Don't forget to delete and reset m_findDialog when closed.
 * (But do NOT delete your KFind object at that point, it's needed for "Find Next".)
 *
 * To use your own extensions: see findExtension().
 *
 * \image html kfinddialog.png "KFindDialog Widget"
 */
class KTEXTWIDGETS_EXPORT KFindDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Construct a modal find dialog
     *
     * @param parent The parent object of this widget.
     * @param options A bitfield of the Options to be checked.
     * @param findStrings The find history, see findHistory()
     * @param hasSelection Whether a selection exists
     */
    explicit KFindDialog(QWidget *parent = nullptr,
                         long options = 0,
                         const QStringList &findStrings = QStringList(),
                         bool hasSelection = false,
                         bool replaceDialog = false);

    /**
     * Destructor.
     */
    ~KFindDialog() override;

    /**
     * Provide the list of @p strings to be displayed as the history
     * of find strings. @p strings might get truncated if it is
     * too long.
     *
     * @param history The find history.
     * @see findHistory
     */
    void setFindHistory(const QStringList &history);

    /**
     * Returns the list of history items.
     *
     * @see setFindHistory
     */
    QStringList findHistory() const;

    /**
     * Enable/disable the 'search in selection' option, depending
     * on whether there actually is a selection.
     *
     * @param hasSelection true if a selection exists
     */
    void setHasSelection(bool hasSelection);

    /**
     * Hide/show the 'from cursor' option, depending
     * on whether the application implements a cursor.
     *
     * @param hasCursor true if the application features a cursor
     * This is assumed to be the case by default.
     */
    void setHasCursor(bool hasCursor);

    /**
     * Enable/disable the 'Find backwards' option, depending
     * on whether the application supports it.
     *
     * @param supports true if the application supports backwards find
     * This is assumed to be the case by default.
     */
    void setSupportsBackwardsFind(bool supports);

    /**
     * Enable/disable the 'Case sensitive' option, depending
     * on whether the application supports it.
     *
     * @param supports true if the application supports case sensitive find
     * This is assumed to be the case by default.
     */
    void setSupportsCaseSensitiveFind(bool supports);

    /**
     * Enable/disable the 'Whole words only' option, depending
     * on whether the application supports it.
     *
     * @param supports true if the application supports whole words only find
     * This is assumed to be the case by default.
     */
    void setSupportsWholeWordsFind(bool supports);

    /**
     * Enable/disable the 'Regular expression' option, depending
     * on whether the application supports it.
     *
     * @param supports true if the application supports regular expression find
     * This is assumed to be the case by default.
     */
    void setSupportsRegularExpressionFind(bool supports);

    /**
     * Set the options which are checked.
     *
     * @param options The setting of the Options.
     *
     * @see options()
     * @see KFind::Options
     */
    void setOptions(long options);

    /**
     * Returns the state of the options. Disabled options may be returned in
     * an indeterminate state.
     *
     * @see setOptions()
     * @see KFind::Options
     */
    long options() const;

    /**
     * Returns the pattern to find.
     */
    QString pattern() const;

    /**
     * Sets the pattern to find
     */
    void setPattern(const QString &pattern);

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately below the regular expression support
     * widgets for the pattern string.
     */
    QWidget *findExtension() const;

Q_SIGNALS:
    /**
     * This signal is sent whenever one of the option checkboxes is toggled.
     * Call options() to get the new state of the checkboxes.
     */
    void optionsChanged();

    /**
     * This signal is sent when the user clicks on Ok button.
     */
    void okClicked();

    /**
     * This signal is sent when the user clicks on Cancel button.
     */
    void cancelClicked();

protected:
    void showEvent(QShowEvent *) override;

protected:
    KTEXTWIDGETS_NO_EXPORT explicit KFindDialog(KFindDialogPrivate &dd,
                                                QWidget *parent = nullptr,
                                                long options = 0,
                                                const QStringList &findStrings = QStringList(),
                                                bool hasSelection = false,
                                                bool replaceDialog = false);

protected:
    std::unique_ptr<class KFindDialogPrivate> const d_ptr;

private:
    Q_DECLARE_PRIVATE(KFindDialog)
};

#endif // KFINDDIALOG_H
