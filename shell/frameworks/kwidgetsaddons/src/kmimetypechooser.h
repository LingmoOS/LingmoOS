/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001-2004 Anders Lund <anders@alweb.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KMIMETYPE_CHOOSER_H
#define KMIMETYPE_CHOOSER_H

#include <QDialog>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

/**
 * @class KMimeTypeChooser kmimetypechooser.h KMimeTypeChooser
 *
 * This widget provides a checkable list of all available MIME types, presented
 * as a treeview, with the MIME type comment and glob patterns as individual columns.
 * It allows users to edit a MIME type by launching a MIME type editor (if it's
 * available on the system).
 *
 * When the user clicks the OK button, a list of the selected MIME type names
 * and associated glob patterns can be retrieved respectively; those lists can
 * be used to populate a text line edit, or set a configuration entry... etc.
 *
 * @author Anders Lund (anders at alweb dk), jan 23, 2002
 */
class KWIDGETSADDONS_EXPORT KMimeTypeChooser : public QWidget
{
    Q_OBJECT

public:
    /**
     * Buttons and data for display.
     */
    enum Visuals {
        Comments = 1, ///< Show the MIME type comment (e.g. "HTML Document") in a column
        Patterns = 2, ///< Show the MIME types glob patterns (e.g. "*.html;*.htm") in a column
        EditButton = 4 ///< Show the "Edit" button, allowing to edit the selected type
    };
    /**
     * Create a new KMimeTypeChooser.
     *
     * @param text A plain text line to display above the list
     * @param selectedMimeTypes A list of MIME type names, these will be initially selected
     *        in the list (provided they exist)
     * @param visuals OR'd KMimetypeChooser::Visuals enum values to to set whether to
     *        show the MIME type comment and glob patterns columns and an Edit button,
     *        respectively, or not
     * @param defaultGroup The group (e.g. "text") to expand in the treeview when no groups
     *        are selected. If not provided, no group is expanded by default.
     *        If @p groupsToShow is provided and it doesn't include @p defaultGroup, this
     *        parameter is ignored
     * @param groupsToShow a list of MIME type groups to show. If empty, all groups are shown
     * @param parent The parent widget to use
     */
    explicit KMimeTypeChooser(const QString &text = QString(),
                              const QStringList &selectedMimeTypes = QStringList(),
                              const QString &defaultGroup = QString(),
                              const QStringList &groupsToShow = QStringList(),
                              int visuals = Comments | Patterns | EditButton,
                              QWidget *parent = nullptr);
    ~KMimeTypeChooser() override;

    /**
     * @return a list of all selected MIME type names
     */
    QStringList mimeTypes() const;
    /**
     * @return a list of the filename glob patterns associated with all selected MIME types
     */
    QStringList patterns() const;

private:
    std::unique_ptr<class KMimeTypeChooserPrivate> const d;
};

/**
 * @class KMimeTypeChooserDialog kmimetypechooser.h KMimeTypeChooserDialog
 *
 * @short A dialog to select MIME types from the list of available ones on the system
 *
 * This dialog embeds KMimeTypeChooser widget, presenting a checkable tree list of
 * MIME types, each with its associated icon, and optionally associated glob patterns
 * (displayed in a separate column); also an optional Edit button to launch a
 * MIME type editor to edit the selected MIME type.
 *
 * Here is an example, using the dialog to set the text of two line edits with the
 * list of MIME type names and glob patterns, respectively, of the MIME types that
 * the user has selected:
 *
 * @code
 *    QLineEdit *leMimetypes = new QLineEdit();
 *    QLineEdit *leGlobPatterns = new QLineEdit();
 *    [...]
 *    QString textLine = i18n("Select MIME types");
 *    QStringList mimeList = QStringList::split(QRegularExpression("\\s*;\\s*"), leMimetypes->text());
 *    KMimeTypeChooserDialog dlg(i18n("Select MIME Types"), textLine, mimeList, "text", this);
 *    if (dlg.exec() == QDialog::Accepted) {
 *        leMimetypes->setText(dlg.chooser()->mimeTypes().join(";"));
 *        leGlobPatterns->setText(dlg.chooser()->patterns().join(";"));
 *    }
 * @endcode
 *
 * \image html kmimetypechooserdialog.png "KMimeTypeChooserDialog in action"
 *
 * @author Anders Lund (anders at alweb dk) dec 19, 2001
 */
class KWIDGETSADDONS_EXPORT KMimeTypeChooserDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a KMimeTypeChooser dialog.
     *
     * @param title The title of the dialog
     * @param text A plain text line to display above the list
     * @param selectedMimeTypes A list of MIME type names, these will be initially selected
     *        in the list, provided they exist
     * @param visuals OR'd KMimetypeChooser::Visuals enum values to to set whether to
     *        show the MIME type comment and glob patterns columns and an Edit button,
     *        respectively, or not
     * @param defaultGroup The group (e.g. "text") to expand in the treeview when no
     *        groups are selected. If not provided, no group is expanded by default
     *        If @p groupsToShow is provided and it doesn't include @p defaultGroup,
     *        this parameter is ignored
     * @param groupsToShow A list of MIME type groups to show. If empty, all groups are shown
     * @param parent The parent widget to use
     */
    explicit KMimeTypeChooserDialog(const QString &title = QString(),
                                    const QString &text = QString(),
                                    const QStringList &selectedMimeTypes = QStringList(),
                                    const QString &defaultGroup = QString(),
                                    const QStringList &groupsToShow = QStringList(),
                                    int visuals = KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton,
                                    QWidget *parent = nullptr);

    /**
     * @overload
     */
    KMimeTypeChooserDialog(const QString &title,
                           const QString &text,
                           const QStringList &selectedMimeTypes,
                           const QString &defaultGroup,
                           QWidget *parent = nullptr);

    ~KMimeTypeChooserDialog() override;

    /**
     * @return a pointer to the KMimeTypeChooser widget
     */
    KMimeTypeChooser *chooser();

    QSize sizeHint() const override;

private:
    std::unique_ptr<class KMimeTypeChooserDialogPrivate> const d;
};
#endif // _KMIMETYPE_CHOOSER_H_
