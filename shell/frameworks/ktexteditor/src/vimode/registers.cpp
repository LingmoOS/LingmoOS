/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "registers.h"
#include "katepartdebug.h"

#include <KConfigGroup>

#include <QApplication>
#include <QClipboard>

using namespace KateVi;

void Registers::readConfig(const KConfigGroup &config)
{
    const QStringList names = config.readEntry("ViRegisterNames", QStringList());
    const QStringList contents = config.readEntry("ViRegisterContents", QStringList());
    const QList<int> flags = config.readEntry("ViRegisterFlags", QList<int>());

    if (names.size() != contents.size() || contents.size() != flags.size()) {
        return;
    }

    for (int i = 0; i < names.size(); i++) {
        if (!names.at(i).isEmpty()) {
            set(names.at(i).at(0), contents.at(i), (OperationMode)(flags.at(i)));
        }
    }
}

void Registers::writeConfig(KConfigGroup &config) const
{
    if (m_registers.empty()) {
        return;
    }

    QStringList names;
    QStringList contents;
    QList<int> flags;
    for (const auto &[name, reg] : m_registers) {
        if (reg.first.length() <= 1000) {
            names << name;
            contents << reg.first;
            flags << int(reg.second);
        } else {
            qCDebug(LOG_KTE) << "Did not save contents of register " << name << ": contents too long (" << reg.first.length() << " characters)";
        }
    }

    config.writeEntry("ViRegisterNames", names);
    config.writeEntry("ViRegisterContents", contents);
    config.writeEntry("ViRegisterFlags", flags);
}

void Registers::setInsertStopped(const QString &text)
{
    set(InsertStoppedRegister, text);
}

void Registers::set(const QChar &reg, const QString &text, OperationMode flag)
{
    if (reg == BlackHoleRegister) {
        return;
    }

    if (reg == PrependNumberedRegister || (reg >= FirstNumberedRegister && reg <= LastNumberedRegister)) { // "kill ring" registers
        setNumberedRegister(reg, text, flag);
    } else if (reg == SystemClipboardRegister) {
        QApplication::clipboard()->setText(text, QClipboard::Clipboard);
    } else if (reg == SystemSelectionRegister) {
        QApplication::clipboard()->setText(text, QClipboard::Selection);
    } else {
        const QChar lowercase_reg = reg.toLower();
        if (reg != lowercase_reg) {
            m_registers[lowercase_reg].first.append(text);
        } else {
            m_registers.insert_or_assign(lowercase_reg, Register(text, flag));
        }
    }

    if (reg == ZeroRegister || reg == PrependNumberedRegister || reg == SmallDeleteRegister) {
        m_default = reg;
    }
}

QString Registers::getContent(const QChar &reg) const
{
    return getRegister(reg).first;
}

OperationMode Registers::getFlag(const QChar &reg) const
{
    return getRegister(reg).second;
}

Registers::Register Registers::getRegister(const QChar &reg) const
{
    Register regPair;
    QChar _reg = (reg != UnnamedRegister ? reg : m_default);

    if (_reg >= FirstNumberedRegister && _reg <= LastNumberedRegister) {
        int index = _reg.digitValue() - 1;
        if (m_numbered.size() > index) {
            regPair = m_numbered.at(index);
        }
    } else if (_reg == PrependNumberedRegister) {
        if (!m_numbered.isEmpty()) {
            regPair = m_numbered.front();
        }
    } else if (_reg == SystemClipboardRegister) {
        QString regContent = QApplication::clipboard()->text(QClipboard::Clipboard);
        regPair = Register(regContent, CharWise);
    } else if (_reg == SystemSelectionRegister) {
        QString regContent = QApplication::clipboard()->text(QClipboard::Selection);
        regPair = Register(regContent, CharWise);
    } else {
        const QChar lowercase_reg = _reg.toLower();
        auto it = m_registers.find(lowercase_reg);
        if (it != m_registers.end()) {
            return it->second;
        }
    }

    return regPair;
}

void Registers::setNumberedRegister(const QChar &reg, const QString &text, OperationMode flag)
{
    int index = reg.digitValue() - 1;
    if (reg == PrependNumberedRegister || index > m_numbered.size()) {
        if (m_numbered.size() == 9) {
            m_numbered.removeLast();
        }

        // register 0 is used for the last yank command, so insert at position 1
        m_numbered.prepend(Register(text, flag));
    } else {
        m_numbered[index].first = text;
        m_numbered[index].second = flag;
    }
}
