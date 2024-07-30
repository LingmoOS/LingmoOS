/*
    SPDX-FileCopyrightText: 2009 Paul Marchouk <pmarchouk@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SIMPLEIPV4ADDRESSVALIDATOR_H
#define SIMPLEIPV4ADDRESSVALIDATOR_H

#include "plasmanm_editor_export.h"

#include <QValidator>

class PLASMANM_EDITOR_EXPORT SimpleIpV4AddressValidator : public QValidator
{
    Q_OBJECT
public:
    enum AddressStyle { Base, WithCidr, WithPort };

    explicit SimpleIpV4AddressValidator(AddressStyle style = AddressStyle::Base, QObject *parent = nullptr);
    ~SimpleIpV4AddressValidator() override;

    State validate(QString &, int &) const override;

    /** Check input value with a regular expression describing simple input mask.
     */
    QValidator::State checkWithInputMask(QString &, int &) const;
    /** Function split input string into tetrads and check them for valid values.
     *  In the tetrads are placed into QList. Input string may be changed.
     */
    QValidator::State checkTetradsRanges(QString &, QList<int> &) const;

private:
    AddressStyle m_addressStyle;
    QRegularExpressionValidator m_validator;
};

#endif // SIMPLEIPV4ADDRESSVALIDATOR_H
