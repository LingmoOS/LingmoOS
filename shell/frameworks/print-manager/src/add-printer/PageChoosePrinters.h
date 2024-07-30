/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGE_CHOOSE_PRINTERS_H
#define PAGE_CHOOSE_PRINTERS_H

#include "GenericPage.h"

namespace Ui
{
class PageChoosePrinters;
}

class PageChoosePrinters : public GenericPage
{
    Q_OBJECT
public:
    explicit PageChoosePrinters(const QVariantMap &args, QWidget *parent = nullptr);
    ~PageChoosePrinters() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool canProceed() const override;

private:
    Ui::PageChoosePrinters *const ui;
};

#endif
