/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_SAFESTRING
#define SCRIPTABLE_SAFESTRING

#include <QObject>

#include "safestring.h"

using namespace KTextTemplate;

class ScriptableSafeString : public QObject
{
    Q_OBJECT
public:
    explicit ScriptableSafeString(QObject *parent = {});

    void setContent(const SafeString &content);
    SafeString wrappedString() const;

public Q_SLOTS:
    bool isSafe() const;
    void setSafety(bool safeness);
    QString rawString();

private:
    SafeString m_safeString;
};

#endif
