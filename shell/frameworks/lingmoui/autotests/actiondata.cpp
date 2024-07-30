// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "actiondata.h"

using namespace Qt::StringLiterals;

ActionData::ActionData(QObject *parent)
    : QObject(parent)
    , m_enabledAction(new QAction(this))
    , m_disabledAction(new QAction(this))
{
    m_enabledAction->setText(u"Enabled Action"_s);

    m_disabledAction->setText(u"Disabled Action"_s);
    m_disabledAction->setEnabled(false);
}

QAction *ActionData::enabledAction() const
{
    return m_enabledAction;
}

QAction *ActionData::disabledAction() const
{
    return m_disabledAction;
}
