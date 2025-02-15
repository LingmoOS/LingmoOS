/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbugreport.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QUrl>
#include <QUrlQuery>

#include <KAboutData>
#include <KConfig>
#include <KEMailSettings>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageDialog>
#include <KTitleWidget>

#include "../kxmlgui_version.h"
#include "config-xmlgui.h"
#include "debug.h"
#include "systeminformation_p.h"

#include <array>

class KBugReportPrivate
{
public:
    KBugReportPrivate(KBugReport *qq)
        : q(qq)
        , m_aboutData(KAboutData::applicationData())
    {
    }

    enum BugDestination {
        BugsKdeOrg,
        CustomEmail,
        CustomUrl,
    };

    // Calls kcmshell5 System/email
    void slotConfigureEmail();

    // Sets the "From" field from the e-mail configuration.
    // Called at creation time, but also after "Configure email" is closed.
    void slotSetFrom();

    // Update the url to match the current OS, selected app, etc
    void updateUrl();

    KBugReport *const q;
    QProcess *m_process = nullptr;
    KAboutData m_aboutData;

    QTextEdit *m_lineedit = nullptr;
    QLineEdit *m_subject = nullptr;
    QLabel *m_from = nullptr;
    QLabel *m_version = nullptr;
    QString m_strVersion;
    QGroupBox *m_bgSeverity = nullptr;
    QPushButton *m_configureEmail = nullptr;

    QString lastError;
    QString kde_version;
    QString appname;
    QString os;
    QUrl url;
    QList<QRadioButton *> severityButtons;
    int currentSeverity() const
    {
        for (int i = 0; i < severityButtons.count(); i++) {
            if (severityButtons[i]->isChecked()) {
                return i;
            }
        }
        return -1;
    }
    BugDestination bugDestination;
};

