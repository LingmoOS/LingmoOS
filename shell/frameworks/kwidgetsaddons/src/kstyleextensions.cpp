/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Thomas Lübking <thomas.luebking@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstyleextensions.h"

#include <QWidget>

namespace KStyleExtensions
{
/*
    Custom Style Element runtime extension:
    We reserve one StyleHint to let the effective style inform widgets whether it supports certain
    string based style elements.
    As this could lead to number conflicts (i.e. an app utilizing one of the hints itself for other
    purposes) there're various safety mechanisms to rule out such interference.

    1) It's most unlikely that a widget in some 3rd party app will accidentally call a general
    QStyle/KStyle styleHint() or draw*() and (unconditionally) expect a valid return, however:
        a. The StyleHint is not directly above Qt's custom base, assuming most 3rd party apps would
           - in case - make use of such
        b. In order to be accepted, the StyleHint query must pass a widget with a perfectly matching
           name, containing the typical element prefix ("CE_", etc.) and being supported by the current
           style
        c. Instead using Qt's fragile qstyleoption_cast on the QStyleOption provided to the StyleHint
           query, try to dump out a string and hope for the best, we now manipulate the widgets
           objectName().
           Plain Qt dependent widgets can do that themselves and if a widget uses KStyle's
           convenience access functions, it won't notice this at all

    2) The key problem is that a common KDE widget will run into an apps custom style which will then
    falsely respond to the styleHint() call with an invalid value.
    To prevent this, supporting styles *must* set a Q_CLASSINFO "X-KDE-CustomElements".

    3) If any of the above traps snaps, the returned id is 0 - the QStyle default, indicating
    that this element is not supported by the current style.

    Obviously, this contains the "diminished clean" action to (temporarily) manipulate the
    objectName() of a const QWidget* - but this happens completely inside KStyle or the widget, if
    it does not make use of KStyles static convenience functions.
    My biggest worry here would be, that in a multithreaded environment a thread (usually not being
    owner of the widget) does something crucially relying on the widgets name property...
    This however would also have to happen during the widget construction or stylechanges, when
    the functions in doubt will typically be called.
    So this is imho unlikely causing any trouble, ever.
*/

/// @private Prevent kapidox's doxygen config to pick up this namespace variable
static const QStyle::StyleHint SH_KCustomStyleElement = (QStyle::StyleHint)0xff000001;
/// @private Prevent kapidox's doxygen config to pick up this namespace variable
static const int X_KdeBase = 0xff000000;

/*
    The functions called by widgets that request custom element support, passed to the effective style.
    Collected in a static inline function due to similarity.
*/

/// @private Prevent kapidox's doxygen config to pick up this namespace method
static inline int customStyleElement(QStyle::StyleHint type, const QString &element, QWidget *widget)
{
    if (!widget || widget->style()->metaObject()->indexOfClassInfo("X-KDE-CustomElements") < 0) {
        return 0;
    }

    const QString originalName = widget->objectName();
    widget->setObjectName(element);
    const int id = widget->style()->styleHint(type, nullptr, widget);
    widget->setObjectName(originalName);
    return id;
}

QStyle::StyleHint customStyleHint(const QString &element, const QWidget *widget)
{
    return (QStyle::StyleHint)customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

QStyle::ControlElement customControlElement(const QString &element, const QWidget *widget)
{
    return (QStyle::ControlElement)customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

QStyle::SubElement customSubElement(const QString &element, const QWidget *widget)
{
    return (QStyle::SubElement)customStyleElement(SH_KCustomStyleElement, element, const_cast<QWidget *>(widget));
}

}
