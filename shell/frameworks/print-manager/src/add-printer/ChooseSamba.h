/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHOOSE_SAMBA_H
#define CHOOSE_SAMBA_H

#include "GenericPage.h"

namespace Ui
{
class ChooseSamba;
}
class ChooseSamba : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseSamba(QWidget *parent = nullptr);
    ~ChooseSamba() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;
    bool canProceed() const override;

public slots:
    void load();

private slots:
    void checkSelected();

private:
    Ui::ChooseSamba *const ui;
};

#endif
