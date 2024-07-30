/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDESKTOPFILEACTION_H
#define KDESKTOPFILEACTION_H
#include <kconfigcore_export.h>

#include <QSharedDataPointer>
#include <QString>

class KDesktopFileActionPrivate;

/**
 * Class for representing an Action of a desktop file
 *
 * @since 6.0
 */
class KCONFIGCORE_EXPORT KDesktopFileAction
{
public:
    /**
     * Construct an empty KDesktopFileAction. Needed so the Action can be stored in containers that require type T to be
     * default constructible (e.g. QVariant).
     */
    explicit KDesktopFileAction();
    /**
     * Construct a KDesktopFileAction with all required properties
     */
    explicit KDesktopFileAction(const QString &name, const QString &text, const QString &icon, const QString &exec, const QString &desktopFilePath);

    KDesktopFileAction(const KDesktopFileAction &other);
    KDesktopFileAction &operator=(const KDesktopFileAction &other);
    KDesktopFileAction(KDesktopFileAction &&other);
    KDesktopFileAction &operator=(KDesktopFileAction &&other);
    ~KDesktopFileAction();

    /**
     * @return the action's internal name
     * For instance Actions=Setup;... and the group [Desktop Action Setup]
     * define an action with the name "Setup".
     */
    QString actionsKey() const;

    /**
     * @return Path of the desktop file this action was loaded from
     */
    QString desktopFilePath() const;

    /**
     * @return the action's Name, as defined by the Name key in the desktop action group
     */
    QString name() const;

    /**
     * @return the action's icon, as defined by the Icon key in the desktop action group
     */
    QString icon() const;

    /**
     * @return the action's exec command, as defined by the Exec key in the desktop action group
     */
    QString exec() const;

    /**
     * Returns whether the action is a separator.
     * This is true when the Actions key contains "_SEPARATOR_".
     */
    bool isSeparator() const;

private:
    QSharedDataPointer<KDesktopFileActionPrivate> d;
};
#endif
