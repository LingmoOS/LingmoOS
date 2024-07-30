/*
    SPDX-FileCopyrightText: 2016 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNEWPASSWORDWIDGET_TEST_H
#define KNEWPASSWORDWIDGET_TEST_H

#include <QDialog>

class QDialogButtonBox;

class KNewPasswordWidget;

class MyPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    MyPasswordDialog(QWidget *parent = nullptr);

    void accept() override;

private Q_SLOTS:
    void slotPasswordStatusChanged();

private:
    KNewPasswordWidget *m_passwordWidget;
    QDialogButtonBox *m_buttonBox;
};

#endif
