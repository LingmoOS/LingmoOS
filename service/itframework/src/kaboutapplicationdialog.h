/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>

    Parts of this class have been take from the KAboutApplication class, which was:
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABOUT_APPLICATION_DIALOG_H
#define KABOUT_APPLICATION_DIALOG_H

#include <QDialog>
#include <memory>

#include <kxmlgui_export.h>

class KAboutData;

/**
 * @class KAboutApplicationDialog kaboutapplicationdialog.h KAboutApplicationDialog
 *
 * @short Standard "About Application" dialog box.
 *
 * This class provides the standard "About Application" dialog box
 * that is used by KHelpMenu. It uses the information of the global
 * KAboutData that is specified at the start of your program in
 * main(). Normally you should not use this class directly but rather
 * the KHelpMenu class or even better just subclass your toplevel
 * window from KMainWindow. If you do the latter, the help menu and
 * thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * \image html kaboutapplicationdialog.png "KAboutApplicationDialog"
 *
 * @author Urs Wolfer uwolfer @ kde.org
 */

class KXMLGUI_EXPORT KAboutApplicationDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Defines some options which can be applied to the about dialog
     *
     * @see Options
     * @since 4.4
     */
    enum Option {
        NoOptions = 0x0, ///< No options, show the standard about dialog
        HideTranslators = 0x1, ///< Don't show the translators tab
        HideLibraries = 0x2, /**< Don't show the libraries tab @since 5.77
                              *
                              * Since 5.87 Don't show the components tab (which replaced the libraries tab)
                              */
#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 77)
        HideKdeVersion KXMLGUI_ENUMERATOR_DEPRECATED_VERSION_BELATED(5, 83, 5, 77, "Use HideLibraries") =
            HideLibraries /**<
                           * Before 5.13: Don't show the KDE Frameworks version next to the application name and version
                           *
                           * Since 5.13: Don't show the Qt and KDE Frameworks libraries in the versions tab
                           *
                           * Since 5.30 Don't show the Qt and KDE Frameworks libraries in the libraries tab
                           *
                           * Since 5.75 Don't show the libraries tab
                           *
                           * @deprecated Since 5.77, use @c HideLibraries instead.
                           */
#endif
    };
    /**
     * Stores a combination of #Option values.
     */
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /**
     * Constructor. Creates a fully featured "About Application" dialog box.
     *
     * @param aboutData A KAboutData object which data
     *        will be used for filling the dialog.
     * @param opts Additional options that can be applied, such as hiding the KDE version
     *             or the translators tab.
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     *
     * @since 4.4
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, Options opts, QWidget *parent = nullptr);

    /**
     * Constructor. Creates a fully featured "About Application" dialog box.
     *
     * @param aboutData A KAboutData object which data
     *        will be used for filling the dialog.
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, QWidget *parent = nullptr);

    ~KAboutApplicationDialog() override;

private:
    std::unique_ptr<class KAboutApplicationDialogPrivate> const d;

    Q_DISABLE_COPY(KAboutApplicationDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAboutApplicationDialog::Options)

#endif
