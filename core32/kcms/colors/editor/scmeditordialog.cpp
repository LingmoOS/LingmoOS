/* ColorEdit widget for KDE Display color scheme setup module
    SPDX-FileCopyrightText: 2016 Olivier Churlaud <olivier@churlaud.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scmeditordialog.h"
#include "scmeditorcolors.h"
#include "scmeditoreffects.h"
#include "scmeditoroptions.h"

#include <QDebug>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QUrl>

#include <KConfigGroup>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KWindowSystem>

SchemeEditorDialog::SchemeEditorDialog(KSharedConfigPtr config, QWidget *parent)
    : QDialog(parent)
    , m_config(config)
{
    init();
}

SchemeEditorDialog::SchemeEditorDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
    , m_filePath(path)
    , m_config(KSharedConfig::openConfig(path, KSharedConfig::SimpleConfig))
{
    m_schemeName = KConfigGroup(m_config, "General").readEntry("Name");
    this->setWindowTitle(m_schemeName);
    init();
}

bool SchemeEditorDialog::showApplyOverwriteButton() const
{
    return m_showApplyOverwriteButton;
}

void SchemeEditorDialog::setShowApplyOverwriteButton(bool show)
{
    m_showApplyOverwriteButton = show;

    buttonBox->button(QDialogButtonBox::Apply)->setVisible(show);
}

void SchemeEditorDialog::init()
{
    setupUi(this);

    m_optionTab = new SchemeEditorOptions(m_config);
    m_colorTab = new SchemeEditorColors(m_config);
    m_disabledTab = new SchemeEditorEffects(m_config, QPalette::Disabled);
    m_inactiveTab = new SchemeEditorEffects(m_config, QPalette::Inactive);
    tabWidget->addTab(m_optionTab, i18n("Options"));
    tabWidget->addTab(m_colorTab, i18n("Colors"));
    tabWidget->addTab(m_disabledTab, i18n("Disabled"));
    tabWidget->setCurrentWidget(m_colorTab);

    connect(m_optionTab, &SchemeEditorOptions::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_colorTab, &SchemeEditorColors::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_disabledTab, &SchemeEditorEffects::changed, this, &SchemeEditorDialog::updateTabs);
    connect(m_inactiveTab, &SchemeEditorEffects::changed, this, &SchemeEditorDialog::updateTabs);

    // "Save" is only shown in overwrite mode
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::save());
    buttonBox->button(QDialogButtonBox::Apply)->setVisible(false);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Save), KStandardGuiItem::saveAs());
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    updateTabs();
}

void SchemeEditorDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (buttonBox->standardButton(button) == QDialogButtonBox::Reset) {
        m_config->markAsClean();
        m_config->reparseConfiguration();
        updateTabs();
        setUnsavedChanges(false);
    } else if (buttonBox->standardButton(button) == QDialogButtonBox::Save) {
        saveScheme(false /*overwrite*/);
    } else if (buttonBox->standardButton(button) == QDialogButtonBox::Apply) {
        saveScheme(true /*overwrite*/);
    } else if (buttonBox->standardButton(button) == QDialogButtonBox::Close) {
        if (m_unsavedChanges) {
            KMessageBox::ButtonCode ans = KMessageBox::questionTwoActions(this,
                                                                          i18n("You have unsaved changes. Do you really want to quit?"),
                                                                          i18n("Unsaved changes"),
                                                                          KStandardGuiItem::quit(),
                                                                          KStandardGuiItem::cancel());
            if (ans == KMessageBox::SecondaryAction) {
                return;
            }
        }
        m_config->markAsClean();
        m_config->reparseConfiguration();
        this->accept();
    }
}

void SchemeEditorDialog::saveScheme(bool overwrite)
{
    QString name = m_schemeName;

    // prompt for the name to save as
    if (!overwrite) {
        bool ok;
        name = QInputDialog::getText(this, i18n("Save Color Scheme"), i18n("&Enter a name for the color scheme:"), QLineEdit::Normal, m_schemeName, &ok);
        if (!ok) {
            return;
        }
    }

    QString filename = name;
    filename.remove(QLatin1Char('\'')); // So Foo's does not become FooS
    QRegularExpression fixer(QStringLiteral("[\\W,.-]+(.?)"));
    for (auto match = fixer.match(filename); match.hasMatch(); match = fixer.match(filename)) {
        filename.replace(match.capturedStart(), match.capturedLength(), match.captured(1).toUpper());
    }
    filename.replace(0, 1, filename.at(0).toUpper());

    // check if that name is already in the list
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/") + filename + QStringLiteral(".colors"));

    QFile file(path);
    const int permissions = file.permissions();
    const bool canWrite = (permissions & QFile::WriteUser);
    // or if we can overwrite it if it exists
    if (path.isEmpty() || !file.exists() || canWrite) {
        if (canWrite && !overwrite) {
            int ret = KMessageBox::questionTwoActions(this,
                                                      i18n("A color scheme with that name already exists.\nDo you want to overwrite it?"),
                                                      i18n("Save Color Scheme"),
                                                      KStandardGuiItem::overwrite(),
                                                      KStandardGuiItem::cancel());

            // on don't overwrite, call again the function
            if (ret == KMessageBox::SecondaryAction) {
                this->saveScheme(overwrite);
                return;
            }
        }

        // go ahead and save it
        QString newpath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/color-schemes/";
        QDir dir;
        dir.mkpath(newpath);
        newpath += filename + ".colors";

        KConfig *config = m_config->copyTo(newpath);
        m_config->markAsClean();
        m_config->reparseConfiguration();
        KConfigGroup group(config, "General");
        group.writeEntry("Name", name);

        // sync it and delete pointer
        config->sync();
        delete config;
        // reopen and update window
        m_config = KSharedConfig::openConfig(newpath);
        m_schemeName = name;
        setWindowTitle(name);

        setUnsavedChanges(false);

        QTextStream out(stdout);
        out << filename << Qt::endl;
    } else if (!canWrite && file.exists()) {
        KMessageBox::error(this, i18n("You do not have permission to overwrite that scheme"), i18n("Error"));
    }
}

void SchemeEditorDialog::updateTabs(bool madeByUser)
{
    if (madeByUser) {
        setUnsavedChanges(true);
    }
    KConfigGroup group(m_config, "ColorEffects:Inactive");
    bool showInactiveTab = group.readEntry("Enable", QVariant(true)).toBool();

    const int idx = tabWidget->indexOf(m_inactiveTab);

    if (showInactiveTab && idx == -1) {
        tabWidget->addTab(m_inactiveTab, i18n("Inactive"));
    } else if (!showInactiveTab && idx > -1) {
        tabWidget->removeTab(idx);
    }

    m_optionTab->updateValues();
    m_colorTab->updateValues();
    m_inactiveTab->updateValues();
    m_disabledTab->updateValues();
}

void SchemeEditorDialog::setUnsavedChanges(bool changes)
{
    m_unsavedChanges = changes;
    if (changes) {
        buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
    } else {
        buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    }
}
