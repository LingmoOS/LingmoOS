/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_REGISTERS_H
#define KATEVI_REGISTERS_H

#include "definitions.h"

#include <QChar>
#include <QList>
#include <QString>
#include <map>

class KConfigGroup;

namespace KateVi
{
const QChar BlackHoleRegister = QLatin1Char('_');
const QChar SmallDeleteRegister = QLatin1Char('-');
const QChar ZeroRegister = QLatin1Char('0');
const QChar PrependNumberedRegister = QLatin1Char('!');
const QChar FirstNumberedRegister = QLatin1Char('1');
const QChar LastNumberedRegister = QLatin1Char('9');
const QChar SystemSelectionRegister = QLatin1Char('*');
const QChar SystemClipboardRegister = QLatin1Char('+');
const QChar UnnamedRegister = QLatin1Char('"');
const QChar InsertStoppedRegister = QLatin1Char('^');

class Registers
{
public:
    void writeConfig(KConfigGroup &config) const;
    void readConfig(const KConfigGroup &config);

    void setInsertStopped(const QString &text);

    void set(const QChar &reg, const QString &text, OperationMode flag = CharWise);
    QString getContent(const QChar &reg) const;
    OperationMode getFlag(const QChar &reg) const;

private:
    typedef QPair<QString, OperationMode> Register;

private:
    void setNumberedRegister(const QChar &reg, const QString &text, OperationMode flag = CharWise);
    Register getRegister(const QChar &reg) const;

private:
    QList<Register> m_numbered;
    std::map<QChar, Register> m_registers;
    QChar m_default;
};

}

#endif // KATEVI_REGISTERS_H
