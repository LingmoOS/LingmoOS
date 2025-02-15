/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>
    SPDX-FileCopyrightText: 2008, 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    Parts of this class have been take from the KAboutApplication class, which was
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kabstractaboutdialog_p.h"

#include "../kxmlgui_version.h"
#include "kaboutapplicationcomponentlistdelegate_p.h"
#include "kaboutapplicationcomponentmodel_p.h"
#include "kaboutapplicationlistview_p.h"
#include "kaboutapplicationpersonlistdelegate_p.h"
#include "kaboutapplicationpersonmodel_p.h"
#include "klicensedialog_p.h"
// KF
#include <KLocalizedString>
#include <KTitleWidget>
// Qt
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

QWidget *KAbstractAboutDialogPrivate::createTitleWidget(const QIcon &icon, const QString &displayName, const QString &version, QWidget *parent)
{
    KTitleWidget *titleWidget = new KTitleWidget(parent);

    titleWidget->setIconSize(QSize(48, 48));
    titleWidget->setIcon(icon, KTitleWidget::ImageLeft);
    titleWidget->setText(
        QLatin1String("<html><font size=\"5\">%1</font><br />%2</html>").arg(displayName, i18nc("Version version-number", "Version %1", version)));
    return titleWidget;
}

QWidget *KAbstractAboutDialogPrivate::createAboutWidget(const QString &shortDescription,
                                                        const QString &otherText,
                                                        const QString &copyrightStatement,
                                                        const QString &homepage,
                                                        const QList<KAboutLicense> &licenses,
                                                        QWidget *parent)
{
    QWidget *aboutWidget = new QWidget(parent);
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutWidget);

    QString aboutPageText = shortDescription + QLatin1Char('\n');

    if (!otherText.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + otherText + QLatin1Char('\n');
    }

    if (!copyrightStatement.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + copyrightStatement + QLatin1Char('\n');
    }

    if (!homepage.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + QStringLiteral("<a href=\"%1\">%1</a>").arg(homepage) + QLatin1Char('\n');
    }
    aboutPageText = aboutPageText.trimmed();

    QLabel *aboutLabel = new QLabel;
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setText(aboutPageText.replace(QLatin1Char('\n'), QStringLiteral("<br />")));
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    aboutLayout->addStretch();
    aboutLayout->addWidget(aboutLabel);

    const int licenseCount = licenses.count();
    for (int i = 0; i < licenseCount; ++i) {
        const KAboutLicense &license = licenses.at(i);

        QLabel *showLicenseLabel = new QLabel;
        showLicenseLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>").arg(QString::number(i), i18n("License: %1", license.name(KAboutLicense::FullName))));
        showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        QObject::connect(showLicenseLabel, &QLabel::linkActivated, parent, [license, parent]() {
            auto *dialog = new KLicenseDialog(license, parent);
            dialog->show();
        });

        aboutLayout->addWidget(showLicenseLabel);
    }

    aboutLayout->addStretch();

    return aboutWidget;
}

QWidget *KAbstractAboutDialogPrivate::createComponentWidget(const QList<KAboutComponent> &components, QWidget *parent)
{
    QWidget *componentWidget = new QWidget(parent);
    QVBoxLayout *componentLayout = new QVBoxLayout(componentWidget);
    componentLayout->setContentsMargins(0, 0, 0, 0);

    QList<KAboutComponent> allComponents = components;
    allComponents.prepend(KAboutComponent(i18n("The <em>%1</em> windowing system", QGuiApplication::platformName())));
    allComponents.prepend(KAboutComponent(i18n("Qt"),
                                          QString(),
                                          i18n("%1 (built against %2)", QString::fromLocal8Bit(qVersion()), QStringLiteral(QT_VERSION_STR)),
                                          QStringLiteral("https://www.qt.io/")));
    allComponents.prepend(KAboutComponent(i18n("KDE Frameworks"),
                                          QString(),
                                          QStringLiteral(KXMLGUI_VERSION_STRING),
                                          QStringLiteral("https://develop.kde.org/products/frameworks/")));

    KDEPrivate::KAboutApplicationComponentModel *componentModel = new KDEPrivate::KAboutApplicationComponentModel(allComponents, componentWidget);

    KDEPrivate::KAboutApplicationListView *componentView = new KDEPrivate::KAboutApplicationListView(componentWidget);

    KDEPrivate::KAboutApplicationComponentListDelegate *componentDelegate =
        new KDEPrivate::KAboutApplicationComponentListDelegate(componentView, componentView);

    componentView->setModel(componentModel);
    componentView->setItemDelegate(componentDelegate);
    componentView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    componentLayout->addWidget(componentView);

    return componentWidget;
}

static QWidget *createAvatarCheck(QWidget *parent, KDEPrivate::KAboutApplicationPersonModel *model)
{
    // Add in a checkbox to allow people to switch the avatar fetch
    // (off-by-default to avoid unwarned online activity)
    QCheckBox *avatarsCheck = new QCheckBox(parent);
    avatarsCheck->setText(i18n("Show author photos"));
    avatarsCheck->setToolTip(i18n("Enabling this will fetch images from an online location"));
    avatarsCheck->setVisible(model->hasAnyAvatars());
    QObject::connect(model, &KDEPrivate::KAboutApplicationPersonModel::hasAnyAvatarsChanged, parent, [avatarsCheck, model]() {
        avatarsCheck->setVisible(model->hasAnyAvatars());
    });
    QObject::connect(avatarsCheck, &QCheckBox::stateChanged, parent, [model](int state) {
        switch (state) {
        case Qt::Checked:
        case Qt::PartiallyChecked:
            // tell model to use avatars
            model->setShowRemoteAvatars(true);
            break;
        case Qt::Unchecked:
        default:
            // tell model not to use avatars
            model->setShowRemoteAvatars(false);
            break;
        }
    });
    return avatarsCheck;
}

