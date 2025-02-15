/*
    SPDX-FileCopyrightText: 2000-2003 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Daniel Molkentin <molkentin@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KTIP_H
#define KTIP_H

#include <QDialog>
#include <QStringList>
#include <memory>

#include <kconfigwidgets_export.h>

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 83)

/**
 * A database for tips-of-the-day.
 *
 * This class provides convenient access to a database containing
 * tips of the day. The database is stored in a XML file and parsed
 * when a KTipDatabase object is created.
 *
 * Once the file is read in, you can access the tips to display
 * them in the tip of the day dialog.
 *
 * The state of the tipdialog is saved to the applications's config file
 * in the group "TipOfDay" with a bool entry "RunOnStart". Check this value
 * if you want to allow the user to enable/disable the tipdialog in the
 * application's configuration dialog.
 *
 * \image html ktip.png "KDE Tip-of-the-Day Dialog"
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 *
 * @deprecated Since 5.83, write the information to the handbook instead
 */
class KCONFIGWIDGETS_EXPORT KTipDatabase
{
public:
    /**
     * This constructor reads in the tips from a file with the given name. If
     * no name is given, a file called 'application-name/tips' will be loaded.
     *
     * @param tipFile The absolute path to the tips file.
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    explicit KTipDatabase(const QString &tipFile = QString());

    /**
     * This constructor takes a list of files that will be merged. This constructor
     * essentially behaves like the one above. It returns when tipFiles is empty.
     *
     * @param tipFiles A list of absolute paths to the tips file
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    explicit KTipDatabase(const QStringList &tipFiles);

    ~KTipDatabase();

    /**
     * Returns the current tip.
     */
    QString tip() const;

    /**
     * The next tip will become the current one.
     */
    void nextTip();

    /**
     * The previous tip will become the current one.
     */
    void prevTip();

private:
    std::unique_ptr<class KTipDatabasePrivate> const d;

    Q_DISABLE_COPY(KTipDatabase)
};

/**
 * @class KTipDialog ktipdialog.h KTipDialog
 *
 * A Tip-of-the-Day dialog.
 *
 * This dialog class presents a tip-of-the-day.
 *
 * The tips will be looked up for translation using gettext
 * with KLocalizedString::applicationDomain() as domain.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 */
class KCONFIGWIDGETS_EXPORT KTipDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Construct a tip dialog.
     *
     * @param database TipDatabase that should be used by the TipDialog. The KTipDialog
     *                 will take ownership of the database, including deleting it.
     * @param parent Parent widget of TipDialog.
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    explicit KTipDialog(KTipDatabase *database, QWidget *parent = nullptr);

    /**
     * Destroys the tip dialog.
     */
    ~KTipDialog() override;

    /**
     * Shows a tip.
     *
     * This static method is all that is needed to add a tip-of-the-day
     * dialog to an application. It will pop up the dialog, unless the
     * user has asked that the dialog does not pop up on startup.
     *
     * Note that you probably want an item in the help menu calling
     * this method with force=true.
     *
     * @param parent Parent widget of TipDialog.
     * @param tipFile The name of the tip file. It has be relative to the GenericDataLocation
     *                resource of QStandardPaths.
     * @param force If true, the dialog is show, even when the users
     *              disabled it.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    static void showTip(QWidget *parent, const QString &tipFile = QString(), bool force = false);

    /**
     * Shows a tip
     *
     * This method behaves essentially as the one above, but expects a list of tips
     *
     * @param parent Parent widget of TipDialog.
     * @param tipFiles A List of tip files. Each has be relative to the GenericDataLocation
     *                resource of QStandardPaths.
     * @param force If true, the dialog is show, even when the users
     *              disabled it.
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    static void showMultiTip(QWidget *parent, const QStringList &tipFiles, bool force = false);

    /**
     * Shows a tip.
     *
     * This methods calls showTip() with the applications main window as parent.
     *
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    static void showTip(const QString &tipFile = QString(), bool force = false);

    /**
     * Toggles the start behavior.
     *
     * Normally, the user can disable the display of the tip in the dialog.
     * This is just a way to change this setting from outside.
     * @deprecated Since 5.83, write the information to the handbook instead
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 83, "write the information to the handbook instead")
    static void setShowOnStart(bool show);

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    std::unique_ptr<class KTipDialogPrivate> const d;

    Q_DISABLE_COPY(KTipDialog)
};

#endif
#endif
