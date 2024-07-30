/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptablesafestring.h"

ScriptableSafeString::ScriptableSafeString(QObject *parent)
    : QObject(parent)
{
}

void ScriptableSafeString::setContent(const KTextTemplate::SafeString &content)
{
    m_safeString = content;
}

SafeString ScriptableSafeString::wrappedString() const
{
    return m_safeString;
}

bool ScriptableSafeString::isSafe() const
{
    return m_safeString.isSafe();
}

void ScriptableSafeString::setSafety(bool safeness)
{
    m_safeString.setSafety(safeness ? KTextTemplate::SafeString::IsSafe : KTextTemplate::SafeString::IsNotSafe);
}

QString ScriptableSafeString::rawString()
{
    return m_safeString;
}

#include "moc_scriptablesafestring.cpp"
