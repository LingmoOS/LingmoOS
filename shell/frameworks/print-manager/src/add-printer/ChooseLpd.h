/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHOOSE_LPD_H
#define CHOOSE_LPD_H

#include "GenericPage.h"

namespace Ui
{
class ChooseLpd;
}
class ChooseLpd : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseLpd(QWidget *parent = nullptr);
    ~ChooseLpd() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;
    bool canProceed() const override;

private slots:
    void checkSelected();

private:
    Ui::ChooseLpd *const ui;
    bool m_isValid;
};

#endif
