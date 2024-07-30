/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "GenericPage.h"

GenericPage::GenericPage(QWidget *parent)
    : QWidget(parent)
    , m_working(0)
{
}

void GenericPage::working()
{
    if (m_working++ == 0) {
        Q_EMIT startWorking();
    }
}

void GenericPage::notWorking()
{
    if (--m_working == 0) {
        Q_EMIT stopWorking();
    }
}

QVariantMap GenericPage::values() const
{
    return m_args;
}

void GenericPage::setValues(const QVariantMap &args)
{
    m_args = args;
}

#include "moc_GenericPage.cpp"
