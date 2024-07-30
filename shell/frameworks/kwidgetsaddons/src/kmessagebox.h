/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

// krazy:excludeall=dpointer

#ifndef KMESSAGEBOX_H
#define KMESSAGEBOX_H

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStringList>

#include <kguiitem.h>
#include <kstandardguiitem.h>

#include <kwidgetsaddons_export.h>

class KMessageBoxDontAskAgainInterface;
class KMessageBoxNotifyInterface;
class QDialog;
class QDialogButtonBox;
class QWidget;
class KConfig;

/**
 * Easy message dialog box.
 *
 * Provides convenience functions for some i18n'ed standard dialogs,
 * as well as audible notification via @ref KNotification
 *
 * The text in message boxes is wrapped automatically. The text may either
 * be plaintext or richtext. If the text is plaintext, a newline-character
 * may be used to indicate the end of a paragraph.
 *
 * \image html kmessagebox.png "An information dialog box"
 *
 * @author Waldo Bastian (bastian@kde.org)
 */
namespace KMessageBox
{
/**
 * Button types.
 */
enum ButtonCode {
    Ok = 1, ///< Ok button
    Cancel = 2, ///< Cancel button
    PrimaryAction = 3, ///< Primary action button; @since 5.100
    SecondaryAction = 4, ///< Secondary action button; @since 5.100
    Continue = 5, ///< Continue button
};

enum DialogType {
    QuestionTwoActions = 1, ///< Question dialog with two buttons; @since 5.100
    WarningTwoActions = 2, ///< Warning dialog with two buttons; @since 5.100
    WarningContinueCancel = 3, ///< Warning dialog with Continue and Cancel
    WarningTwoActionsCancel = 4, ///< Warning dialog with two buttons and Cancel; @since 5.100
    Information = 5, ///< Information dialog
    // Reserved for: SSLMessageBox = 6
    Error = 8, ///< Error dialog
    QuestionTwoActionsCancel = 9, ///< Question dialog with two buttons and Cancel; @since 5.100
};

/**
 * @see Options
 */
enum Option {
    Notify = 1, ///< Emit a KNotify event
    AllowLink = 2, ///< The message may contain links.
    Dangerous = 4, ///< The action to be confirmed by the dialog is a potentially destructive one. The default button will be set to Cancel or SecondaryAction,
                   ///< depending on which is available.
    NoExec = 16, ///< Do not call exec() in createKMessageBox()
    WindowModal = 32, ///< The window is to be modal relative to its parent. By default, it is application modal.
};

/**
 * Stores a combination of #Option values.
 */
Q_DECLARE_FLAGS(Options, Option)

// This declaration must be defined before first Option is used in method signatures
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

/**
 * Display a "question" dialog with two action buttons.
 *
 * To be used for questions like "Do you want to save the message for later or discard it?".
 *
 * The default button is the primary button. Pressing "Esc" triggers the secondary button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param title   the message box title. If an empty string, defaults to i18n("Question").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options see Option
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActions(QWidget *parent,
                              const QString &text,
                              const QString &title,
                              const KGuiItem &primaryAction,
                              const KGuiItem &secondaryAction,
                              const QString &dontAskAgainName = QString(),
                              Options options = Notify);

/**
 * Display a "question" dialog with two action buttons and a cancel button.
 *
 * To be used for questions like "Do you want to save the message for later or discard it?".
 *
 * The default button is the primary button. Pressing "Esc" triggers the cancel button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param title   the message box title. If an empty string, defaults to i18n("Question").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param cancelAction the action for the cancel button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options  see Option
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered. @c Cancel if the cancel button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsCancel(QWidget *parent,
                                    const QString &text,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                    const QString &dontAskAgainName = QString(),
                                    Options options = Notify);

/**
 * Display a "question" dialog with a listbox to show information to the user
 * and two action buttons.
 *
 * To be used for questions like "Do you really want to delete these files?"
 * and show the user exactly which files are going to be deleted in case.
 *
 * The default button is the primary button. Pressing "Esc" triggers the secondary button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param strlist List of strings to be written in the listbox. If the list is
 *                empty, it doesn't show any listbox, working as questionTwoActions().
 * @param title   the message box title. If an empty string, defaults to i18n("Question").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options  see Option
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsList(QWidget *parent,
                                  const QString &text,
                                  const QStringList &strlist,
                                  const QString &title,
                                  const KGuiItem &primaryAction,
                                  const KGuiItem &secondaryAction,
                                  const QString &dontAskAgainName = QString(),
                                  Options options = Notify);

/**
 * Display a "warning" dialog with two action buttons.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the secondary button. Pressing "Esc" triggers the secondary button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param title   the message box title. If an empty string, defaults to i18n("Warning").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options see Options
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActions(QWidget *parent,
                             const QString &text,
                             const QString &title,
                             const KGuiItem &primaryAction,
                             const KGuiItem &secondaryAction,
                             const QString &dontAskAgainName = QString(),
                             Options options = Options(Notify | Dangerous));

/**
 * Display a "warning" dialog with a listbox to show information to the user
 * and two action buttons.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the secondary button. Pressing "Esc" triggers the secondary button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param strlist List of strings to be written in the listbox. If the list is
 *                empty, it doesn't show any listbox, working as warningTwoActions.
 * @param title   the message box title. If an empty string, defaults to i18n("Warning").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options see Options
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsList(QWidget *parent,
                                 const QString &text,
                                 const QStringList &strlist,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName = QString(),
                                 Options options = Options(Notify | Dangerous));

/**
 * Display a "warning" dialog.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Warning").
 * @param buttonContinue The text for the first button.
 *                       The default is KStandardGuiItem::cont().
 * @param buttonCancel The text for the second button.
 *                     The default is KStandardGuiItem::cancel().
 * @param dontAskAgainName If provided, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options  see Options
 *
 * @return  @p Continue is returned if the Continue-button is pressed.
 *          @p Cancel is returned if the Cancel-button is pressed.
 *
 * To be used for questions like "You are about to Print. Are you sure?"
 * the continueButton should then be labeled "Print".
 *
 * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancel(QWidget *parent,
                                                       const QString &text,
                                                       const QString &title = QString(),
                                                       const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                       const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                       const QString &dontAskAgainName = QString(),
                                                       Options options = Notify);

/**
 * Display a "warning" dialog with a collapsible "Details" section.
 *
 * @since 5.61
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelDetailed(QWidget *parent,
                                                               const QString &text,
                                                               const QString &title = QString(),
                                                               const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                               const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                               const QString &dontAskAgainName = QString(),
                                                               Options options = Notify,
                                                               const QString &details = QString());

/**
 * Display a "warning" dialog with a listbox to show information to the user.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as warningContinueCancel.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Warning").
 * @param buttonContinue The text for the first button.
 *                       The default is KStandardGuiItem::cont().
 * @param buttonCancel The text for the second button.
 *                     The default is KStandardGuiItem::cancel().
 * @param dontAskAgainName If provided, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * @param options  see Options
 *
 * @return  @p Continue is returned if the Continue-button is pressed.
 *          @p Cancel is returned if the Cancel-button is pressed.
 *
 * To be used for questions like "You are about to Print. Are you sure?"
 * the continueButton should then be labeled "Print".
 *
 * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelList(QWidget *parent,
                                                           const QString &text,
                                                           const QStringList &strlist,
                                                           const QString &title = QString(),
                                                           const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                           const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                           const QString &dontAskAgainName = QString(),
                                                           Options options = Notify);

/**
 * Display a "warning" dialog with two action buttons and a cancel button.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the cancel button. Pressing "Esc" triggers the cancel button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param title   the message box title. If an empty string, defaults to i18n("Warning").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param cancelAction the action for the cancel button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options see Options
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered. @c Cancel if the cancel button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &title,
                                   const KGuiItem &primaryAction,
                                   const KGuiItem &secondaryAction,
                                   const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                   const QString &dontAskAgainName = QString(),
                                   Options options = Options(Notify | Dangerous));

/**
 * Display a "warning" dialog with a listbox to show information
 * to the user, two action buttons and a cancel button.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the cancel button. Pressing "Esc" triggers the cancel button.
 *
 * @param parent  the parent widget
 * @param text    the message string
 * @param strlist a List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as warningTwoActionsCancel().
 * @param title   the message box title. If an empty string, defaults to i18n("Warning").
 * @param primaryAction the action for the primary button
 * @param secondaryAction the action for the secondary button
 * @param cancelAction the action for the cancel button
 * @param dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If @p dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * @param options see Options
 *
 * @returns @c PrimaryAction if the primary button is triggered, @c SecondaryAction
 *          if the secondary button is triggered. @c Cancel if the cancel button is triggered.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelList(QWidget *parent,
                                       const QString &text,
                                       const QStringList &strlist,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                       const QString &dontAskAgainName = QString(),
                                       Options options = Options(Notify | Dangerous));

/**
 * Display an "Error" dialog.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 * @param options  see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * @note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void error(QWidget *parent, const QString &text, const QString &title = QString(), Options options = Notify);

/**
 * Display an "Error" dialog.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 * @param buttonOk The text for the only button.
 *                 The default is KStandardGuiItem::ok().
 * @param options  see Options
 *
 * There is only one button, therefore it's the default button, and pressing "Esc" selects it as well.
 *
 * @since 5.97
 */
KWIDGETSADDONS_EXPORT
void error(QWidget *parent,
           const QString &text,
           const QString &title /*= QString()*/,
           const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
           Options options = Notify); // TODO KF6 merge with previous overload

/**
 * Display an "Error" dialog with a listbox.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as error().
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 * @param options  see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * @note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
errorList(QWidget *parent, const QString &text, const QStringList &strlist, const QString &title = QString(), Options options = Notify);

/**
 * Displays an "Error" dialog with a "Details >>" button.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param details Detailed message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 * @param options  see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The @p details message can contain additional information about
 * the problem and can be shown on request to advanced/interested users.
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * @note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
detailedError(QWidget *parent, const QString &text, const QString &details, const QString &title = QString(), Options options = Notify);

/**
 * Displays an "Error" dialog with a "Details >>" button.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param details Detailed message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 * @param buttonOk The text for the only button.
 *                 The default is KStandardGuiItem::ok().
 * @param options  see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The @p details message can contain additional information about
 * the problem and can be shown on request to advanced/interested users.
 *
 * There is only one button, therefore it's the default button, and pressing "Esc" selects it as well.
 *
 * @since 5.97
 */
KWIDGETSADDONS_EXPORT
void detailedError(QWidget *parent,
                   const QString &text,
                   const QString &details,
                   const QString &title /*= QString()*/,
                   const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
                   Options options = Notify); // TODO KF6 merge with previous overload

/**
 * Display an "Information" dialog.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Information").
 * @param dontShowAgainName If provided, a checkbox is added with which
 *                further notifications can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 * @param options  see Options
 *
 *
 * Your program wants to tell the user something.
 * To be used for things like:
 * "Your bookmarks have been rearranged."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 *  @note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
information(QWidget *parent, const QString &text, const QString &title = QString(), const QString &dontShowAgainName = QString(), Options options = Notify);

/**
 * Display an "Information" dialog with a listbox.
 *
 * @param parent  Parent widget.
 * @param text    Message string.
 * @param strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as information.
 * @param title   Message box title. The application name is added to
 *                the title. The default title is i18n("Information").
 * @param dontShowAgainName If provided, a checkbox is added with which
 *                further notifications can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 * @param options  see Options
 *
 *
 * Your program wants to tell the user something.
 * To be used for things like:
 * "The following bookmarks have been rearranged:"
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 *  @note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void informationList(QWidget *parent,
                                           const QString &text,
                                           const QStringList &strlist,
                                           const QString &title = QString(),
                                           const QString &dontShowAgainName = QString(),
                                           Options options = Notify);

/**
 * Enable all messages which have been turned off with the
 * @p dontShowAgainName feature.
 */
KWIDGETSADDONS_EXPORT void enableAllMessages();

/**
 * Re-enable a specific @p dontShowAgainName messages that had
 * previously been turned off.
 * @see saveDontShowAgainTwoActions()
 * @see saveDontShowAgainContinue()
 */
KWIDGETSADDONS_EXPORT void enableMessage(const QString &dontShowAgainName);

/**
 * Alternate method to show a messagebox:
 *
 * @param parent Parent widget.
 * @param type type of message box: QuestionTwoActions, WarningTwoActions, WarningContinueCancel...
 * @param text Message string.
 * @param title Message box title.
 * @param primaryAction The KGuiItem for the first button.
 * @param secondaryAction The KGuiItem for the second button.
 * @param cancelAction The text for the third button.
 *                     The default is KStandardGuiItem::cancel().
 * @param dontShowAskAgainName If provided, a checkbox is added with which
 *                further questions/information can be turned off. If turned off
 *                all questions will be automatically answered with the
 *                last answer (either PrimaryAction or SecondaryAction),
 *                if the message box needs an answer.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 * @param options  see Options
 * Note: for ContinueCancel, primaryAction is the continue button and secondaryAction is unused.
 *       and for Information, none is used.
 * @return a button code, as defined in KMessageBox.
 */
KWIDGETSADDONS_EXPORT
ButtonCode messageBox(QWidget *parent,
                      DialogType type,
                      const QString &text,
                      const QString &title,
                      const KGuiItem &primaryAction,
                      const KGuiItem &secondaryAction,
                      const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                      const QString &dontShowAskAgainName = QString(),
                      Options options = Notify);

/**
 * @param dontShowAgainName the name that identifies the message box.
 *                          If empty, @c true is always returned.
 * @param result reference to a variable to be set to the choice (@c PrimaryAction or @c SecondaryAction)
 *               that was chosen the last time the message box was shown.
 *               Only meaningful if the message box should not be shown.
 * @returns @c true if the corresponding two actions message box should be shown, @c false otherwise.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
bool shouldBeShownTwoActions(const QString &dontShowAgainName, ButtonCode &result);

/**
 * @return true if the corresponding continue/cancel message box should be
 * shown.
 * @param dontShowAgainName the name that identify the message box. If
 * empty, true is always returned.
 */
KWIDGETSADDONS_EXPORT bool shouldBeShownContinue(const QString &dontShowAgainName);

/**
 * Save the fact that a two actions message box should not be shown again.
 *
 * @param dontShowAgainName the name that identifies the message box.
 *                          If empty, this method does nothing.
 * @param result the value (@c PrimaryAction or @c SecondaryAction) that should be used
 *               as the result for the message box.
 *
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
void saveDontShowAgainTwoActions(const QString &dontShowAgainName, ButtonCode result);

/**
 * Save the fact that the continue/cancel message box should not be shown
 * again.
 * @param dontShowAgainName the name that identify the message box. If
 * empty, this method does nothing.
 */
KWIDGETSADDONS_EXPORT void saveDontShowAgainContinue(const QString &dontShowAgainName);

/**
 * Use @p cfg for all settings related to the dontShowAgainName feature.
 * If @p cfg is 0 (default) KGlobal::config() will be used.
 */
KWIDGETSADDONS_EXPORT void setDontShowAgainConfig(KConfig *cfg);

/**
 * Use @p dontAskAgainInterface for all settings related to the dontShowAgain feature.
 * This method does not take ownership of @p dontAskAgainInterface.
 *
 * @since 5.0
 */
KWIDGETSADDONS_EXPORT void setDontShowAgainInterface(KMessageBoxDontAskAgainInterface *dontAskAgainInterface);

/**
 * Use @p notifyInterface to send notifications.
 * This method does not take ownership of @p notifyInterface.
 *
 * @since 5.0
 */
KWIDGETSADDONS_EXPORT void setNotifyInterface(KMessageBoxNotifyInterface *notifyInterface);

/**
 * Create content and layout of a standard dialog
 *
 * @param dialog  The parent dialog base
 * @param buttons a QDialogButtonBox instance. This function will take care of connecting to it.
 * @param icon    Which predefined icon the message box shall show.
 * @param text    Message string.
 * @param strlist List of strings to be written in the listbox.
 *                If the list is empty, it doesn't show any listbox
 * @param ask     The text of the checkbox. If empty none will be shown.
 * @param checkboxReturn The result of the checkbox. If it's initially
 *                true then the checkbox will be checked by default.
 *                May be a null pointer. Incompatible with NoExec.
 * @param options  see Options
 * @param details Detailed message string.
 * @return A QDialogButtonBox::StandardButton button code, not a KMessageBox
 *         button code, based on the buttonmask given to the constructor of
 *         the @p dialog (ie. will return QDialogButtonBox::Yes instead of
 *         KMessageBox::PrimaryAction). Will return QDialogButtonBox::NoButton if the
 *         message box is queued for display instead of exec()ed immediately
 *         or if the option NoExec is set.
 * @note   Unless NoExec is used,
 *         the @p dialog that is passed in is deleted by this
 *         function. Do not delete it yourself.
 */
KWIDGETSADDONS_EXPORT QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                                         QDialogButtonBox *buttons,
                                                                         QMessageBox::Icon icon, // krazy:exclude=qclasses
                                                                         const QString &text,
                                                                         const QStringList &strlist,
                                                                         const QString &ask,
                                                                         bool *checkboxReturn,
                                                                         Options options,
                                                                         const QString &details = QString());

