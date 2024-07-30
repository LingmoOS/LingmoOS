/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katemodeconfigpage.h"

#include "kateautoindent.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katesyntaxmanager.h"
#include "kateview.h"

#include "ui_filetypeconfigwidget.h"

#include "katepartdebug.h"
#include <KMimeTypeChooser>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QToolButton>
// END Includes

ModeConfigPage::ModeConfigPage(QWidget *parent)
    : KateConfigPage(parent)
{
    m_lastType = -1;

    // This will let us have more separation between this page and
    // the QTabWidget edge (ereslibre)
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *newWidget = new QWidget(this);

    ui = new Ui::FileTypeConfigWidget();
    ui->setupUi(newWidget);

    ui->cmbHl->addItem(i18n("<Unchanged>"), QVariant(QString()));
    const auto modeList = KateHlManager::self()->modeList();
    for (const auto &hl : modeList) {
        const auto section = hl.translatedSection();
        if (!section.isEmpty()) {
            ui->cmbHl->addItem(section + QLatin1Char('/') + hl.translatedName(), QVariant(hl.name()));
        } else {
            ui->cmbHl->addItem(hl.translatedName(), QVariant(hl.name()));
        }
    }

    QStringList indentationModes;
    indentationModes << i18n("Use Default");
    indentationModes << KateAutoIndent::listModes();
    ui->cmbIndenter->addItems(indentationModes);

    connect(ui->cmbFiletypes, &QComboBox::activated, this, &ModeConfigPage::typeChanged);
    connect(ui->btnNew, &QPushButton::clicked, this, &ModeConfigPage::newType);
    connect(ui->btnDelete, &QPushButton::clicked, this, &ModeConfigPage::deleteType);
    ui->btnMimeTypes->setIcon(QIcon::fromTheme(QStringLiteral("tools-wizard")));
    connect(ui->btnMimeTypes, &QToolButton::clicked, this, &ModeConfigPage::showMTDlg);

    reload();

    connect(ui->edtName, &QLineEdit::textChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->edtSection, &QLineEdit::textChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->edtVariables, &VariableLineEdit::textChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->edtFileExtensions, &QLineEdit::textChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->edtMimeTypes, &QLineEdit::textChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->sbPriority, &QSpinBox::valueChanged, this, &ModeConfigPage::slotChanged);
    connect(ui->cmbHl, &QComboBox::activated, this, &ModeConfigPage::slotChanged);
    connect(ui->cmbIndenter, &QComboBox::activated, this, &ModeConfigPage::slotChanged);

    // make the context help a bit easier to access
    ui->sbPriority->setToolTip(ui->sbPriority->whatsThis());

    layout->addWidget(newWidget);
}

ModeConfigPage::~ModeConfigPage()
{
    qDeleteAll(m_types);
    delete ui;
}

void ModeConfigPage::apply()
{
    if (!hasChanged()) {
        return;
    }

    save();
    if (m_lastType != -1) {
        ui->gbProperties->setTitle(i18n("Properties of %1", ui->cmbFiletypes->itemText(m_lastType)));
    }

    KTextEditor::EditorPrivate::self()->modeManager()->save(m_types);
}

void ModeConfigPage::reload()
{
    qDeleteAll(m_types);
    m_types.clear();

    // deep copy...
    const QList<KateFileType *> &modeList = KTextEditor::EditorPrivate::self()->modeManager()->list();
    m_types.reserve(modeList.size());
    for (KateFileType *type : modeList) {
        KateFileType *t = new KateFileType();
        *t = *type;
        m_types.append(t);
    }

    update();
}

void ModeConfigPage::reset()
{
    reload();
}

void ModeConfigPage::defaults()
{
    reload();
}

void ModeConfigPage::update()
{
    m_lastType = -1;

    ui->cmbFiletypes->clear();

    for (KateFileType *type : std::as_const(m_types)) {
        if (!type->sectionTranslated().isEmpty()) {
            ui->cmbFiletypes->addItem(type->sectionTranslated() + QLatin1Char('/') + type->nameTranslated());
        } else {
            ui->cmbFiletypes->addItem(type->nameTranslated());
        }
    }

    // get current filetype from active view via the host application
    int currentIndex = 0;
    KTextEditor::ViewPrivate *kv =
        qobject_cast<KTextEditor::ViewPrivate *>(KTextEditor::EditorPrivate::self()->application()->activeMainWindow()->activeView());
    if (kv) {
        const QString filetypeName = kv->doc()->fileType();
        for (int i = 0; i < m_types.size(); ++i) {
            if (filetypeName == m_types[i]->name) {
                currentIndex = i;
                break;
            }
        }
    }
    ui->cmbFiletypes->setCurrentIndex(currentIndex);
    typeChanged(currentIndex);

    ui->cmbFiletypes->setEnabled(ui->cmbFiletypes->count() > 0);
}

void ModeConfigPage::deleteType()
{
    int type = ui->cmbFiletypes->currentIndex();

    if (type > -1 && type < m_types.count()) {
        delete m_types[type];
        m_types.removeAt(type);
        update();
    }
}

