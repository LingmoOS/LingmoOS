// vi: ts=8 sts=4 sw=4
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Pietro Iglio <iglio@fub.it>
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2004, 2005 Andrew Coles <andrew_coles@yahoo.co.uk>
    SPDX-FileCopyrightText: 2006, 2007 Olivier Goffart <ogoffart @ kde.org>
    SPDX-FileCopyrightText: 2015 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KNEWPASSWORDWIDGET_H
#define KNEWPASSWORDWIDGET_H

#include <KPassword>
#include <KPasswordLineEdit>
#include <QWidget>
#include <memory>

#include <kwidgetsaddons_export.h>

/**
 * @class KNewPasswordWidget knewpasswordwidget.h KNewPasswordWidget
 *
 * @short A password input widget.
 *
 * This widget allows the user to enter a new password.
 *
 * The password has to be entered twice to check if the passwords
 * match. A hint about the strength of the entered password is also
 * shown.
 *
 * In order to embed this widget in your custom password dialog,
 * you may want to connect to the passwordStatus() signal.
 * This way you can e.g. disable the OK button if the passwords
 * don't match, warn the user if the password is too weak, and so on.
 *
 * \section usage Usage Example
 * \subsection Setup
 *
 * \code
 *  KNewPasswordWidget *m_passwordWidget = new KNewPasswordWidget(this);
 *  // set a background warning color (taken from the current color scheme)
 *  KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
 *  m_passwordWidget->setBackgroundWarningColor(colorScheme.background(KColorScheme::NegativeBackground).color());
 *  // listen to password status updates
 *  connect(m_passwordWidget, &KNewPasswordWidget::passwordStatusChanged, this, &MyCustomDialog::slotPasswordStatusChanged);
 *  ...
 * \endcode
 *
 * \subsection update Update your custom dialog
 *
 * @snippet knewpasswordwidget_test.cpp update_custom_dialog
 *
 * \subsection accept Accept your custom dialog
 *
 * @snippet knewpasswordwidget_test.cpp accept_custom_dialog
 *
 * @author Geert Jansen <jansen@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Elvis Angelaccio <elvis.angelaccio@kde.org>
 * @since 5.16
 */
class KWIDGETSADDONS_EXPORT KNewPasswordWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(PasswordStatus passwordStatus READ passwordStatus)
    Q_PROPERTY(bool allowEmptyPasswords READ allowEmptyPasswords WRITE setAllowEmptyPasswords)
    Q_PROPERTY(int minimumPasswordLength READ minimumPasswordLength WRITE setMinimumPasswordLength)
    Q_PROPERTY(int maximumPasswordLength READ maximumPasswordLength WRITE setMaximumPasswordLength)
    Q_PROPERTY(int reasonablePasswordLength READ reasonablePasswordLength WRITE setReasonablePasswordLength)
    Q_PROPERTY(int passwordStrengthWarningLevel READ passwordStrengthWarningLevel WRITE setPasswordStrengthWarningLevel)
    Q_PROPERTY(QColor backgroundWarningColor READ backgroundWarningColor WRITE setBackgroundWarningColor)
    Q_PROPERTY(bool passwordStrengthMeterVisible READ isPasswordStrengthMeterVisible WRITE setPasswordStrengthMeterVisible)
    /**
     * @since 5.31
     */
#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
    Q_PROPERTY(bool revealPasswordAvailable READ isRevealPasswordAvailable WRITE setRevealPasswordAvailable)
#endif
    Q_PROPERTY(KPassword::RevealMode revealPasswordMode READ revealPasswordMode WRITE setRevealPasswordMode)

