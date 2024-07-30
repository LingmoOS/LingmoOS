/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFileDialog>
#include <QTableWidgetItem>
#include <QWhatsThis>

#include <KLocalizedString>
#include <KMessageBox>

#include "ui_configwidget.h"
#include <utils/kateconfig.h>
#include <utils/kateglobal.h>
#include <vimode/config/configtab.h>
#include <vimode/keyparser.h>

using namespace KateVi;

ConfigTab::ConfigTab(QWidget *parent, Mappings *mappings)
    : KateConfigPage(parent)
    , m_mappings(mappings)
{
    // This will let us have more separation between this page and
    // the QTabWidget edge (ereslibre)
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *newWidget = new QWidget(this);

    ui = new Ui::ConfigWidget();
    ui->setupUi(newWidget);

    // Make the header take all the width in equal parts.
    ui->tblNormalModeMappings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblInsertModeMappings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblVisualModeMappings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // What's This? help can be found in the ui file
    reload();

    //
    // after initial reload, connect the stuff for the changed() signal
    //
    connect(ui->chkViCommandsOverride, &QCheckBox::toggled, this, &ConfigTab::slotChanged);
    connect(ui->chkViRelLineNumbers, &QCheckBox::toggled, this, &ConfigTab::slotChanged);
    connect(ui->tblNormalModeMappings, &QTableWidget::cellChanged, this, &ConfigTab::slotChanged);
    connect(ui->btnAddNewRow, &QPushButton::clicked, this, &ConfigTab::addMappingRow);
    connect(ui->btnAddNewRow, &QPushButton::clicked, this, &ConfigTab::slotChanged);
    connect(ui->btnRemoveSelectedRows, &QPushButton::clicked, this, &ConfigTab::removeSelectedMappingRows);
    connect(ui->btnRemoveSelectedRows, &QPushButton::clicked, this, &ConfigTab::slotChanged);
    connect(ui->btnImportNormal, &QPushButton::clicked, this, &ConfigTab::importNormalMappingRow);
    connect(ui->btnImportNormal, &QPushButton::clicked, this, &ConfigTab::slotChanged);

    layout->addWidget(newWidget);
}

ConfigTab::~ConfigTab()
{
    delete ui;
}

void ConfigTab::applyTab(QTableWidget *mappingsTable, Mappings::MappingMode mode)
{
    m_mappings->clear(mode);

    for (int i = 0; i < mappingsTable->rowCount(); i++) {
        QTableWidgetItem *from = mappingsTable->item(i, 0);
        QTableWidgetItem *to = mappingsTable->item(i, 1);
        QTableWidgetItem *recursive = mappingsTable->item(i, 2);

        if (from && to && recursive) {
            const Mappings::MappingRecursion recursion = recursive->checkState() == Qt::Checked ? Mappings::Recursive : Mappings::NonRecursive;
            m_mappings->add(mode, from->text(), to->text(), recursion);
        }
    }
}

void ConfigTab::reloadTab(QTableWidget *mappingsTable, Mappings::MappingMode mode)
{
    const QStringList l = m_mappings->getAll(mode);
    mappingsTable->setRowCount(l.size());

    int i = 0;
    for (const QString &f : l) {
        QTableWidgetItem *from = new QTableWidgetItem(KeyParser::self()->decodeKeySequence(f));
        QString s = m_mappings->get(mode, f);
        QTableWidgetItem *to = new QTableWidgetItem(KeyParser::self()->decodeKeySequence(s));
        QTableWidgetItem *recursive = new QTableWidgetItem();
        recursive->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        const bool isRecursive = m_mappings->isRecursive(mode, f);
        recursive->setCheckState(isRecursive ? Qt::Checked : Qt::Unchecked);

        mappingsTable->setItem(i, 0, from);
        mappingsTable->setItem(i, 1, to);
        mappingsTable->setItem(i, 2, recursive);

        i++;
    }
}

void ConfigTab::apply()
{
    // nothing changed, no need to apply stuff
    if (!hasChanged()) {
        return;
    }
    m_changed = false;

    KateViewConfig::global()->configStart();

    // General options.
    KateViewConfig::global()->setValue(KateViewConfig::ViRelativeLineNumbers, ui->chkViRelLineNumbers->isChecked());
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, ui->chkViCommandsOverride->isChecked());

    // Mappings.
    applyTab(ui->tblNormalModeMappings, Mappings::NormalModeMapping);
    applyTab(ui->tblInsertModeMappings, Mappings::InsertModeMapping);
    applyTab(ui->tblVisualModeMappings, Mappings::VisualModeMapping);

    KateViewConfig::global()->configEnd();
}

