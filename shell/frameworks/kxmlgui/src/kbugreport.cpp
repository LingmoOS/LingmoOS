/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbugreport.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QUrlQuery>

#include <KAboutData>
#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageDialog>
#include <KTitleWidget>

#include "config-xmlgui.h"
#include "systeminformation_p.h"
#include <kxmlgui_version.h>

class KBugReportPrivate
{
public:
    KBugReportPrivate(KBugReport *qq)
        : q(qq)
    {
    }

    enum BugDestination {
        BugsKdeOrg,
        CustomUrl,
    };

    // Update the url to match the current OS, selected app, etc
    void updateUrl();

    KBugReport *const q;

    QLabel *m_version = nullptr;
    QString m_strVersion;

    QString lastError;
    QString kde_version;
    QString appname;
    QString os;
    QUrl url;
    BugDestination bugDestination = KBugReportPrivate::CustomUrl;
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

    const QString bugAddress = aboutData.bugAddress();
    if (bugAddress == QLatin1String("submit@bugs.kde.org")) {
        // This is a core KDE application -> redirect to the web form
        d->bugDestination = KBugReportPrivate::BugsKdeOrg;
    } else if (!QUrl(bugAddress).scheme().isEmpty()) {
        // The bug reporting address is a URL -> redirect to that
        d->bugDestination = KBugReportPrivate::CustomUrl;
    }

    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::close());

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

    // Program name
    QString qwtstr =
        i18n("The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application");
    tmpLabel = new QLabel(i18n("Application: "), this);
    glay->addWidget(tmpLabel, row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    QLabel *appLabel = new QLabel(this);
    d->appname = aboutData.productName();
    appLabel->setText(d->appname);
    glay->addWidget(appLabel, row, 1);
    tmpLabel->setWhatsThis(qwtstr);

    // Version
    qwtstr = i18n("The version of this application - please make sure that no newer version is available before sending a bug report");
    tmpLabel = new QLabel(i18n("Version:"), this);
    glay->addWidget(tmpLabel, ++row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    d->m_strVersion = aboutData.version();
    if (d->m_strVersion.isEmpty()) {
        d->m_strVersion = i18n("no version set (programmer error)");
    }
    d->kde_version = QStringLiteral(KXMLGUI_VERSION_STRING);
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
    // Point to the web form

    QString text;
    if (d->bugDestination == KBugReportPrivate::BugsKdeOrg) {
        text = i18n(
            "<qt>To submit a bug report, click on the button below. This will open a web browser "
            "window on <a href=\"https://bugs.kde.org\">https://bugs.kde.org</a> where you will find "
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

    lay->addWidget(buttonBox);
    setMinimumHeight(sizeHint().height() + 20); // WORKAROUND: prevent "cropped" qcombobox
}

KBugReport::~KBugReport() = default;

void KBugReportPrivate::updateUrl()
{
    url = QUrl(QStringLiteral("https://bugs.kde.org/enter_bug.cgi"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("guided")); // use the guided form

    // the string format is product/component, where component is optional
    QStringList list = appname.split(QLatin1Char('/'));
    query.addQueryItem(QStringLiteral("product"), list[0]);
    if (list.size() == 2) {
        query.addQueryItem(QStringLiteral("component"), list[1]);
    }

    query.addQueryItem(QStringLiteral("version"), m_strVersion);
    url.setQuery(query);

    // TODO: guess and fill OS(sys_os) and Platform(rep_platform) fields
}

void KBugReport::accept()
{
    QDesktopServices::openUrl(d->url);
    QDialog::accept();
}

#include "moc_kbugreport.cpp"