/**
 * Create content and layout of a standard dialog
 *
 * @param dialog  The parent dialog base
 * @param buttons a QDialogButtonBox instance. This function will take care of connecting to it.
 * @param icon    A QPixmap containing the icon to be displayed in the
 *                dialog next to the text.
 * @param text    Message string.
 * @param strlist List of strings to be written in the listbox.
 *                If the list is empty, it doesn't show any listbox
 * @param ask     The text of the checkbox. If empty none will be shown.
 * @param checkboxReturn The result of the checkbox. If it's initially
 *                true then the checkbox will be checked by default.
 *                May be a null pointer. Incompatible with NoExec.
 * @param options  see Options
 * @param details Detailed message string.
 * @param notifyType The type of notification to send when this message
 *                is presentend.
 * @return A QDialogButtonBox::StandardButton button code, not a KMessageBox
 *         button code, based on the buttonmask given to the constructor of
 *         the @p dialog (ie. will return QDialogButtonBox::Yes instead of
 *         KMessageBox::PrimaryAction). Will return QDialogButtonBox::NoButton if the
 *         message box is queued for display instead of exec()ed immediately
 *         or if the option NoExec is set.
 * @note   Unless NoExec is used,
 *         the @p dialog that is passed in is deleted by this
 *         function. Do not delete it yourself.
 */