void ModeConfigPage::newType()
{
    QString newN = i18n("New Filetype");

    for (int i = 0; i < m_types.count(); ++i) {
        KateFileType *type = m_types.at(i);
        if (type->name == newN) {
            ui->cmbFiletypes->setCurrentIndex(i);
            typeChanged(i);
            return;
        }
    }

    KateFileType *newT = new KateFileType();
    newT->priority = 0;
    newT->name = newN;
    newT->hlGenerated = false;

    m_types.prepend(newT);

    update();
    // show new filetype so that it is immediately available for editing
    ui->cmbFiletypes->setCurrentIndex(0);
    typeChanged(0);
}

void ModeConfigPage::save()
{
    if (m_lastType != -1) {
        if (!m_types[m_lastType]->hlGenerated) {
            m_types[m_lastType]->name = ui->edtName->text();
            m_types[m_lastType]->section = ui->edtSection->text();

            if (!m_types[m_lastType]->sectionTranslated().isEmpty()) {
                ui->cmbFiletypes->setItemText(m_lastType, m_types[m_lastType]->sectionTranslated() + QLatin1Char('/') + m_types[m_lastType]->nameTranslated());
            } else {
                ui->cmbFiletypes->setItemText(m_lastType, m_types[m_lastType]->nameTranslated());
            }
        }
        m_types[m_lastType]->varLine = ui->edtVariables->text();
        m_types[m_lastType]->wildcards = ui->edtFileExtensions->text().split(QLatin1Char(';'), Qt::SkipEmptyParts);
        m_types[m_lastType]->mimetypes = ui->edtMimeTypes->text().split(QLatin1Char(';'), Qt::SkipEmptyParts);
        m_types[m_lastType]->priority = ui->sbPriority->value();
        m_types[m_lastType]->hl = ui->cmbHl->itemData(ui->cmbHl->currentIndex()).toString();

        if (ui->cmbIndenter->currentIndex() > 0) {
            m_types[m_lastType]->indenter = KateAutoIndent::modeName(ui->cmbIndenter->currentIndex() - 1);
        } else {
            m_types[m_lastType]->indenter = QString();
        }
    }
}

void ModeConfigPage::typeChanged(int type)
{
    save();

    ui->cmbHl->setEnabled(true);
    ui->btnDelete->setEnabled(true);
    ui->edtName->setEnabled(true);
    ui->edtSection->setEnabled(true);

    if (type > -1 && type < m_types.count()) {
        KateFileType *t = m_types.at(type);

        ui->gbProperties->setTitle(i18n("Properties of %1", ui->cmbFiletypes->itemText(type)));

        ui->gbProperties->setEnabled(true);
        ui->btnDelete->setEnabled(true);

        ui->edtName->setText(t->nameTranslated());
        ui->edtSection->setText(t->sectionTranslated());
        ui->edtVariables->setText(t->varLine);
        ui->edtFileExtensions->setText(t->wildcards.join(QLatin1Char(';')));
        ui->edtMimeTypes->setText(t->mimetypes.join(QLatin1Char(';')));
        ui->sbPriority->setValue(t->priority);

        ui->cmbHl->setEnabled(!t->hlGenerated);
        ui->btnDelete->setEnabled(!t->hlGenerated);
        ui->edtName->setEnabled(!t->hlGenerated);
        ui->edtSection->setEnabled(!t->hlGenerated);

        // activate current hl...
        for (int i = 0; i < ui->cmbHl->count(); ++i) {
            if (ui->cmbHl->itemData(i).toString() == t->hl) {
                ui->cmbHl->setCurrentIndex(i);
            }
        }

        // activate the right indenter
        int indenterIndex = 0;
        if (!t->indenter.isEmpty()) {
            indenterIndex = KateAutoIndent::modeNumber(t->indenter) + 1;
        }
        ui->cmbIndenter->setCurrentIndex(indenterIndex);
    } else {
        ui->gbProperties->setTitle(i18n("Properties"));

        ui->gbProperties->setEnabled(false);
        ui->btnDelete->setEnabled(false);

        ui->edtName->clear();
        ui->edtSection->clear();
        ui->edtVariables->clear();
        ui->edtFileExtensions->clear();
        ui->edtMimeTypes->clear();
        ui->sbPriority->setValue(0);
        ui->cmbHl->setCurrentIndex(0);
        ui->cmbIndenter->setCurrentIndex(0);
    }

    m_lastType = type;
}

void ModeConfigPage::showMTDlg()
{
    QString text =
        i18n("Select the MimeTypes you want for this file type.\nPlease note that this will automatically edit the associated file extensions as well.");
    QStringList list = ui->edtMimeTypes->text().split(QRegularExpression(QStringLiteral("\\s*;\\s*")), Qt::SkipEmptyParts);
    KMimeTypeChooserDialog d(i18n("Select Mime Types"), text, list, QStringLiteral("text"), this);
    if (d.exec() == QDialog::Accepted) {
        // do some checking, warn user if mime types or patterns are removed.
        // if the lists are empty, and the fields not, warn.
        ui->edtFileExtensions->setText(d.chooser()->patterns().join(QLatin1Char(';')));
        ui->edtMimeTypes->setText(d.chooser()->mimeTypes().join(QLatin1Char(';')));
    }
}

QString ModeConfigPage::name() const
{
    return i18n("Modes && Filetypes");
}
