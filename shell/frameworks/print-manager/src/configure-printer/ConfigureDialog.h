/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include <KPageDialog>

class PrinterPage;
class ModifyPrinter;
class PrinterOptions;
class QAbstractButton;
class QCloseEvent;

class Q_DECL_EXPORT ConfigureDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~ConfigureDialog() override;

private:
    void currentPageChangedSlot(KPageWidgetItem *current, KPageWidgetItem *before);
    void enableButtonApply(bool enable);
    void slotButtonClicked(QAbstractButton *pressedButton);
    void ppdChanged();

    ModifyPrinter *modifyPrinter = nullptr;
    PrinterOptions *printerOptions = nullptr;
    void closeEvent(QCloseEvent *event) override;
    // return false if the dialog was canceled
    bool savePage(PrinterPage *page);
};

#endif
