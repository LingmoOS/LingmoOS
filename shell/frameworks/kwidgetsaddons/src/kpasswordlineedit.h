/*
    SPDX-FileCopyrightText: 2017 Montel Laurent <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KPASSWORDLINEEDIT_H
#define KPASSWORDLINEEDIT_H

#include <KPassword>
#include <QLineEdit>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>
class QAction;

/**
 * @class KPasswordLineEdit kpasswordlineedit.h KPasswordLineEdit
 *
 * A lineedit which allows to display password
 *
 * \section usage Usage Example
 *
 * Get password
 *
 * \code
 * KPasswordLineEdit *passwordLineEdit = new KPasswordLineEdit(parent);
 * QString password = passwordLineEdit->password();
 * \endcode
 *
 * @author Laurent Montel <montel@kde.org>
 * @since 5.37
 */
class KWIDGETSADDONS_EXPORT KPasswordLineEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool clearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled)
    Q_PROPERTY(QLineEdit::EchoMode echoMode READ echoMode WRITE setEchoMode NOTIFY echoModeChanged)
public:
    /**
     * Constructs a lineedit password widget.
     * @since 5.37
     *
     * @param parent Passed to lower level constructor.
     */
    explicit KPasswordLineEdit(QWidget *parent = nullptr);
    /**
     * Destructs the lineedit password widget.
     */
    ~KPasswordLineEdit() override;

    /**
     * Assign password
     */
    void setPassword(const QString &password);

    /**
     * Returns the password entered.
     */
    QString password() const;

    /**
     * Clear text
     */
    void clear();

    /**
     * Show/hide clear button (false by default)
     */
    void setClearButtonEnabled(bool clear);

    /**
     * Inform if we show or not clear button
     */
    bool isClearButtonEnabled() const;

    /**
     * Change echo mode (QLineEdit::Password by default)
     */
    void setEchoMode(QLineEdit::EchoMode mode);

    /**
     * Return echo mode
     */
    QLineEdit::EchoMode echoMode() const;

    /**
     * Set whether the line edit is read only.
     * @since 6.0
     */
    void setReadOnly(bool readOnly);

    /**
     * Return whether the line edit is read only.
     * @since 6.0
     */
    bool isReadOnly() const;

    /**
     * Return when the reveal password button is visible.
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
     *     passwordLineEdit.setRevealPasswordMode(KPasswordLineEdit::RevealPasswordMode::OnlyNew);
     * } else {
     *     passwordLineEdit.setRevealPasswordMode(KPasswordLineEdit::RevealPasswordMode::Never);
     * }
     * @endcode
     * @since 6.0
     */
    void setRevealPasswordMode(KPassword::RevealMode revealPasswordMode);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
    /**
     * Whether to show the visibility trailing action in the line edit.
     * Default is true. This can be used to honor the lineedit_reveal_password
     * kiosk key, for example:
     * \code
     * passwordLineEdit.setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
     * \endcode
     */
    [[deprecated("Use setRevealPasswordMode")]] void setRevealPasswordAvailable(bool reveal);

    /**
     * Whether the visibility trailing action in the line edit is visible.
     */
    [[deprecated("Use revealPasswordMode instead.")]] bool isRevealPasswordAvailable() const;
#endif

    /**
     * @internal
     * Returns the QAction
     */
    QAction *toggleEchoModeAction() const;

    /**
     * Returns the lineedit widget.
     */
    QLineEdit *lineEdit() const;

Q_SIGNALS:
    /**
     * When we click on visibility icon echo mode is switched between Normal echo mode and Password echo mode
     */
    void echoModeChanged(QLineEdit::EchoMode echoMode);
    void passwordChanged(const QString &password);

private:
    std::unique_ptr<class KPasswordLineEditPrivate> const d;
};

#endif
