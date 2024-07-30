/*
    SPDX-FileCopyrightText: 2024 Stephen Robinson <stephen.robinson@eqware.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef INTERFACENAMEVALIDATOR_H
#define INTERFACENAMEVALIDATOR_H

#include <QValidator>

class InterfaceNameValidator : public QValidator
{
    Q_OBJECT
public:
    explicit InterfaceNameValidator(QObject *parent = nullptr);
    ~InterfaceNameValidator() override;

    QValidator::State validate(QString &, int &) const override;
};

#endif // INTERFACENAMEVALIDATOR_H
