/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2007 Krzysztof Lichota <lichota@mimuw.edu.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KSWITCHLANGUAGEDIALOG_H_
#define _KSWITCHLANGUAGEDIALOG_H_

#include <kxmlgui_export.h>

#include <QDialog>

namespace KDEPrivate
{
KXMLGUI_EXPORT void setApplicationSpecificLanguage(const QByteArray &languageCode);
KXMLGUI_EXPORT void initializeLanguages();

class KSwitchLanguageDialogPrivate;

/**
 * @short Standard "switch application language" dialog box.
 *
 * This class provides "switch application language" dialog box that is used
 * in KHelpMenu
 *
 * @author Krzysztof Lichota (lichota@mimuw.edu.pl)
 * @internal
 */

class KSwitchLanguageDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor. Creates a fully featured "Switch application language" dialog box.
     * Note that this dialog is made modeless in the KHelpMenu class so
     * the users may expect a modeless dialog.
     *
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     * @param name Internal name of the widget. This name in not used in the
     *        caption.
     * @param modal If false, this widget will be modeless and must be
     *        made visible using QWidget::show(). Otherwise it will be
     *        modal and must be made visible using QWidget::exec()
     */
    explicit KSwitchLanguageDialog(QWidget *parent = nullptr);

    ~KSwitchLanguageDialog() override;

protected Q_SLOTS:
    /**
     * Activated when the Ok button has been clicked.
     */
    virtual void slotOk();
    void slotDefault();

    /**
        Called when one of language buttons changes state.
    */
    virtual void languageOnButtonChanged(const QString &);

    /**
        Called to add one language button to dialog.
    */
    virtual void slotAddLanguageButton();

    /**
        Called when "Remove" language button is clicked.
    */
    virtual void removeButtonClicked();

private:
    KSwitchLanguageDialogPrivate *const d;

    friend class KSwitchLanguageDialogPrivate;
};

}

#endif
