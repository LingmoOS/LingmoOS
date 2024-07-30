/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __kparts_partactivateevent_h__
#define __kparts_partactivateevent_h__

#include <QEvent>
#include <kparts/kparts_export.h>
#include <memory>

class QWidget;

namespace KParts
{
class Part;

class PartActivateEventPrivate;
/**
 * @class PartActivateEvent partactivateevent.h <KParts/PartActivateEvent>
 *
 * @short This event is sent by the part manager when the active part changes.
 * Each time the active part changes, it will send first a PartActivateEvent
 * with activated=false, part=oldActivePart, widget=oldActiveWidget
 * and then another PartActivateEvent
 * with activated=true, part=newPart, widget=newWidget.
 * @see KParts::Part::partActivateEvent
 */
class KPARTS_EXPORT PartActivateEvent : public QEvent
{
public:
    PartActivateEvent(bool activated, Part *part, QWidget *widget);
    ~PartActivateEvent() override;
    bool activated() const;

    Part *part() const;
    QWidget *widget() const;

    static bool test(const QEvent *event);

private:
    const std::unique_ptr<PartActivateEventPrivate> d;
};

} // namespace

#endif
