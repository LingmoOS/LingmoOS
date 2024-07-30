/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Paul Marchouk <pmarchouk@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SIMPLEIPV6ADDRESSVALIDATOR_H
#define SIMPLEIPV6ADDRESSVALIDATOR_H

#include "plasmanm_editor_export.h"

#include <QValidator>

class PLASMANM_EDITOR_EXPORT SimpleIpV6AddressValidator : public QValidator
{
    Q_OBJECT
public:
    enum AddressStyle { Base, WithCidr, WithPort };

    explicit SimpleIpV6AddressValidator(AddressStyle style = AddressStyle::Base, QObject *parent = nullptr);
    ~SimpleIpV6AddressValidator() override;

    State validate(QString &, int &) const override;

    /** Check input value with a regular expression describing simple input mask.
     */
    QValidator::State checkWithInputMask(QString &, int &) const;
    /** Function split input string into tetrads and check them for valid values.
     *  In the tetrads are placed into QList. Input string may be changed.
     */
    QValidator::State checkTetradsRanges(QString &) const;

private:
    const AddressStyle m_addressStyle;
    QRegularExpressionValidator m_validator;
};

#endif // SIMPLEIPV6ADDRESSVALIDATOR_H
