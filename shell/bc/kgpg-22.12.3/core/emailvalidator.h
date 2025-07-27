/*
    SPDX-FileCopyrightText: 2006 Jimmy Gilles <jimmygilles@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QValidator>

namespace KgpgCore
{

class EmailValidator : public QValidator
{
public:
    explicit EmailValidator(QObject *parent = nullptr) : QValidator(parent) { }
    QValidator::State validate(QString &input, int &pos) const override;
};

} // namespace KgpgCore

#endif // EMAILVALIDATOR_H