KWIDGETSADDONS_EXPORT QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                                         QDialogButtonBox *buttons,
                                                                         const QIcon &icon,
                                                                         const QString &text,
                                                                         const QStringList &strlist,
                                                                         const QString &ask,
                                                                         bool *checkboxReturn,
                                                                         Options options,
                                                                         const QString &details = QString(),
                                                                         QMessageBox::Icon notifyType = QMessageBox::Information); // krazy:exclude=qclasses

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see questionTwoActions()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsWId(WId parent_id,
                                 const QString &text,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName = QString(),
                                 Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see questionTwoActionsCancel()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsCancelWId(WId parent_id,
                                       const QString &text,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                       const QString &dontAskAgainName = QString(),
                                       Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see questionTwoActionsList()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsListWId(WId parent_id,
                                     const QString &text,
                                     const QStringList &strlist,
                                     const QString &title,
                                     const KGuiItem &primaryAction,
                                     const KGuiItem &secondaryAction,
                                     const QString &dontAskAgainName = QString(),
                                     Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see warningTwoActions()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsWId(WId parent_id,
                                const QString &text,
                                const QString &title,
                                const KGuiItem &primaryAction,
                                const KGuiItem &secondaryAction,
                                const QString &dontAskAgainName = QString(),
                                Options options = Options(Notify | Dangerous));

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see warningTwoActionsList()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsListWId(WId parent_id,
                                    const QString &text,
                                    const QStringList &strlist,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const QString &dontAskAgainName = QString(),
                                    Options options = Options(Notify | Dangerous));

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelWId(WId parent_id,
                                                          const QString &text,
                                                          const QString &title = QString(),
                                                          const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                          const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                          const QString &dontAskAgainName = QString(),
                                                          Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelListWId(WId parent_id,
                                                              const QString &text,
                                                              const QStringList &strlist,
                                                              const QString &title = QString(),
                                                              const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                              const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                              const QString &dontAskAgainName = QString(),
                                                              Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see warningTwoActionsCancel()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelWId(WId parent_id,
                                      const QString &text,
                                      const QString &title,
                                      const KGuiItem &primaryAction,
                                      const KGuiItem &secondaryAction,
                                      const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                      const QString &dontAskAgainName = QString(),
                                      Options options = Options(Notify | Dangerous));

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * @see warningTwoActionsCancelList()
 * @since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelListWId(WId parent_id,
                                          const QString &text,
                                          const QStringList &strlist,
                                          const QString &title,
                                          const KGuiItem &primaryAction,
                                          const KGuiItem &secondaryAction,
                                          const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                          const QString &dontAskAgainName = QString(),
                                          Options options = Options(Notify | Dangerous));

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void errorWId(WId parent_id, const QString &text, const QString &title = QString(), Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
errorListWId(WId parent_id, const QString &text, const QStringList &strlist, const QString &title = QString(), Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
detailedErrorWId(WId parent_id, const QString &text, const QString &details, const QString &title = QString(), Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 * @since 5.97
 */
KWIDGETSADDONS_EXPORT
void detailedErrorWId(WId parent_id,
                      const QString &text,
                      const QString &details,
                      const QString &title /*= QString()*/,
                      const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
                      Options options = Notify); // TODO KF6 merge with previous overload

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
informationWId(WId parent_id, const QString &text, const QString &title = QString(), const QString &dontShowAgainName = QString(), Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void informationListWId(WId parent_id,
                                              const QString &text,
                                              const QStringList &strlist,
                                              const QString &title = QString(),
                                              const QString &dontShowAgainName = QString(),
                                              Options options = Notify);

/**
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT
ButtonCode messageBoxWId(WId parent_id,
                         DialogType type,
                         const QString &text,
                         const QString &title,
                         const KGuiItem &primaryAction,
                         const KGuiItem &secondaryAction,
                         const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                         const QString &dontShowAskAgainName = QString(),
                         Options options = Notify);
}

#endif