KBugReport::KBugReport(const KAboutData &aboutData, QWidget *_parent)
    : QDialog(_parent)
    , d(new KBugReportPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Submit Bug Report"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    d->m_aboutData = aboutData;
    d->m_process = nullptr;
    d->bugDestination = KBugReportPrivate::CustomEmail;

    const QString bugAddress = d->m_aboutData.bugAddress();
    if (bugAddress == QLatin1String("team@lingmo.org")) {
        // 这是一个 LingmoOS 核心应用 -> 重定向到 GitHub Issues
        d->bugDestination = KBugReportPrivate::BugsKdeOrg;
    } else if (!QUrl(bugAddress).scheme().isEmpty()) {
        // bug 报告地址是一个 URL -> 重定向到该地址
        d->bugDestination = KBugReportPrivate::CustomUrl;
    }

    if (d->bugDestination != KBugReportPrivate::CustomEmail) {
        KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::close());
    }

    QLabel *tmpLabel;
    QVBoxLayout *lay = new QVBoxLayout(this);

    KTitleWidget *title = new KTitleWidget(this);
    title->setText(i18n("Submit Bug Report"));
    title->setIconSize(QSize(32, 32));
    title->setIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug")));
    lay->addWidget(title);

    QGridLayout *glay = new QGridLayout();
    lay->addLayout(glay);

    int row = 0;

    if (d->bugDestination == KBugReportPrivate::CustomEmail) {
        // From
        QString qwtstr = i18n("Your email address. If incorrect, use the Configure Email button to change it");
        tmpLabel = new QLabel(i18nc("Email sender address", "From:"), this);
        glay->addWidget(tmpLabel, row, 0);
        tmpLabel->setWhatsThis(qwtstr);
        d->m_from = new QLabel(this);
        glay->addWidget(d->m_from, row, 1);
        d->m_from->setWhatsThis(qwtstr);

        // Configure email button
        d->m_configureEmail = new QPushButton(i18nc("@action:button", "Configure Email..."), this);
        connect(d->m_configureEmail, &QPushButton::clicked, this, [this]() {
            d->slotConfigureEmail();
        });
        glay->addWidget(d->m_configureEmail, 0, 2, 3, 1, Qt::AlignTop | Qt::AlignRight);

        // To
        qwtstr = i18n("The email address this bug report is sent to.");
        tmpLabel = new QLabel(i18nc("Email receiver address", "To:"), this);
        glay->addWidget(tmpLabel, ++row, 0);
        tmpLabel->setWhatsThis(qwtstr);
        tmpLabel = new QLabel(d->m_aboutData.bugAddress(), this);
        tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        glay->addWidget(tmpLabel, row, 1);
        tmpLabel->setWhatsThis(qwtstr);

        KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok),
                         KGuiItem(i18nc("@action:button", "&Send"),
                                  QStringLiteral("mail-send"),
                                  i18nc("@info:tooltip", "Send bug report."),
                                  i18nc("@info:whatsthis", "Send this bug report to %1.", d->m_aboutData.bugAddress())));
        row++;
    } else {
        d->m_configureEmail = nullptr;
        d->m_from = nullptr;
    }

    // Program name
    QString qwtstr =
        i18n("The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application");
    tmpLabel = new QLabel(i18n("Application: "), this);
    glay->addWidget(tmpLabel, row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    QLabel *appLabel = new QLabel(this);
    d->appname = d->m_aboutData.productName();
    appLabel->setText(d->appname);
    glay->addWidget(appLabel, row, 1);
    tmpLabel->setWhatsThis(qwtstr);

    // Version
    qwtstr = i18n("The version of this application - please make sure that no newer version is available before sending a bug report");
    tmpLabel = new QLabel(i18n("Version:"), this);
    glay->addWidget(tmpLabel, ++row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    d->m_strVersion = d->m_aboutData.version();
    if (d->m_strVersion.isEmpty()) {
        d->m_strVersion = i18n("no version set (programmer error)");
    }
    d->kde_version = QStringLiteral(KXMLGUI_VERSION_STRING) + QLatin1String(", ") + QStringLiteral(XMLGUI_DISTRIBUTION_TEXT);
    if (d->bugDestination != KBugReportPrivate::BugsKdeOrg) {
        d->m_strVersion += QLatin1Char(' ') + d->kde_version;
    }
    d->m_version = new QLabel(d->m_strVersion, this);
    d->m_version->setTextInteractionFlags(Qt::TextBrowserInteraction);
    // glay->addWidget( d->m_version, row, 1 );
    glay->addWidget(d->m_version, row, 1, 1, 2);
    d->m_version->setWhatsThis(qwtstr);

    tmpLabel = new QLabel(i18n("OS:"), this);
    glay->addWidget(tmpLabel, ++row, 0);

#ifdef Q_OS_WINDOWS
    d->os = i18nc("%1 is the operating system name, e.g. 'Windows 10', %2 is the CPU architecture, e.g. 'x86_64'",
                  "%1 (%2)",
                  QSysInfo::prettyProductName(),
                  QSysInfo::currentCpuArchitecture());
#else
    if (QSysInfo::productVersion() != QLatin1String("unknown")) {
        d->os = i18nc(
            "%1 is the operating system name, e.g. 'Fedora Linux', %2 is the operating system version, e.g. '35', %3 is the CPU architecture, e.g. 'x86_64'",
            "%1 %2 (%3)",
            QSysInfo::prettyProductName(),
            QSysInfo::productVersion(),
            QSysInfo::currentCpuArchitecture());
    } else {
        d->os = i18nc("%1 is the operating system name, e.g. 'Fedora Linux', %2 is the CPU architecture, e.g. 'x86_64'",
                      "%1 (%2)",
                      QSysInfo::prettyProductName(),
                      QSysInfo::currentCpuArchitecture());
    }
#endif

    tmpLabel = new QLabel(d->os, this);
    tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    glay->addWidget(tmpLabel, row, 1, 1, 2);

    if (d->bugDestination == KBugReportPrivate::CustomEmail) {
        // Severity
        d->m_bgSeverity = new QGroupBox(i18nc("@title:group", "Se&verity"), this);

        struct SeverityData {
            QString name;
            QString text;
        };
        const std::array<SeverityData, 5> severityData = {{
            {QStringLiteral("critical"), i18nc("bug severity", "Critical")},
            {QStringLiteral("grave"), i18nc("bug severity", "Grave")},
            {QStringLiteral("normal"), i18nc("bug severity", "Normal")},
            {QStringLiteral("wishlist"), i18nc("bug severity", "Wishlist")},
            {QStringLiteral("i18n"), i18nc("bug severity", "Translation")},
        }};

        QHBoxLayout *severityLayout = new QHBoxLayout(d->m_bgSeverity);
        for (auto &severityDatum : severityData) {
            // Store the severity string as the name
            QRadioButton *rb = new QRadioButton(severityDatum.text, d->m_bgSeverity);
            rb->setObjectName(severityDatum.name);
            d->severityButtons.append(rb);
            severityLayout->addWidget(rb);
        }
        d->severityButtons[2]->setChecked(true); // default : "normal"

        lay->addWidget(d->m_bgSeverity);

        // Subject
        QHBoxLayout *hlay = new QHBoxLayout();
        lay->addItem(hlay);
        tmpLabel = new QLabel(i18n("S&ubject: "), this);
        hlay->addWidget(tmpLabel);
        d->m_subject = new QLineEdit(this);
        d->m_subject->setClearButtonEnabled(true);
        d->m_subject->setFocus();
        tmpLabel->setBuddy(d->m_subject);
        hlay->addWidget(d->m_subject);

        QString text = i18n(
            "Enter the text (in English if possible) that you wish to submit for the "
            "bug report.\n"
            "If you press \"Send\", a mail message will be sent to the maintainer of "
            "this program.\n");
        QLabel *label = new QLabel(this);

        label->setText(text);
        lay->addWidget(label);

        // The multiline-edit
        d->m_lineedit = new QTextEdit(this);
        d->m_lineedit->setMinimumHeight(180); // make it big
        d->m_lineedit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        d->m_lineedit->setLineWrapMode(QTextEdit::WidgetWidth);
        lay->addWidget(d->m_lineedit, 10 /*stretch*/);

        d->slotSetFrom();
    } else {
        // Point to the web form

        QString text;
        if (d->bugDestination == KBugReportPrivate::BugsKdeOrg) {
            text = i18n(
                "<qt>To submit a bug report, click on the button below. This will open a web browser "
                "window on <a href=\"https://github.com/LingmoOS/LingmoOS/issues\">https://github.com/LingmoOS/LingmoOS/issues</a> where you will find "
                "a form to fill in. The information displayed above will be transferred to that server.</qt>");
            d->updateUrl();
        } else {
            text = i18n(
                "<qt>To submit a bug report, click on the button below. This will open a web browser "
                "window on <a href=\"%1\">%2</a>.</qt>",
                bugAddress,
                bugAddress);
            d->url = QUrl(bugAddress);
        }

        lay->addSpacing(10);
        QLabel *label = new QLabel(text, this);
        label->setOpenExternalLinks(true);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
        label->setWordWrap(true);
        lay->addWidget(label);
        lay->addSpacing(10);

        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        if (d->bugDestination == KBugReportPrivate::BugsKdeOrg) {
            okButton->setText(i18nc("@action:button", "&Launch Bug Report Wizard"));
        } else {
            okButton->setText(i18nc("@action:button", "&Submit Bug Report"));
        }
        okButton->setIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug")));
    }

    lay->addWidget(buttonBox);
    setMinimumHeight(sizeHint().height() + 20); // WORKAROUND: prevent "cropped" qcombobox
}

KBugReport::~KBugReport() = default;

QString KBugReport::messageBody() const
{
    if (d->bugDestination == KBugReportPrivate::CustomEmail) {
        return d->m_lineedit->toPlainText();
    } else {
        return QString();
    }
}

void KBugReport::setMessageBody(const QString &messageBody)
{
    if (d->bugDestination == KBugReportPrivate::CustomEmail) {
        d->m_lineedit->setPlainText(messageBody);
    }
}

void KBugReportPrivate::updateUrl()
{
    url = QUrl(QStringLiteral("https://github.com/LingmoOS/LingmoOS/issues"));

    // TODO: guess and fill OS(sys_os) and Platform(rep_platform) fields
}

void KBugReportPrivate::slotConfigureEmail()
{
    if (m_process) {
        return;
    }

    const QString exec = QStandardPaths::findExecutable(QStringLiteral("kcmshell5"));
    if (exec.isEmpty()) {
        const QString msg = i18nc("The second arg is 'kde-cli-tools' which is the package that contains kcmshell5 (the first arg)",
                                  "Could not find <application>%1</application> executable (usually it's part of the \"%2\" package).",
                                  QStringLiteral("kcmshell5"),
                                  QStringLiteral("kde-cli-tools"));
        auto *dlg = new KMessageDialog(KMessageDialog::Error, msg, q);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(true);
        dlg->show();
        return;
    }

    m_process = new QProcess;
    QObject::connect(m_process, &QProcess::finished, q, [this]() {
        slotSetFrom();
    });
    m_process->start(exec, QStringList{QStringLiteral("kcm_users")});
    if (!m_process->waitForStarted()) {
        // qCDebug(DEBUG_KXMLGUI) << "Couldn't start kcmshell5..";
        delete m_process;
        m_process = nullptr;
        return;
    }
    m_configureEmail->setEnabled(false);
}

void KBugReportPrivate::slotSetFrom()
{
    delete m_process;
    m_process = nullptr;
    m_configureEmail->setEnabled(true);

    KEMailSettings emailSettings;
    QString fromaddr = emailSettings.getSetting(KEMailSettings::EmailAddress);
    if (fromaddr.isEmpty()) {
        fromaddr = SystemInformation::userName();
    } else {
        QString name = emailSettings.getSetting(KEMailSettings::RealName);
        if (!name.isEmpty()) {
            fromaddr = name + QLatin1String(" <") + fromaddr + QLatin1Char('>');
        }
    }
    m_from->setText(fromaddr);
}

void KBugReport::accept()
{
    if (d->bugDestination != KBugReportPrivate::CustomEmail) {
        QDesktopServices::openUrl(d->url);
        return;
    }

    if (d->m_lineedit->toPlainText().isEmpty() || d->m_subject->text().isEmpty()) {
        QString msg = i18n(
            "You must specify both a subject and a description "
            "before the report can be sent.");
        KMessageBox::error(this, msg);
        return;
    }

    switch (d->currentSeverity()) {
    case 0: // critical
        if (KMessageBox::questionTwoActions(this,
                                            i18n("<p>You chose the severity <b>Critical</b>. "
                                                 "Please note that this severity is intended only for bugs that:</p>"
                                                 "<ul><li>break unrelated software on the system (or the whole system)</li>"
                                                 "<li>cause serious data loss</li>"
                                                 "<li>introduce a security hole on the system where the affected package is installed</li></ul>\n"
                                                 "<p>Does the bug you are reporting cause any of the above damage? "
                                                 "If it does not, please select a lower severity. Thank you.</p>"),
                                            QString(),
                                            KStandardGuiItem::cont(),
                                            KStandardGuiItem::cancel())
            == KMessageBox::SecondaryAction) {
            return;
        }
        break;
    case 1: // grave
        if (KMessageBox::questionTwoActions(
                this,
                i18n("<p>You chose the severity <b>Grave</b>. "
                     "Please note that this severity is intended only for bugs that:</p>"
                     "<ul><li>make the package in question unusable or mostly so</li>"
                     "<li>cause data loss</li>"
                     "<li>introduce a security hole allowing access to the accounts of users who use the affected package</li></ul>\n"
                     "<p>Does the bug you are reporting cause any of the above damage? "
                     "If it does not, please select a lower severity. Thank you.</p>"),
                QString(),
                KStandardGuiItem::cont(),
                KStandardGuiItem::cancel())
            == KMessageBox::SecondaryAction) {
            return;
        }
        break;
    default:
        break;
    }
    if (!sendBugReport()) {
        QString msg = i18n(
            "Unable to send the bug report.\n"
            "Please submit a bug report manually....\n"
            "See https://github.com/LingmoOS/LingmoOS/issues for instructions.");
        KMessageBox::error(this, msg + QLatin1String("\n\n") + d->lastError);
        return;
    }

    KMessageBox::information(this, i18n("Bug report sent, thank you for your input."));
    QDialog::accept();
}

void KBugReport::closeEvent(QCloseEvent *e)
{
    if (d->bugDestination == KBugReportPrivate::CustomEmail && ((d->m_lineedit->toPlainText().length() > 0) || d->m_subject->isModified())) {
        int rc = KMessageBox::warningTwoActions(this,
                                                i18n("Close and discard\nedited message?"),
                                                i18nc("@title:window", "Close Message"),
                                                KStandardGuiItem::discard(),
                                                KStandardGuiItem::cont());
        if (rc == KMessageBox::SecondaryAction) {
            e->ignore();
            return;
        }
    }
    QDialog::closeEvent(e);
}

QString KBugReport::text() const
{
    // qCDebug(DEBUG_KXMLGUI) << d->severityButtons[d->currentSeverity()]->objectName();
    // Prepend the pseudo-headers to the contents of the mail
    QString severity = d->severityButtons[d->currentSeverity()]->objectName();
    QString appname = d->appname;
    QString os = QStringLiteral("OS: %1 (%2)\n").arg(QStringLiteral(XMLGUI_COMPILING_OS), QStringLiteral(XMLGUI_DISTRIBUTION_TEXT));
    QString bodyText;
    /*  for(int i = 0; i < m_lineedit->numLines(); i++)
      {
         QString line = m_lineedit->textLine(i);
         if (!line.endsWith("\n"))
            line += '\n';
         bodyText += line;
      }
    */
    bodyText = d->m_lineedit->toPlainText();
    if (bodyText.length() > 0) {
        if (bodyText[bodyText.length() - 1] != QLatin1Char('\n')) {
            bodyText += QLatin1Char('\n');
        }
    }
    if (severity == QLatin1String("i18n") && QLocale().language() != QLocale::system().language()) {
        // Case 1 : i18n bug
        QString package = QLatin1String("i18n_") + QLocale::languageToString(QLocale().language());
        package.replace(QLatin1Char('_'), QLatin1Char('-'));
        /* clang-format off */
        return QLatin1String("Package: ") + package
            + QLatin1String("\nApplication: ") + appname
            + QLatin1String("\nVersion: ") + d->m_strVersion // not really i18n's version, so better here IMHO
            + QLatin1Char('\n') + os
            + QLatin1Char('\n') + bodyText;
        /* clang-format on */
    } else {
        appname.replace(QLatin1Char('_'), QLatin1Char('-'));
        // Case 2 : normal bug
        /* clang-format off */
        return QLatin1String("Package: ") + appname
            + QLatin1String("\nVersion: ") + d->m_strVersion
            + QLatin1String("\nSeverity: ") + severity
            + QLatin1Char('\n') + os
            + QLatin1Char('\n') + bodyText;
        /* clang-format on */
    }
}

bool KBugReport::sendBugReport()
{
    QString recipient = d->m_aboutData.bugAddress();
    if (recipient.isEmpty()) {
        recipient = QStringLiteral("team@lingmo.org");
    }

    QString command = QStandardPaths::findExecutable(QStringLiteral("ksendbugmail"));
    if (command.isEmpty()) {
        command = QFile::decodeName(CMAKE_INSTALL_PREFIX "/" KDE_INSTALL_LIBEXECDIR_KF "/ksendbugmail");
    }

    QProcess proc;
    QStringList args;
    args << QStringLiteral("--subject") << d->m_subject->text() << QStringLiteral("--recipient") << recipient;
    proc.start(command, args);
    // qCDebug(DEBUG_KXMLGUI) << command << args;
    if (!proc.waitForStarted()) {
        qCCritical(DEBUG_KXMLGUI) << "Unable to open a pipe to " << command;
        return false;
    }
    proc.write(text().toUtf8());
    proc.closeWriteChannel();

    proc.waitForFinished();
    // qCDebug(DEBUG_KXMLGUI) << "kbugreport: sendbugmail exit, status " << proc.exitStatus() << " code " << proc.exitCode();

    QByteArray line;
    if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() != 0) {
        // XXX not stderr?
        while (!proc.atEnd()) {
            line = proc.readLine();
        }
        d->lastError = QString::fromUtf8(line);
        return false;
    }
    return true;
}

#include "moc_kbugreport.cpp"
