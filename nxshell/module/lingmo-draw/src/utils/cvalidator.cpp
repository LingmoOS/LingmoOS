// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cvalidator.h"

CIntValidator::CIntValidator(int bottom, int top, QObject *parent)
    : QIntValidator(bottom, top, parent)
{

}

void CIntValidator::setEmptyStrToBottom(bool b)
{
    m_emptyStrIsBottom = b;
}

//bool CIntValidator::emptyStrIsBottom() const
//{
//    return m_emptyStrIsBottom;
//}

QValidator::State CIntValidator::validate(QString &s, int &n) const
{
    if (s.isEmpty() && !m_emptyStrIsBottom)
        return QValidator::Acceptable;

    return QIntValidator::validate(s, n);
}

void CIntValidator::fixup(QString &str) const
{
    if (str.isEmpty() && !m_emptyStrIsBottom) {
        return;
    }
    str = QString("%1").arg(bottom());
    QIntValidator::fixup(str);
}
