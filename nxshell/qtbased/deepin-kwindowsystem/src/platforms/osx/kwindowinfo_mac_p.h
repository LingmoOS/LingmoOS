/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Marijn Kruisselbrink (m.kruisselbrink@student.tue.nl)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KWINDOWINFO_MAC_P_H
#define KWINDOWINFO_MAC_P_H

#include "kwindowinfo.h"
#include <Carbon/Carbon.h>
#include <QtCore/QString>
#include <QtCore/QList>

// bah, why do header files invade my namespace and define such normal words as check...
#ifdef check
#undef check
#endif

struct Q_DECL_HIDDEN KWindowInfo::Private {
    Private();
    ~Private();
    int ref;
    WId win;
    bool isLocal;
    AXUIElementRef axElement() const
    {
        return m_axWin;
    }
    void setAxElement(const AXUIElementRef &axWin);
    ProcessSerialNumber psn() const
    {
        return m_psn;
    }
    pid_t pid() const
    {
        return m_pid;
    }
    void setProcessSerialNumber(const ProcessSerialNumber &psn);
    QString name;
#ifdef Q_OS_MAC32
    FSSpec iconSpec;
#else
    FSRef iconSpec;
#endif
    bool loadedData;
    void updateData();
    AXUIElementRef m_axWin;
    QList<KWindowInfo::Private *> children;
    KWindowInfo::Private *parent;
private:
    Private(const Private &);
    void operator=(const Private &);
    ProcessSerialNumber m_psn;
    pid_t m_pid;
};

#endif

