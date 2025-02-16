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

#include "../kxmlgui_version.h"

namespace KDEPrivate
{
KAboutKdeDialog::KAboutKdeDialog(QWidget *parent)
    : QDialog(parent)
    , d(nullptr)
{
    setWindowTitle(i18nc("@title:window", "About Lingmo OS"));

    KTitleWidget *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("<html><font size=\"6\">Lingmo OS</font></html>"));
    titleWidget->setIconSize(QSize(48, 48));
    titleWidget->setIcon(QIcon::fromTheme(QStringLiteral("systemlogo")), KTitleWidget::ImageLeft);

    QLabel *about = new QLabel;
    about->setMargin(10);
    about->setAlignment(Qt::AlignTop);
    about->setWordWrap(true);
    about->setOpenExternalLinks(true);
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->setText(
        i18n("<html>"
             "<b>Lingmo OS</b> is a operating system based on <a href=\"%1\">Debian GNU/Linux</a>. "
             "It offers a beautiful Desktop environment, includes software tailored for this environment, "
             "and features a unified UI style.</html>",
             QStringLiteral("https://www.debian.org/")));

    QLabel *report = new QLabel;
    report->setMargin(10);
    report->setAlignment(Qt::AlignTop);
    report->setWordWrap(true);
    report->setOpenExternalLinks(true);
    report->setTextInteractionFlags(Qt::TextBrowserInteraction);
    report->setText(
        i18n("<html>"
             "When using Lingmo OS, you may encounter some problems or defects. "
             "Since the Lingmo OS team is small, bugs are inevitable. "
             "You can report these problems on our <a href=\"%1\">Github</a>.</html>",
             QStringLiteral("https://github.com/LingmoOS/LingmoOS/issues")));

    QLabel *join = new QLabel;
    join->setMargin(10);
    join->setAlignment(Qt::AlignTop);
    join->setWordWrap(true);
    join->setOpenExternalLinks(true);
    join->setTextInteractionFlags(Qt::TextBrowserInteraction);
    join->setText(
        i18n("<html>"
             "If you are interested in the Lingmo OS project or want to participate in its development,"
             "you can contribute code on our <a href=\"%1\">Github</a>.</html>",
             QStringLiteral("https://github.com/LingmoOS/")));

    QLabel *aboutkde = new QLabel;
    aboutkde->setMargin(10);
    aboutkde->setAlignment(Qt::AlignTop);
    aboutkde->setWordWrap(true);
    aboutkde->setOpenExternalLinks(true);
    aboutkde->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutkde->setText(
        i18n("<html>"
             "The source code of <b><a href=\"%1\">Lingmo OS</a></b> can be found on its <a href=\"%2\">Github</a>. <br /><br />"
             "The desktop of Lingmo OS uses some KDE components, so thanks to the KDE community.<br /><br />"
             "<b>KDE</b> is a world-wide community of software engineers, artists, writers, "
             "translators and creators who are committed to <a href=\"%3\">Free Software</a> "
             "development. KDE produces the Plasma desktop environment, hundreds of applications, "
             "and the many software libraries that support them.<br /><br />"
             "KDE is a cooperative enterprise: no single entity controls its direction or products. "
             "Instead, we work together to achieve the common goal of building the world's finest "
             "Free Software. Everyone is welcome to <a href=\"%4\">join and contribute</a> to KDE, including you.<br /><br />"
             "Visit <a href=\"%5\">%5</a> for "
             "more information about the KDE community and the software we produce.</html>",
             QStringLiteral("https://lingmo.org/"),
             QStringLiteral("https://github.com/LingmoOS/"),
             QStringLiteral("https://www.gnu.org/philosophy/free-sw.html"),
             QStringLiteral("https://community.kde.org/Get_Involved"),
             QStringLiteral("https://www.kde.org/")));

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);
    tabWidget->addTab(about, i18nc("@title:tab About Lingmo OS", "&About"));
    tabWidget->addTab(report, i18nc("@title:tab", "&Report Bugs or Wishes"));
    tabWidget->addTab(join, i18nc("@title:tab", "&Join Lingmo OS"));
    tabWidget->addTab(aboutkde, i18nc("@title:tab", "&About KDE"));

    QHBoxLayout *midLayout = new QHBoxLayout;
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
