/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef __parts_h__
#define __parts_h__

#include <kparts/readonlypart.h>

class QTextEdit;
namespace KParts
{
class GUIActivateEvent;
}

class Part1 : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    Part1(QObject *parent, QWidget *parentWidget);
    ~Part1() override;

public Q_SLOTS:
    void slotBlah();
    void slotFooBar();

protected:
    bool openFile() override;

protected:
    QTextEdit *m_edit;
};

class Part2 : public KParts::Part
{
    Q_OBJECT
public:
    Part2(QObject *parent, QWidget *parentWidget);
    ~Part2() override;

protected:
    // This is not mandatory - only if you care about setting the
    // part caption when the part is used in a multi-part environment
    // (i.e. in a part manager)
    // There is a default impl for ReadOnlyPart...
    void guiActivateEvent(KParts::GUIActivateEvent *) override;
};

#endif
