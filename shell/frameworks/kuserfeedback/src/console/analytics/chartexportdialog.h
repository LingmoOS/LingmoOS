/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_CHARTEXPORTDIALOG_H
#define KUSERFEEDBACK_CONSOLE_CHARTEXPORTDIALOG_H

#include <QDialog>

#include <memory>

namespace KUserFeedback {
namespace Console {

namespace Ui
{
class ChartExportDialog;
}

class ChartExportDialog : public QDialog
{
    Q_OBJECT
public:
    enum Type { Image, SVG, PDF };

    explicit ChartExportDialog(QWidget *parent);
    ~ChartExportDialog() override;

    Type type() const;
    QString filename() const;
    QSize size() const;

private:
    void fileButtonClicked();
    void validate();

    std::unique_ptr<Ui::ChartExportDialog> ui;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_CHARTEXPORTDIALOG_H