public:
    /**
     * Status of the password being typed in the widget.
     */
    enum PasswordStatus {
        EmptyPasswordNotAllowed, /**< Both passwords fields empty, but minimum length > 0. */
        PasswordTooShort, /**< Password length is too low. */
        PasswordNotVerified, /**< Password and verification password don't match. */
        WeakPassword, /**< Passwords match but the strength level is not enough. */
        StrongPassword, /**< Passwords match and the strength level is good. */
    };
    Q_ENUM(PasswordStatus)

    /**
     * This enum describe when the reveal password button is visible.
     * @since 6.0
     */
    enum class RevealPasswordMode {
        /**
         * Display the button when entering a new password, but doesn't let you see a
         * previously entered password. This is the default.
         */
        OnlyNew,
        /**
         * Never display the reveal button.
         */
        Never,
        /**
         * Always display the reveal button. Usefull in a password manager for example.
         */
        Always,
    };
    Q_ENUM(RevealPasswordMode)

    /**
     * Constructs a password widget.
     *
     * @param parent Passed to lower level constructor.
     */
    explicit KNewPasswordWidget(QWidget *parent = nullptr);

    /**
     * Destructs the password widget.
     */
    ~KNewPasswordWidget() override;

    /**
     * The current status of the password in the widget.
     */
    PasswordStatus passwordStatus() const;

    /**
     * Allow empty passwords?
     *
     * @return true if minimumPasswordLength() == 0
     */
    bool allowEmptyPasswords() const;

    /**
     * Minimum acceptable password length.
     */
    int minimumPasswordLength() const;

    /**
     * Maximum acceptable password length.
     */
    int maximumPasswordLength() const;

    /**
     * Password length that is expected to be reasonably safe.
     */
    int reasonablePasswordLength() const;

    /**
     * Password strength level below which a warning is given
     */
    int passwordStrengthWarningLevel() const;

    /**
     * The color used as warning for the verification password field's background.
     */
    QColor backgroundWarningColor() const;

    /**
     * Whether the password strength meter is visible.
     */
    bool isPasswordStrengthMeterVisible() const;

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
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
     * Returns the password entered.
     * @note Only returns meaningful data when passwordStatus
     *       is either WeakPassword or StrongPassword.
     */
    QString password() const;

public Q_SLOTS:

    /**
     * Allow empty passwords? - Default: true
     *
     * same as setMinimumPasswordLength( allowed ? 0 : 1 )
     */
    void setAllowEmptyPasswords(bool allowed);

    /**
     * Minimum acceptable password length.
     *
     * Default: 0
     *
     * @param minLength The new minimum password length
     */
    void setMinimumPasswordLength(int minLength);

    /**
     * Maximum acceptable password length.
     *
     * @param maxLength The new maximum password length.
     */
    void setMaximumPasswordLength(int maxLength);

    /**
     * Password length that is expected to be reasonably safe.
     * The value is guaranteed to be in the range from 1 to maximumPasswordLength().
     *
     * Used to compute the strength level
     *
     * Default: 8 - the standard UNIX password length
     *
     * @param reasonableLength The new reasonable password length.
     */
    void setReasonablePasswordLength(int reasonableLength);

    /**
     * Set the password strength level below which a warning is given
     * The value is guaranteed to be in the range from 0 to 99. Empty passwords score 0;
     * non-empty passwords score up to 100, depending on their length and whether they
     * contain numbers, mixed case letters and punctuation.
     *
     * Default: 1 - warn if the password has no discernible strength whatsoever
     * @param warningLevel The level below which a warning should be given.
     */
    void setPasswordStrengthWarningLevel(int warningLevel);

    /**
     * When the verification password does not match, the background color
     * of the verification field is set to @p color. As soon as the passwords match,
     * the original color of the verification field is restored.
     */
    void setBackgroundWarningColor(const QColor &color);

    /**
     * Whether to show the password strength meter (label and progress bar).
     * Default is true.
     */
    void setPasswordStrengthMeterVisible(bool visible);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
    /**
     * Whether to show the visibility trailing action in the line edit.
     * Default is true. This can be used to honor the lineedit_reveal_password
     * kiosk key, for example:
     * \code
     * passwordWidget.setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
     * \endcode
     * @since 5.31
     */
    [[deprecated("Use setRevealPasswordMode instead.")]] void setRevealPasswordAvailable(bool reveal);
#endif

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
     *     newPasswordWidget.setRevealPasswordMode(KPassword::RevealMode::OnlyNew);
     * } else {
     *     newPasswordWidget.setRevealPasswordMode(KPassword::RevealMode::Never);
     * }
     * @endcode
     * @since 6.0
     */
    void setRevealPasswordMode(KPassword::RevealMode revealPasswordMode);

Q_SIGNALS:

    /**
     * Notify about the current status of the password being typed.
     */
    void passwordStatusChanged();

private:
    std::unique_ptr<class KNewPasswordWidgetPrivate> const d;

    Q_DISABLE_COPY(KNewPasswordWidget)
};

#endif // KNEWPASSWORDWIDGET_H
