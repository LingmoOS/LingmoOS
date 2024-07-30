/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pendingfile.h"
#include "baloodebug.h"

using namespace Baloo;

PendingFile::PendingFile(const QString& path)
    : m_path(path)
    , m_created(false)
    , m_closedOnWrite(false)
    , m_attributesChanged(false)
    , m_deleted(false)
    , m_modified(false)
{
}

QString PendingFile::path() const
{
    return m_path;
}

bool PendingFile::isNewFile() const
{
    return m_created;
}

bool PendingFile::shouldIndexContents() const
{
    if (m_created || m_closedOnWrite || m_modified) {
        return true;
    }
    return false;
}

bool PendingFile::shouldIndexXAttrOnly() const
{
    if (m_attributesChanged && !shouldIndexContents()) {
        return true;
    }
    return false;
}

bool PendingFile::shouldRemoveIndex() const
{
    return m_deleted;
}

void PendingFile::merge(const PendingFile& file)
{
    m_attributesChanged |= file.m_attributesChanged;
    m_closedOnWrite |= file.m_closedOnWrite;
    m_created |= file.m_created;
    m_modified |= file.m_modified;
}

void PendingFile::printFlags() const
{
    qCDebug(BALOO) << "AttributesChanged:" << m_attributesChanged;
    qCDebug(BALOO) << "ClosedOnWrite:" << m_closedOnWrite;
    qCDebug(BALOO) << "Created:" << m_created;
    qCDebug(BALOO) << "Deleted:" << m_deleted;
    qCDebug(BALOO) << "Modified:" << m_modified;
}
