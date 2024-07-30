/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGE_ADD_PRINTER_H
#define PAGE_ADD_PRINTER_H

#include "GenericPage.h"

namespace Ui
{
class PageAddPrinter;
}

class PageAddPrinter : public GenericPage
{
    Q_OBJECT
public:
    explicit PageAddPrinter(QWidget *parent = nullptr);
    ~PageAddPrinter() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool canProceed() const override;

    bool finishClicked() override;

public slots:
    void load();

private slots:
    void checkSelected();

private:
    Ui::PageAddPrinter *const ui;
};

#endif