QWidget *KAbstractAboutDialogPrivate::createAuthorsWidget(const QList<KAboutPerson> &authors,
                                                          bool customAuthorTextEnabled,
                                                          const QString &customAuthorRichText,
                                                          const QString &bugAddress,
                                                          QWidget *parent)
{
    QWidget *authorWidget = new QWidget(parent);
    QVBoxLayout *authorLayout = new QVBoxLayout(authorWidget);
    authorLayout->setContentsMargins(0, 0, 0, 0);

    if (!customAuthorTextEnabled || !customAuthorRichText.isEmpty()) {
        QLabel *bugsLabel = new QLabel(authorWidget);
        bugsLabel->setContentsMargins(4, 2, 0, 4);
        bugsLabel->setOpenExternalLinks(true);
        if (!customAuthorTextEnabled) {
            if (bugAddress.isEmpty() || bugAddress == QLatin1String("submit@bugs.kde.org")) {
                bugsLabel->setText(i18nc("Reference to website",
                                         "Please use %1 to report bugs.\n",
                                         QLatin1String("<a href=\"https://bugs.kde.org\">https://bugs.kde.org</a>")));
            } else {
                QUrl bugUrl(bugAddress);
                if (bugUrl.scheme().isEmpty()) {
                    bugUrl.setScheme(QStringLiteral("mailto"));
                }
                bugsLabel->setText(i18nc("Reference to email address",
                                         "Please report bugs to %1.\n",
                                         QLatin1String("<a href=\"%1\">%2</a>").arg(bugUrl.toString(), bugAddress)));
            }
        } else {
            bugsLabel->setText(customAuthorRichText);
        }
        bugsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        authorLayout->addWidget(bugsLabel);
    }

    KDEPrivate::KAboutApplicationPersonModel *authorModel = new KDEPrivate::KAboutApplicationPersonModel(authors, authorWidget);

    KDEPrivate::KAboutApplicationListView *authorView = new KDEPrivate::KAboutApplicationListView(authorWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *authorDelegate = new KDEPrivate::KAboutApplicationPersonListDelegate(authorView, authorView);

    authorView->setModel(authorModel);
    authorView->setItemDelegate(authorDelegate);
    authorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    authorLayout->addWidget(createAvatarCheck(parent, authorModel));
    authorLayout->addWidget(authorView);

    return authorWidget;
}

QWidget *KAbstractAboutDialogPrivate::createCreditWidget(const QList<KAboutPerson> &credits, QWidget *parent)
{
    QWidget *creditWidget = new QWidget(parent);
    QVBoxLayout *creditLayout = new QVBoxLayout(creditWidget);
    creditLayout->setContentsMargins(0, 0, 0, 0);

    KDEPrivate::KAboutApplicationPersonModel *creditModel = new KDEPrivate::KAboutApplicationPersonModel(credits, creditWidget);

    KDEPrivate::KAboutApplicationListView *creditView = new KDEPrivate::KAboutApplicationListView(creditWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *creditDelegate = new KDEPrivate::KAboutApplicationPersonListDelegate(creditView, creditView);

    creditView->setModel(creditModel);
    creditView->setItemDelegate(creditDelegate);
    creditView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    creditLayout->addWidget(createAvatarCheck(parent, creditModel));
    creditLayout->addWidget(creditView);

    return creditWidget;
}

QWidget *KAbstractAboutDialogPrivate::createTranslatorsWidget(const QList<KAboutPerson> &translators, QWidget *parent)
{
    QWidget *translatorWidget = new QWidget(parent);
    QVBoxLayout *translatorLayout = new QVBoxLayout(translatorWidget);
    translatorLayout->setContentsMargins(0, 0, 0, 0);

    KDEPrivate::KAboutApplicationPersonModel *translatorModel = new KDEPrivate::KAboutApplicationPersonModel(translators, translatorWidget);

    KDEPrivate::KAboutApplicationListView *translatorView = new KDEPrivate::KAboutApplicationListView(translatorWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *translatorDelegate = new KDEPrivate::KAboutApplicationPersonListDelegate(translatorView, translatorView);

    translatorView->setModel(translatorModel);
    translatorView->setItemDelegate(translatorDelegate);
    translatorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    translatorLayout->addWidget(createAvatarCheck(parent, translatorModel));
    translatorLayout->addWidget(translatorView);

    QString aboutTranslationTeam = KAboutData::aboutTranslationTeam();
    if (!aboutTranslationTeam.isEmpty()) {
        QLabel *translationTeamLabel = new QLabel(translatorWidget);
        translationTeamLabel->setContentsMargins(4, 2, 4, 4);
        translationTeamLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        translationTeamLabel->setWordWrap(true);
        translationTeamLabel->setText(aboutTranslationTeam);
        translationTeamLabel->setOpenExternalLinks(true);
        translatorLayout->addWidget(translationTeamLabel);
        // TODO: this could be displayed as a view item to save space
    }

    return translatorWidget;
}

void KAbstractAboutDialogPrivate::createForm(QWidget *titleWidget, QWidget *tabWidget, QDialog *dialog)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // And we jam everything together in a layout...
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
}
