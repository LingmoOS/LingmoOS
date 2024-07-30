// vi: ts=8 sts=4 sw=4
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Pietro Iglio <iglio@fub.it>
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2004, 2005 Andrew Coles <andrew_coles@yahoo.co.uk>
    SPDX-FileCopyrightText: 2006,2007 Olivier Goffart <ogoffart @ kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KNEWPASSWORDDIALOG_H
#define KNEWPASSWORDDIALOG_H

#include <KPassword>
#include <QDialog>
#include <memory>

#include <kwidgetsaddons_export.h>

class QWidget;

/**
 * @class KNewPasswordDialog knewpassworddialog.h KNewPasswordDialog
 *
 * @short A password input dialog.
 *
 * This dialog asks the user to enter a new password.
 *
 * The password has to be entered twice to check if the passwords
 * match. A hint about the strength of the entered password is also
 * shown.
 *
 * \section usage Usage Example
 * \subsection asynchronous Asynchronous
 *
 * \code
 *  KNewPasswordDialog *dlg = new KNewPasswordDialog( parent );
 *  dlg->setPrompt(i18n("Enter a password"));
 *  connect(dlg, &KNewPasswordDialog::newPassword, this, [this](const QString &pass) { setPassword(pass); });
 *  connect(dlg, &QDialog::rejected, this, [this]() { slotCancel(); });
 *  dlg->show();
 * \endcode
 *
 * \subsection synchronous Synchronous
 *
 * \code
 *  KNewPasswordDialog dlg(parent);
 *  dlg.setPrompt(i18n("Enter a password"));
 *  if(dlg.exec()) {
 *      setPassword(dlg.password());
 *  }
 * \endcode
 *
 * \image html knewpassworddialog.png "KNewPasswordDialog"
 *
 * @author Geert Jansen <jansen@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KWIDGETSADDONS_EXPORT KNewPasswordDialog : public QDialog
{
    Q_OBJECT

    /**
     * @since 6.0
     */
    Q_PROPERTY(KPassword::RevealMode revealPasswordMode READ revealPasswordMode WRITE setRevealPasswordMode)

public:
    /**
     * Constructs a password dialog.
     *
     * @param parent Passed to lower level constructor.
     */
    explicit KNewPasswordDialog(QWidget *parent = nullptr);

    /**
     * Destructs the password dialog.
     */
    ~KNewPasswordDialog() override;

    /**
     * Sets the password prompt.
     */
    void setPrompt(const QString &prompt);

    /**
     * Returns the password prompt.
     */
    QString prompt() const;

    /**
     * Sets the icon that appears next to the prompt in the dialog. The default icon represents a simple key.
     * @since 5.63
     */
    void setIcon(const QIcon &icon);

    /**
     * Returns the icon that appears next to the prompt in the dialog. The default icon represents a simple key.
     * @since 5.63
     */
    QIcon icon() const;

    /**
     * Allow empty passwords? - Default: true
     *
     * same as setMinimumPasswordLength( allowed ? 0 : 1 )
     */
    void setAllowEmptyPasswords(bool allowed);

    /**
     * Allow empty passwords?
     *
     * @return true if minimumPasswordLength() == 0
     */
    bool allowEmptyPasswords() const;

    /**
     * Minimum acceptable password length.
     *
     * Default: 0
     *
     * @param minLength The new minimum password length
     */
    void setMinimumPasswordLength(int minLength);

    /**
     * Minimum acceptable password length.
     */
    int minimumPasswordLength() const;

    /**
     * Maximum acceptable password length.
     *
     * @param maxLength The new maximum password length.
     */
    void setMaximumPasswordLength(int maxLength);

    /**
     * Maximum acceptable password length.
     */
    int maximumPasswordLength() const;

    /**
     * Password length that is expected to be reasonably safe.
     *
     * Used to compute the strength level
     *
     * Default: 8 - the standard UNIX password length
     *
     * @param reasonableLength The new reasonable password length.
     */
    void setReasonablePasswordLength(int reasonableLength);

    /**
     * Password length that is expected to be reasonably safe.
     */
    int reasonablePasswordLength() const;

    /**
     * Set the password strength level below which a warning is given
     * Value is in the range 0 to 99. Empty passwords score 0;
     * non-empty passwords score up to 100, depending on their length and whether they
     * contain numbers, mixed case letters and punctuation.
     *
     * Default: 1 - warn if the password has no discernible strength whatsoever
     * @param warningLevel The level below which a warning should be given.
     */
    void setPasswordStrengthWarningLevel(int warningLevel);

    /**
     * Password strength level below which a warning is given
     */
    int passwordStrengthWarningLevel() const;

    /**
     * When the verification password does not match, the background color
     * of the verification field is set to @p color. As soon as the passwords match,
     * the original color of the verification field is restored.
     *
     * Default: the background color from the current theme.
     * @since 5.17
     */
    void setBackgroundWarningColor(const QColor &color);

    /**
     * The color used as warning for the verification password field's background.
     * @since 5.17
     */
    QColor backgroundWarningColor() const;

    /**
     * Returns the password entered.
     * @note Only has meaningful data after accept has been called
     *       if you want to access the password from a subclass use
     *       checkAndGetPassword()
     */
    QString password() const;

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
    /**
     * Whether to show the visibility trailing action in the line edit.
     * Default is true. This can be used to honor the lineedit_reveal_password
     * kiosk key, for example:
     * \code
     * passwordDialog.setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
     * \endcode
     * @since 5.31
     */
    [[deprecated("Use setRevealPasswordMode instead.")]] void setRevealPasswordAvailable(bool reveal);

    /**
     * Whether the visibility trailing action in the line edit is visible.
     * @since 5.31
     */
    [[deprecated("Use revealPasswordMode instead.")]] bool isRevealPasswordAvailable() const;
#endif

    /**
     * Whether the visibility trailing action in the line edit is visible.
     * @since 6.0
     */
    KPassword::RevealMode revealPasswordMode() const;

    /**
     * Set when the reveal password button will be visible.
     *
     * The default is RevealPasswordMode::OnlyNew and the reveal password button will
     * only be visible when entering a new password.
     *
     * This can be used to honor the lineedit_reveal_password kiosk key, for example:
     *
     * @code{.cpp}
     * if (KAuthorized::authorize(QStringLiteral("lineedit_reveal_password"))) {
     *     newPasswordDialog.setRevealPasswordMode(KPassword::RevealMode::OnlyNew);
     * } else {
     *     newPasswordDialog.setRevealPasswordMode(KPassword::RevealMode::Never);
     * }
     * @endcode
     * @since 6.0
     */
    void setRevealPasswordMode(KPassword::RevealMode revealPasswordMode);

    /**
     * @internal
     */
    void accept() override;

protected:
    /**
     * Virtual function that can be overridden to provide password
     * checking in derived classes. It should return @p true if the
     * password is valid, @p false otherwise.
     */
    virtual bool checkPassword(const QString &);

    /**
     * Checks input password.
     * If the password is right, returns true
     * and fills pwd with the password.
     * Otherwise returns false and pwd will be null.
     * @since 4.2
     */
    bool checkAndGetPassword(QString *pwd);

Q_SIGNALS:

    /**
     * The dialog has been accepted, and the new password is @p password
     */
    void newPassword(const QString &password);

private:
    std::unique_ptr<class KNewPasswordDialogPrivate> const d;
};

#endif // KNEWPASSWORDDIALOG_H
