/*

    This file is part of the KDE project, module kfile.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <chris@kde.org>
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONDIALOG_H
#define KICONDIALOG_H

#include "kiconwidgets_export.h"

#include <QDialog>
#include <QPushButton>
#include <memory>

#include <kiconloader.h>

/**
 * @class KIconDialog kicondialog.h KIconDialog
 *
 * Dialog for interactive selection of icons. Use the function
 * getIcon() to let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KICONWIDGETS_EXPORT KIconDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructs an icon selection dialog using the global icon loader.
     *
     * @param parent The parent widget.
     */
    explicit KIconDialog(QWidget *parent = nullptr);

    /**
     * Destructs the dialog.
     */
    ~KIconDialog() override;

    /**
     * Sets a strict icon size policy for allowed icons.
     *
     * @param policy When true, only icons of the specified group's
     * size in getIcon() are shown.
     * When false, icons not available at the desired group's size will
     * also be selectable.
     */
    void setStrictIconSize(bool policy);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;

    /**
     * Sets the location of the custom icon directory. Only local directory
     * paths are allowed.
     */
    void setCustomLocation(const QString &location);

    /**
     * Sets the size of the icons to be shown / selected.
     * @see KIconLoader::StdSizes
     * @see iconSize
     */
    void setIconSize(int size);
    /**
     * Returns the icon size set via setIconSize() or 0, if the default
     * icon size will be used.
     */
    int iconSize() const;

    /**
     * Sets the icon that is initially selected in the dialog.
     *
     * @note Changing this after the dialog has been shown has no effect.
     * @note If the given icon cannot be found in the current context,
     * no icon will be selected.
     * @param iconName The name of the icon to select
     * @since 5.89
     */
    void setSelectedIcon(const QString &iconName);

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon(), as well as two additional parameters to lock
     * the choice between system and user directories and to lock the
     * custom icon directory itself.
     */
    void setup(KIconLoader::Group group,
               KIconLoader::Context context = KIconLoader::Application,
               bool strictIconSize = false,
               int iconSize = 0,
               bool user = false,
               bool lockUser = false,
               bool lockCustomDir = false);

    /**
     * exec()utes this modal dialog and returns the name of the selected icon,
     * or QString() if the dialog was aborted.
     * @returns the name of the icon, suitable for loading with KIconLoader.
     * @see getIcon
     */
    QString openDialog();

    /**
     * show()s this dialog and emits a newIconName(const QString&) signal when
     * successful. QString() will be emitted if the dialog was aborted.
     */
    void showDialog();

    /**
     * Pops up the dialog an lets the user select an icon.
     *
     * @param group The icon group this icon is intended for. Providing the
     * group shows the icons in the dialog with the same appearance as when
     * used outside the dialog.
     * @param context The initial icon context. Initially, the icons having
     * this context are shown in the dialog. The user can change this.
     * @param strictIconSize When true, only icons of the specified group's size
     * are shown, otherwise icon not available in the desired group's size
     * will also be selectable.
     * @param iconSize the size of the icons -- the default of the icon group
     *        if set to 0
     * @param user Begin with the "user icons" instead of "system icons".
     * @param parent The parent widget of the dialog.
     * @param title The title to use for the dialog.
     * @return The name of the icon, suitable for loading with KIconLoader.
     */
    static QString getIcon(KIconLoader::Group group = KIconLoader::Desktop,
                           KIconLoader::Context context = KIconLoader::Application,
                           bool strictIconSize = false,
                           int iconSize = 0,
                           bool user = false,
                           QWidget *parent = nullptr,
                           const QString &title = QString());

Q_SIGNALS:
    void newIconName(const QString &iconName);

protected:
    void showEvent(QShowEvent *event) override;

protected Q_SLOTS:
    void slotOk();

private:
    std::unique_ptr<class KIconDialogPrivate> const d;

    friend class KIconDialogPrivate;

    Q_DISABLE_COPY(KIconDialog)
};

#endif // KICONDIALOG_H
