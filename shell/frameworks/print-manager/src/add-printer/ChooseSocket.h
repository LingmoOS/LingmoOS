/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHOOSE_SOCKET_H
#define CHOOSE_SOCKET_H

#include "GenericPage.h"

namespace Ui
{
class ChooseSocket;
}
class ChooseSocket : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseSocket(QWidget *parent = nullptr);
    ~ChooseSocket() override;

    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;
    bool canProceed() const override;

private:
    Ui::ChooseSocket *const ui;
    bool m_isValid = false;
};

#endif