void ConfigTab::reload()
{
    // General options.
    ui->chkViRelLineNumbers->setChecked(KateViewConfig::global()->viRelativeLineNumbers());
    ui->chkViCommandsOverride->setChecked(KateViewConfig::global()->viInputModeStealKeys());

    // Mappings.
    reloadTab(ui->tblNormalModeMappings, Mappings::NormalModeMapping);
    reloadTab(ui->tblInsertModeMappings, Mappings::InsertModeMapping);
    reloadTab(ui->tblVisualModeMappings, Mappings::VisualModeMapping);
}

void ConfigTab::reset()
{
    /* Do nothing. */
}

void ConfigTab::defaults()
{
    /* Do nothing. */
}

void ConfigTab::showWhatsThis(const QString &text)
{
    QWhatsThis::showText(QCursor::pos(), text);
}

void ConfigTab::addMappingRow()
{
    // Pick the current widget.
    QTableWidget *mappingsTable = ui->tblNormalModeMappings;
    if (ui->tabMappingModes->currentIndex() == 1) {
        mappingsTable = ui->tblInsertModeMappings;
    } else if (ui->tabMappingModes->currentIndex() == 2) {
        mappingsTable = ui->tblVisualModeMappings;
    }

    // And add a new row.
    int rows = mappingsTable->rowCount();
    mappingsTable->insertRow(rows);
    QTableWidgetItem *recursive = new QTableWidgetItem();
    recursive->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    recursive->setCheckState(Qt::Unchecked);
    mappingsTable->setItem(rows, 2, recursive);
    mappingsTable->setCurrentCell(rows, 0);
    mappingsTable->editItem(mappingsTable->currentItem());
}

void ConfigTab::removeSelectedMappingRows()
{
    // Pick the current widget.
    QTableWidget *mappingsTable = ui->tblNormalModeMappings;
    if (ui->tabMappingModes->currentIndex() == 1) {
        mappingsTable = ui->tblInsertModeMappings;
    } else if (ui->tabMappingModes->currentIndex() == 2) {
        mappingsTable = ui->tblVisualModeMappings;
    }

    // And remove the selected rows.
    const QList<QTableWidgetSelectionRange> l = mappingsTable->selectedRanges();
    for (const QTableWidgetSelectionRange &range : l) {
        for (int i = 0; i < range.bottomRow() - range.topRow() + 1; i++) {
            mappingsTable->removeRow(range.topRow());
        }
    }
}

void ConfigTab::importNormalMappingRow()
{
    const QString &fileName = QFileDialog::getOpenFileName(this);

    if (fileName.isEmpty()) {
        return;
    }

    QFile configFile(fileName);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KMessageBox::error(this, i18n("Unable to open the config file for reading."), i18n("Unable to open file"));
        return;
    }

    QTextStream stream(&configFile);
    const QRegularExpression mapleader(QStringLiteral("(?:\\w:)?mapleader"));
    while (!stream.atEnd()) {
        const QStringList line = stream.readLine().split(QLatin1Char(' '));

        // TODO - allow recursive mappings to be read.
        if (line.size() > 2
            && (line[0] == QLatin1String("noremap") || line[0] == QLatin1String("no") || line[0] == QLatin1String("nnoremap")
                || line[0] == QLatin1String("nn"))) {
            int rows = ui->tblNormalModeMappings->rowCount();
            ui->tblNormalModeMappings->insertRow(rows);
            ui->tblNormalModeMappings->setItem(rows, 0, new QTableWidgetItem(line[1]));
            ui->tblNormalModeMappings->setItem(rows, 1, new QTableWidgetItem(line[2]));
            QTableWidgetItem *recursive = new QTableWidgetItem();
            recursive->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
            recursive->setCheckState(Qt::Unchecked);
            ui->tblNormalModeMappings->setItem(rows, 2, recursive);
        } else if (line.size() == 4 && line[0] == QLatin1String("let") && line[2] == QLatin1String("=") && mapleader.match(line[1]).hasMatch()) {
            const QStringView leader = QStringView(line[3]).mid(1, line[3].length() - 2);
            if (!leader.isEmpty()) {
                m_mappings->setLeader(leader[0]);
            }
        }
    }
}

QString ConfigTab::name() const
{
    return i18n("Vi Input Mode");
}
