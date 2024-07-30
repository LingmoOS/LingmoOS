/*
    SPDX-FileCopyrightText: 2009 Andrey Batyiev <batyiev@gmail.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_LIST_VALIDATOR_H
#define PLASMA_NM_LIST_VALIDATOR_H

#include <QValidator>

/**
 * This class validates each string item with a validator.
 * String items are separated by comma.
 * The validator should be set with setInnerValidator(..) method.
 * Please note, space characters are allowed only after comma characters.
 */
class ListValidator : public QValidator
{
    Q_OBJECT
public:
    explicit ListValidator(QObject *parent);
    ~ListValidator() override;

    State validate(QString &text, int &pos) const override;
    void setInnerValidator(QValidator *validator);

private:
    QValidator *inner = nullptr;
};

#endif // PLASMA_NM_LIST_VALIDATOR_H
