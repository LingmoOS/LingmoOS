/*
 *   SPDX-FileCopyrightText: 2006 Hans van Leeuwen <hanz@hanz.nl>
 *   SPDX-FileCopyrightText: 2008-2010 Armin Berres <armin@space-based.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <memory>
#include <sys/resource.h>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>
#include <kwallet.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QInputDialog>
#include <QLoggingCategory>
#include <QPointer>
#include <QRegularExpression>
#include <QTextStream>

Q_LOGGING_CATEGORY(LOG_KSSHASKPASS, "ksshaskpass")

constexpr const char *PROMPT_TYPE_ENV_VAR = "SSH_ASKPASS_PROMPT";

// Standard prompt types defined by openssh.
enum class PromptType {
    Confirm,
    Entry,
    None,
};

// Implemented UI display types.
enum class DisplayType {
    Password,
    ClearText,
    Confirm,
    ConfirmCancel,
};

static void parsePrompt(PromptType promptType, const QString &prompt, QString &identifier, bool &ignoreWallet, DisplayType &displayType)
{
    if (promptType == PromptType::Confirm) {
        displayType = DisplayType::Confirm;
        ignoreWallet = true;
        return;
    }

    if (promptType == PromptType::None) {
        displayType = DisplayType::ConfirmCancel;
        ignoreWallet = true;
        return;
    }

    // "Entry" prompt type: password or text input. We parse several known prompts from openssh and git (which have no i18n)
    // to extract credential names and determine whether to use cleartext.
    QRegularExpressionMatch match;

    // openssh sshconnect2.c
    // Case: password for authentication on remote ssh server
    match = QRegularExpression(QStringLiteral("^(.*@.*)'s password: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // openssh sshconnect2.c
    // Case: password change request
    match = QRegularExpression(QStringLiteral("^(Enter|Retype) (.*@.*)'s (old|new) password: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(2);
        displayType = DisplayType::Password;
        ignoreWallet = true;
        return;
    }

    // openssh sshconnect2.c and sshconnect1.c
    // Case: asking for passphrase for a certain keyfile
    match = QRegularExpression(QStringLiteral("^Enter passphrase for( RSA)? key '(.*)': $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(2);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // openssh ssh-add.c
    // Case: asking for passphrase for a certain keyfile for the first time => we should try a password from the wallet
    match = QRegularExpression(QStringLiteral("^Enter passphrase for (.*?)( \\(will confirm each use\\))?: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // openssh ssh-add.c
    // Case: re-asking for passphrase for a certain keyfile => probably we've tried a password from the wallet, no point
    // in trying it again
    match = QRegularExpression(QStringLiteral("^Bad passphrase, try again for (.*?)( \\(will confirm each use\\))?: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = true;
        return;
    }

    // openssh ssh-pkcs11.c
    // Case: asking for PIN for some token label
    match = QRegularExpression(QStringLiteral("Enter PIN for '(.*)': $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // openssh ssh-agent.c
    // Case: asking to provide the PIN of the security key device
    // match after "for" is key type, match after "key" is SHA digest of key
    match = QRegularExpression(QStringLiteral("^Enter PIN( and confirm user presence)? for (.*?) key (.*?): $")).match(prompt);
    if (match.hasMatch()) {
        identifier = QStringLiteral("PIN:") + match.captured(3);
        displayType = DisplayType::Password;
        ignoreWallet = true;
        return;
    }

    // git credential.c
    // Case: asking for username by git without specifying any other information
    match = QRegularExpression(QStringLiteral("^Username: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = QString();
        displayType = DisplayType::ClearText;
        ignoreWallet = true;
        return;
    }

    // git credential.c
    // Case: asking for password by git without specifying any other information
    match = QRegularExpression(QStringLiteral("^Password: $")).match(prompt);
    if (match.hasMatch()) {
        identifier = QString();
        displayType = DisplayType::Password;
        ignoreWallet = true;
        return;
    }

    // git credential.c
    // Case: asking for username by git for some identifier
    match = QRegularExpression(QStringLiteral("^Username for '(.*)': $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::ClearText;
        ignoreWallet = false;
        return;
    }

    // git credential.c
    // Case: asking for password by git for some identifier
    match = QRegularExpression(QStringLiteral("^Password for '(.*)': $")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // Case: username extraction from git-lfs
    match = QRegularExpression(QStringLiteral("^Username for \"(.*?)\"$")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::ClearText;
        ignoreWallet = false;
        return;
    }

    // Case: password extraction from git-lfs
    match = QRegularExpression(QStringLiteral("^Password for \"(.*?)\"$")).match(prompt);
    if (match.hasMatch()) {
        identifier = match.captured(1);
        displayType = DisplayType::Password;
        ignoreWallet = false;
        return;
    }

    // Nothing matched; either it was called by some sort of a script with a custom prompt (i.e. not ssh-add), or
    // strings we're looking for were broken. Issue a warning and continue without identifier.
    qCWarning(LOG_KSSHASKPASS) << "Unable to parse phrase" << prompt;
}

void cancelDialog(QWidget *parent, const QString &text)
{
    QDialog *d = new QDialog(parent);
    d->setWindowTitle(i18nc("@title:window", "Enter SSH Credentials"));
    d->setObjectName(QStringLiteral("information"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(d);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

    KMessageBox::createKMessageBox(d, buttonBox, QMessageBox::Information, text, QStringList(), QString(), nullptr, KMessageBox::Notify);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("ksshaskpass"));

    // TODO update it.
    KAboutData about(QStringLiteral("ksshaskpass"),
                     i18n("SSH Credentials"),
                     QStringLiteral(PROJECT_VERSION),
                     i18n("KDE version of ssh-askpass"),
                     KAboutLicense::GPL,
                     i18n("(c) 2006 Hans van Leeuwen\n(c) 2008-2010 Armin Berres\n(c) 2013 Pali Rohár"),
                     i18n("Ksshaskpass allows you to interactively prompt users for a passphrase for ssh-add"),
                     QStringLiteral("https://commits.kde.org/ksshaskpass"),
                     QStringLiteral("armin@space-based.de"));

    about.addAuthor(i18n("Armin Berres"), i18n("Current author"), QStringLiteral("armin@space-based.de"));
    about.addAuthor(i18n("Hans van Leeuwen"), i18n("Original author"), QStringLiteral("hanz@hanz.nl"));
    about.addAuthor(i18n("Pali Rohár"), i18n("Contributor"), QStringLiteral("pali.rohar@gmail.com"));
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+[prompt]"), i18nc("Name of a prompt for a password", "Prompt")));

    parser.process(app);
    about.processCommandLine(&parser);

    const QString promptTypeString = qEnvironmentVariable(PROMPT_TYPE_ENV_VAR);
    PromptType promptType = PromptType::Entry;
    if (promptTypeString == QLatin1String("confirm")) {
        promptType = PromptType::Confirm;
    } else if (promptTypeString == QLatin1String("none")) {
        promptType = PromptType::None;
    }

    const QString walletFolder = app.applicationName();
    QString dialog = i18n("Please enter passphrase"); // Default dialog text.
    QString identifier;
    QString item;
    bool ignoreWallet = false;
    DisplayType displayType = DisplayType::Password;

    // Parse commandline arguments
    if (!parser.positionalArguments().isEmpty()) {
        dialog = parser.positionalArguments().at(0);
        parsePrompt(promptType, dialog, identifier, ignoreWallet, displayType);
    }

    // Open KWallet to see if an item was previously stored
    std::unique_ptr<KWallet::Wallet> wallet(ignoreWallet ? nullptr : KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0));

    if ((!ignoreWallet) && (!identifier.isNull()) && wallet.get() && wallet->hasFolder(walletFolder)) {
        wallet->setFolder(walletFolder);

        wallet->readPassword(identifier, item);

        if (item.isEmpty()) {
            // There was a bug in previous versions of ksshaskpass that caused it to create keys with single quotes
            // around the identifier and even older versions have an extra space appended to the identifier.
            // key file name. Try these keys too, and, if there's a match, ensure that it's properly
            // replaced with proper one.
            for (auto templ : QStringList{QStringLiteral("'%0'"), QStringLiteral("%0 "), QStringLiteral("'%0' ")}) {
                const QString keyFile = templ.arg(identifier);
                wallet->readPassword(keyFile, item);
                if (!item.isEmpty()) {
                    qCWarning(LOG_KSSHASKPASS) << "Detected legacy key for " << identifier << ", enabling workaround";
                    wallet->renameEntry(keyFile, identifier);
                    break;
                }
            }
        }
    }

    if (!item.isEmpty()) {
        QTextStream(stdout) << item;
        return 0;
    }

    // Item could not be retrieved from wallet. Open dialog
    switch (displayType) {
    case DisplayType::ConfirmCancel: {
        cancelDialog(nullptr, dialog);
        // dialog can only be canceled
        return 1;
    }
    case DisplayType::Confirm: {
        if (KMessageBox::questionTwoActions(nullptr,
                                            dialog,
                                            i18nc("@title:window", "Enter SSH Credentials"),
                                            KGuiItem(i18nc("@action:button", "Accept"), QStringLiteral("dialog-ok")),
                                            KStandardGuiItem::cancel())
            != KMessageBox::PrimaryAction) {
            // dialog has been canceled
            return 1;
        }
        item = QStringLiteral("yes\n");
        break;
    }
    case DisplayType::ClearText:
        // Should use a dialog with visible input, but KPasswordDialog doesn't support that and
        // other available dialog types don't have a "Keep" checkbox.
        /* fallthrough */
    case DisplayType::Password: {
        // create the password dialog, but only show "Enable Keep" button, if the wallet is open
        KPasswordDialog::KPasswordDialogFlag flag(KPasswordDialog::NoFlags);
        if (wallet.get()) {
            flag = KPasswordDialog::ShowKeepPassword;
        }
        QPointer<KPasswordDialog> kpd = new KPasswordDialog(nullptr, flag);

        kpd->setPrompt(dialog);
        kpd->setWindowTitle(i18nc("@title:window", "Enter SSH Credentials"));
        // We don't want to dump core when the password dialog is shown, because it could contain the entered password.
        // KPasswordDialog::disableCoreDumps() seems to be gone in KDE 4 -- do it manually
        struct rlimit rlim;
        rlim.rlim_cur = rlim.rlim_max = 0;
        setrlimit(RLIMIT_CORE, &rlim);

        if (kpd->exec() == QDialog::Accepted) {
            item = kpd->password();
            // If "Enable Keep" is enabled, open/create a folder in KWallet and store the password.
            if ((!identifier.isNull()) && wallet.get() && kpd->keepPassword()) {
                if (!wallet->hasFolder(walletFolder)) {
                    wallet->createFolder(walletFolder);
                }
                wallet->setFolder(walletFolder);
                wallet->writePassword(identifier, item);
            }
        } else {
            // dialog has been canceled
            return 1;
        }
        break;
    }
    }

    QTextStream out(stdout);
    out << item << "\n";
    return 0;
}
