/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2007 Krzysztof Lichota <lichota@mimuw.edu.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kswitchlanguagedialog_p.h"

#include "debug.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <private/qlocale_p.h>

#include <KLanguageButton>
#include <KLocalizedString>
#include <KMessageBox>

// Believe it or not we can't use KConfig from here
// (we need KConfig during QCoreApplication ctor which is too early for it)
// So we cooked a QSettings based solution
static std::unique_ptr<QSettings> localeOverridesSettings()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(QStringLiteral("."));
    }

    return std::make_unique<QSettings>(configPath + QLatin1String("/klanguageoverridesrc"), QSettings::IniFormat);
}

static QByteArray getApplicationSpecificLanguage(const QByteArray &defaultCode = QByteArray())
{
    std::unique_ptr<QSettings> settings = localeOverridesSettings();
    settings->beginGroup(QStringLiteral("Language"));
    return settings->value(qAppName(), defaultCode).toByteArray();
}

namespace KDEPrivate
{
Q_COREAPP_STARTUP_FUNCTION(initializeLanguages)

void setApplicationSpecificLanguage(const QByteArray &languageCode)
{
    std::unique_ptr<QSettings> settings = localeOverridesSettings();
    settings->beginGroup(QStringLiteral("Language"));

    if (languageCode.isEmpty()) {
        settings->remove(qAppName());
    } else {
        settings->setValue(qAppName(), languageCode);
    }
}

void initializeLanguages()
{
    const QByteArray languageCode = getApplicationSpecificLanguage();

    if (!languageCode.isEmpty()) {
        QByteArray languages = qgetenv("LANGUAGE");
        if (languages.isEmpty()) {
            qputenv("LANGUAGE", languageCode);
        } else {
            qputenv("LANGUAGE", languageCode + ':' + languages);
        }
        // Ideally setting the LANGUAGE would change the default QLocale too
        // but unfortunately this is too late since the QCoreApplication constructor
        // already created a QLocale at this stage so we need to set the reset it
        // by triggering the creation and destruction of a QSystemLocale
        // this is highly dependent on Qt internals, so may break, but oh well
        QSystemLocale *dummy = new QSystemLocale();
        delete dummy;
    }
}

struct LanguageRowData {
    LanguageRowData()
    {
        label = nullptr;
        languageButton = nullptr;
        removeButton = nullptr;
    }
    QLabel *label;
    KLanguageButton *languageButton;
    QPushButton *removeButton;

    void setRowWidgets(QLabel *label, KLanguageButton *languageButton, QPushButton *removeButton)
    {
        this->label = label;
        this->languageButton = languageButton;
        this->removeButton = removeButton;
    }
};

class KSwitchLanguageDialogPrivate
{
public:
    KSwitchLanguageDialogPrivate(KSwitchLanguageDialog *parent);

    KSwitchLanguageDialog *p; // parent class

    /**
        Fills language button with names of languages for which given application has translation.
    */
    void fillApplicationLanguages(KLanguageButton *button);

    /**
        Adds one button with language to widget.
    */
    void addLanguageButton(const QString &languageCode, bool primaryLanguage);

    /**
        Returns list of languages chosen for application or default languages is they are not set.
    */
    QStringList applicationLanguageList();

    QMap<QPushButton *, LanguageRowData> languageRows;
    QList<KLanguageButton *> languageButtons;
    QGridLayout *languagesLayout;
};

/*************************** KSwitchLanguageDialog **************************/

KSwitchLanguageDialog::KSwitchLanguageDialog(QWidget *parent)
    : QDialog(parent)
    , d(new KSwitchLanguageDialogPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Language"));

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel(i18n("Please choose the language which should be used for this application:"), this);
    topLayout->addWidget(label);

    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(languageHorizontalLayout);

    d->languagesLayout = new QGridLayout();
    languageHorizontalLayout->addLayout(d->languagesLayout);
    languageHorizontalLayout->addStretch();

    const QStringList defaultLanguages = d->applicationLanguageList();

    int count = defaultLanguages.count();
    for (int i = 0; i < count; ++i) {
        QString language = defaultLanguages[i];
        bool primaryLanguage = (i == 0);
        d->addLanguageButton(language, primaryLanguage);
    }

    if (!count) {
        QLocale l;
        d->addLanguageButton(l.name(), true);
    }

    QHBoxLayout *addButtonHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(addButtonHorizontalLayout);

    QPushButton *addLangButton = new QPushButton(i18nc("@action:button", "Add Fallback Language"), this);
    addLangButton->setToolTip(i18nc("@info:tooltip", "Adds one more language which will be used if other translations do not contain a proper translation."));
    connect(addLangButton, &QPushButton::clicked, this, &KSwitchLanguageDialog::slotAddLanguageButton);
    addButtonHorizontalLayout->addWidget(addLangButton);
    addButtonHorizontalLayout->addStretch();

    topLayout->addStretch(10);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());

    topLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KSwitchLanguageDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &KSwitchLanguageDialog::slotDefault);
}

KSwitchLanguageDialog::~KSwitchLanguageDialog()
{
    delete d;
}

void KSwitchLanguageDialog::slotAddLanguageButton()
{
    // adding new button with en_US as it should always be present
    d->addLanguageButton(QStringLiteral("en_US"), d->languageButtons.isEmpty());
}

void KSwitchLanguageDialog::removeButtonClicked()
{
    QObject const *signalSender = sender();
    if (!signalSender) {
        qCCritical(DEBUG_KXMLGUI) << "KSwitchLanguageDialog::removeButtonClicked() called directly, not using signal";
        return;
    }

    QPushButton *removeButton = const_cast<QPushButton *>(::qobject_cast<const QPushButton *>(signalSender));
    if (!removeButton) {
        qCCritical(DEBUG_KXMLGUI) << "KSwitchLanguageDialog::removeButtonClicked() called from something else than QPushButton";
        return;
    }

    QMap<QPushButton *, LanguageRowData>::iterator it = d->languageRows.find(removeButton);
    if (it == d->languageRows.end()) {
        qCCritical(DEBUG_KXMLGUI) << "KSwitchLanguageDialog::removeButtonClicked called from unknown QPushButton";
        return;
    }

    LanguageRowData languageRowData = it.value();

    d->languageButtons.removeAll(languageRowData.languageButton);

    languageRowData.label->deleteLater();
    languageRowData.languageButton->deleteLater();
    languageRowData.removeButton->deleteLater();
    d->languageRows.erase(it);
}

void KSwitchLanguageDialog::languageOnButtonChanged(const QString &languageCode)
{
    Q_UNUSED(languageCode);
#if 0
    for (int i = 0, count = d->languageButtons.count(); i < count; ++i) {
        KLanguageButton *languageButton = d->languageButtons[i];
        if (languageButton->current() == languageCode) {
            //update all buttons which have matching id
            //might update buttons which were not changed, but well...
            languageButton->setText(KLocale::global()->languageCodeToName(languageCode));
        }
    }
#endif
}

void KSwitchLanguageDialog::slotOk()
{
    QStringList languages;
    languages.reserve(d->languageButtons.size());
    for (auto *languageButton : std::as_const(d->languageButtons)) {
        languages << languageButton->current();
    }

    if (d->applicationLanguageList() != languages) {
        QString languageString = languages.join(QLatin1Char(':'));
        // list is different from defaults or saved languages list
        setApplicationSpecificLanguage(languageString.toLatin1());

        KMessageBox::information(
            this,
            i18n("The language for this application has been changed. The change will take effect the next time the application is started."), // text
            i18nc("@title:window", "Application Language Changed"), // caption
            QStringLiteral("ApplicationLanguageChangedWarning") // dontShowAgainName
        );
    }

    accept();
}

void KSwitchLanguageDialog::slotDefault()
{
    const QStringList defaultLanguages = d->applicationLanguageList();

    setApplicationSpecificLanguage(QByteArray());

    // read back the new default
    QString language = QString::fromLatin1(getApplicationSpecificLanguage("en_US"));

    if (defaultLanguages != (QStringList() << language)) {
        KMessageBox::information(
            this,
            i18n("The language for this application has been changed. The change will take effect the next time the application is started."), // text
            i18n("Application Language Changed"), // caption
            QStringLiteral("ApplicationLanguageChangedWarning") // dontShowAgainName
        );
    }

    accept();
}

