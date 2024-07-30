/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTINFO_H
#define KGLOBALSHORTCUTINFO_H

#include <kglobalaccel_export.h>

#include <QDBusArgument>
#include <QKeySequence>
#include <QList>
#include <QObject>

class KGlobalShortcutInfoPrivate;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KGLOBALACCEL_EXPORT KGlobalShortcutInfo : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.kglobalaccel.KShortcutInfo")

    /* clang-format off */
    Q_SCRIPTABLE Q_PROPERTY(QString uniqueName READ uniqueName)
    Q_SCRIPTABLE Q_PROPERTY(QString friendlyName READ friendlyName)

    Q_SCRIPTABLE Q_PROPERTY(QString componentUniqueName READ componentUniqueName)
    Q_SCRIPTABLE Q_PROPERTY(QString componentFriendlyName READ componentFriendlyName)

    Q_SCRIPTABLE Q_PROPERTY(QString contextUniqueName READ contextUniqueName)
    Q_SCRIPTABLE Q_PROPERTY(QString contextFriendlyName READ contextFriendlyName)

    Q_SCRIPTABLE Q_PROPERTY(QList<QKeySequence> keys READ keys)
    Q_SCRIPTABLE Q_PROPERTY(QList<QKeySequence> defaultKeys READ keys)

public:
    KGlobalShortcutInfo();
    /* clang-format on */

    KGlobalShortcutInfo(const KGlobalShortcutInfo &rhs);

    ~KGlobalShortcutInfo() override;

    KGlobalShortcutInfo &operator=(const KGlobalShortcutInfo &rhs);

    QString contextFriendlyName() const;

    QString contextUniqueName() const;

    QString componentFriendlyName() const;

    QString componentUniqueName() const;

    QList<QKeySequence> defaultKeys() const;

    QString friendlyName() const;

    QList<QKeySequence> keys() const;

    QString uniqueName() const;

private:
    friend class GlobalShortcut;

    friend KGLOBALACCEL_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfo &shortcut);
    friend KGLOBALACCEL_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, QKeySequence &sequence);

    //! Implementation details
    KGlobalShortcutInfoPrivate *d;
};

KGLOBALACCEL_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfo &shortcut);
KGLOBALACCEL_EXPORT QDBusArgument &operator<<(QDBusArgument &argument, const QKeySequence &sequence);

KGLOBALACCEL_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfo &shortcut);
KGLOBALACCEL_EXPORT const QDBusArgument &operator>>(const QDBusArgument &argument, QKeySequence &sequence);

Q_DECLARE_METATYPE(KGlobalShortcutInfo)

#endif /* #ifndef KGLOBALSHORTCUTINFO_H */
