/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "chartexportdialog.h"
#include "ui_chartexportdialog.h"

#include <QFileDialog>

using namespace KUserFeedback::Console;

ChartExportDialog::ChartExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChartExportDialog)
{
    ui->setupUi(this);

    connect(ui->fileEdit, &QLineEdit::textChanged, this, &ChartExportDialog::validate);
    connect(ui->fileButton, &QPushButton::clicked, this, &ChartExportDialog::fileButtonClicked);
    validate();
}

ChartExportDialog::~ChartExportDialog() = default;

ChartExportDialog::Type ChartExportDialog::type() const
{
    if (ui->imageButton->isChecked())
        return Image;
    if (ui->svgButton->isChecked())
        return SVG;
    if (ui->pdfButton->isChecked())
        return PDF;
    Q_UNREACHABLE();
}

QString ChartExportDialog::filename() const
{
    return ui->fileEdit->text();
}

QSize ChartExportDialog::size() const
{
    return QSize(ui->width->value(), ui->height->value());
}

void ChartExportDialog::fileButtonClicked()
{
    const auto fn = QFileDialog::getSaveFileName(this, tr("Export Chart"));
    if (!fn.isEmpty())
        ui->fileEdit->setText(fn);
}

void ChartExportDialog::validate()
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!ui->fileEdit->text().isEmpty());
}

#include "moc_chartexportdialog.cpp"
