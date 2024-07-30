/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GLOBALSHORTCUTCONTEXT_H
#define GLOBALSHORTCUTCONTEXT_H

#include "kglobalshortcutinfo.h"

#include <QHash>
#include <QString>

#include <kglobalaccel.h>

class Component;
class GlobalShortcut;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class GlobalShortcutContext
{
public:
    /**
     * Default constructor
     */
    GlobalShortcutContext(const QString &uniqueName, const QString &friendlyName, Component *component);

    /**
     * Destructor
     */
    virtual ~GlobalShortcutContext();

    //! Adds @p shortcut to the context
    void addShortcut(GlobalShortcut *shortcut);

    //! Return KGlobalShortcutInfos for all shortcuts
    QList<KGlobalShortcutInfo> allShortcutInfos() const;

    /**
     * Get the name for the context
     */
    QString uniqueName() const;
    QString friendlyName() const;

    Component *component();
    Component const *component() const;

    //! Get shortcut for @p key or nullptr
    GlobalShortcut *getShortcutByKey(const QKeySequence &key, KGlobalAccel::MatchType type) const;

    //! Remove @p shortcut from the context. The shortcut is not deleted.
    GlobalShortcut *takeShortcut(GlobalShortcut *shortcut);

    // Returns true if key is not used by any global shortcuts in this context,
    // otherwise returns false
    bool isShortcutAvailable(const QKeySequence &key) const;

private:
    friend class Component;
    friend class KServiceActionComponent;

    //! The unique name for this context
    QString _uniqueName;

    //! The unique name for this context
    QString _friendlyName;

    //! The component the context belongs to
    Component *_component = nullptr;

    //! The actions associated with this context
    QHash<QString, GlobalShortcut *> _actionsMap;
};

#endif /* #ifndef GLOBALSHORTCUTCONTEXT_H */