/************************ KSwitchLanguageDialogPrivate ***********************/

KSwitchLanguageDialogPrivate::KSwitchLanguageDialogPrivate(KSwitchLanguageDialog *parent)
    : p(parent)
{
    // NOTE: do NOT use "p" in constructor, it is not fully constructed
}

static bool stripCountryCode(QString *languageCode)
{
    const int idx = languageCode->indexOf(QLatin1Char('_'));
    if (idx != -1) {
        *languageCode = languageCode->left(idx);
        return true;
    }
    return false;
}

void KSwitchLanguageDialogPrivate::fillApplicationLanguages(KLanguageButton *button)
{
    const QLocale cLocale(QLocale::C);
    QSet<QString> insertedLanguages;

    const QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const QLocale &l : allLocales) {
        if (l != cLocale) {
            QString languageCode = l.name();
            if (!insertedLanguages.contains(languageCode) && KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                button->insertLanguage(languageCode);
                insertedLanguages << languageCode;
            } else if (stripCountryCode(&languageCode)) {
                if (!insertedLanguages.contains(languageCode) && KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                    button->insertLanguage(languageCode);
                    insertedLanguages << languageCode;
                }
            }
        }
    }
}

QStringList KSwitchLanguageDialogPrivate::applicationLanguageList()
{
    QStringList languagesList;

    QByteArray languageCode = getApplicationSpecificLanguage();
    if (!languageCode.isEmpty()) {
        languagesList = QString::fromLatin1(languageCode).split(QLatin1Char(':'));
    }
    if (languagesList.isEmpty()) {
        QLocale l;
        languagesList = l.uiLanguages();

        // We get en-US here but we use en_US
        for (auto &language : languagesList) {
            language.replace(QLatin1Char('-'), QLatin1Char('_'));
        }
    }

    for (int i = 0; i < languagesList.count();) {
        QString languageCode = languagesList[i];
        if (!KLocalizedString::isApplicationTranslatedInto(languageCode)) {
            if (stripCountryCode(&languageCode)) {
                if (KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                    languagesList[i] = languageCode;
                    ++i;
                    continue;
                }
            }
            languagesList.removeAt(i);
        } else {
            ++i;
        }
    }

    return languagesList;
}

void KSwitchLanguageDialogPrivate::addLanguageButton(const QString &languageCode, bool primaryLanguage)
{
    QString labelText = primaryLanguage ? i18n("Primary language:") : i18n("Fallback language:");

    KLanguageButton *languageButton = new KLanguageButton(p);
    languageButton->showLanguageCodes(true);

    fillApplicationLanguages(languageButton);

    languageButton->setCurrentItem(languageCode);

    QObject::connect(languageButton, &KLanguageButton::activated, p, &KSwitchLanguageDialog::languageOnButtonChanged);

    LanguageRowData languageRowData;
    QPushButton *removeButton = nullptr;

    if (!primaryLanguage) {
        removeButton = new QPushButton(i18nc("@action:button", "Remove"), p);

        QObject::connect(removeButton, &QPushButton::clicked, p, &KSwitchLanguageDialog::removeButtonClicked);
    }

    languageButton->setToolTip(
        primaryLanguage ? i18nc("@info:tooltip", "This is the main application language which will be used first, before any other languages.")
                        : i18nc("@info:tooltip", "This is the language which will be used if any previous languages do not contain a proper translation."));

    int numRows = languagesLayout->rowCount();

    QLabel *languageLabel = new QLabel(labelText, p);
    languagesLayout->addWidget(languageLabel, numRows + 1, 1, Qt::AlignLeft);
    languagesLayout->addWidget(languageButton, numRows + 1, 2, Qt::AlignLeft);

    if (!primaryLanguage) {
        languagesLayout->addWidget(removeButton, numRows + 1, 3, Qt::AlignLeft);
        languageRowData.setRowWidgets(languageLabel, languageButton, removeButton);
        removeButton->show();
    }

    languageRows.insert(removeButton, languageRowData);

    languageButtons.append(languageButton);
    languageButton->show();
    languageLabel->show();
}

}
