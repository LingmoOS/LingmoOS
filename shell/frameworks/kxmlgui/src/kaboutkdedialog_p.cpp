/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>

    Parts of this class have been take from the KAboutKDE class, which was
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kaboutkdedialog_p.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KTitleWidget>

#include <kxmlgui_version.h>

namespace KDEPrivate
{
KAboutKdeDialog::KAboutKdeDialog(QWidget *parent)
    : QDialog(parent)
    , d(nullptr)
{
    setWindowTitle(i18nc("@title:window", "About KDE"));

    KTitleWidget *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("<html><font size=\"5\">KDE - Be Free!</font></html>"));
    titleWidget->setIconSize(QSize(48, 48));
    titleWidget->setIcon(QIcon::fromTheme(QStringLiteral("kde")), KTitleWidget::ImageLeft);

    QLabel *about = new QLabel;
    about->setMargin(10);
    about->setAlignment(Qt::AlignTop);
    about->setWordWrap(true);
    about->setOpenExternalLinks(true);
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->setText(
        i18n("<html>"
             "<b>KDE</b> is a world-wide community of software engineers, artists, writers, "
             "translators and creators who are committed to <a href=\"%1\">Free Software</a> "
             "development. KDE produces the Plasma desktop environment, hundreds of applications, "
             "and the many software libraries that support them.<br /><br />"
             "KDE is a cooperative enterprise: no single entity controls its direction or products. "
             "Instead, we work together to achieve the common goal of building the world's finest "
             "Free Software. Everyone is welcome to <a href=\"%2\">join and contribute</a> to KDE, including you.<br /><br />"
             "Visit <a href=\"%3\">%3</a> for "
             "more information about the KDE community and the software we produce.</html>",
             QStringLiteral("https://www.gnu.org/philosophy/free-sw.html"),
             QStringLiteral("https://community.kde.org/Get_Involved"),
             QStringLiteral("https://www.kde.org/")));

    QLabel *report = new QLabel;
    report->setMargin(10);
    report->setAlignment(Qt::AlignTop);
    report->setWordWrap(true);
    report->setOpenExternalLinks(true);
    report->setTextInteractionFlags(Qt::TextBrowserInteraction);
    report->setText(
        i18n("<html>"
             "Software can always be improved, and the KDE team is ready to "
             "do so. However, you - the user - must tell us when "
             "something does not work as expected or could be done better.<br /><br />"
             "KDE has a bug tracking system. Visit "
             "<a href=\"%1\">%1</a> or "
             "use the \"Report Bug...\" dialog from the \"Help\" menu to report bugs.<br /><br />"
             "If you have a suggestion for improvement then you are welcome to use "
             "the bug tracking system to register your wish. Make sure you use the "
             "severity called \"Wishlist\".</html>",
             QStringLiteral("https://bugs.kde.org/")));

    QLabel *join = new QLabel;
    join->setMargin(10);
    join->setAlignment(Qt::AlignTop);
    join->setWordWrap(true);
    join->setOpenExternalLinks(true);
    join->setTextInteractionFlags(Qt::TextBrowserInteraction);
    join->setText(
        i18n("<html>"
             "You do not have to be a software developer to be a member of the "
             "KDE team. You can join the language teams that translate "
             "program interfaces. You can provide graphics, themes, sounds, and "
             "improved documentation. You decide!"
             "<br /><br />"
             "Visit "
             "<a href=\"%1\">%1</a> "
             "for information on some projects in which you can participate."
             "<br /><br />"
             "If you need more information or documentation, then a visit to "
             "<a href=\"%2\">%2</a> "
             "will provide you with what you need.</html>",
             QStringLiteral("https://community.kde.org/Get_Involved"),
             QStringLiteral("https://develop.kde.org/")));

    QLabel *support = new QLabel;
    support->setMargin(10);
    support->setAlignment(Qt::AlignTop);
    support->setWordWrap(true);
    support->setOpenExternalLinks(true);
    support->setTextInteractionFlags(Qt::TextBrowserInteraction);
    support->setText(
        i18n("<html>"
             "KDE software is and will always be available free of charge, however creating it is not free.<br /><br />"
             "To support development the KDE community has formed the KDE e.V., a non-profit organization "
             "legally founded in Germany. KDE e.V. represents the KDE community in legal and financial matters. "
             "See <a href=\"%1\">%1</a>"
             " for information on KDE e.V.<br /><br />"
             "KDE benefits from many kinds of contributions, including financial. "
             "We use the funds to reimburse members and others for expenses "
             "they incur when contributing. Further funds are used for legal "
             "support and organizing conferences and meetings. <br /> <br />"
             "We would like to encourage you to support our efforts with a "
             "financial donation, using one of the ways described at "
             "<a href=\"%2\">%2</a>."
             "<br /><br />Thank you very much in advance for your support.</html>",
             QStringLiteral("https://ev.kde.org/"),
             QStringLiteral("https://www.kde.org/community/donations/")));
    support->setMinimumSize(support->sizeHint());

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);
    tabWidget->addTab(about, i18nc("@title:tab About KDE", "&About"));
    tabWidget->addTab(report, i18nc("@title:tab", "&Report Bugs or Wishes"));
    tabWidget->addTab(join, i18nc("@title:tab", "&Join KDE"));
    tabWidget->addTab(support, i18nc("@title:tab", "&Support KDE"));

    QLabel *image = new QLabel;
    QIcon icon(QStringLiteral(":/kxmlgui5/aboutkde.svg"));
    image->setPixmap(icon.pixmap(150, 250));

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(image);
    midLayout->addWidget(tabWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addLayout(midLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

}

#include "moc_kaboutkdedialog_p.cpp"
