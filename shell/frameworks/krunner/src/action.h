// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#ifndef KRUNNER_ACTION_H
#define KRUNNER_ACTION_H

#include "krunner_export.h"

#include <QMetaType>
#include <QString>
#include <memory>

namespace KRunner
{
class ActionPrivate;
/**
 * This class represents an action that will be shown next to a match.
 * The goal is to make it more reliable, because QIcon::fromTheme which is often needed in a QAction constructor is not thread safe.
 * Also, it makes the API more consistent with the org.kde.krunner1 DBus interface and forces consumers to set an icon.
 *
 * @since 6.0
 */
class KRUNNER_EXPORT Action final
{
    Q_GADGET
    /// User-visible text
    Q_PROPERTY(QString text READ text CONSTANT)
    /// Source for the icon: Name of the icon from a theme, file path or file URL
    Q_PROPERTY(QString iconSource READ iconSource CONSTANT)
public:
    /**
     * Constructs a new action
     * @param id ID which identifies the action uniquely within the context of the respective runner plugin
     * @param iconSource name for the icon, that can be passed in to QIcon::fromTheme or file path/URL
     */
    explicit Action(const QString &id, const QString &iconSource, const QString &text);

    /// Empty constructor creating invalid action
    Action();

    ~Action();

    /**
     * Copy constructor
     * @internal
     */
    Action(const KRunner::Action &other);

    Action &operator=(const Action &other);

    /// Check if the action is valid
    explicit operator bool() const
    {
        return !id().isEmpty();
    }

    bool operator==(const KRunner::Action &other) const
    {
        return id() == other.id();
    }

    QString id() const;
    QString text() const;
    QString iconSource() const;

private:
    std::unique_ptr<ActionPrivate> d;
};

using Actions = QList<KRunner::Action>;
}

Q_DECLARE_METATYPE(KRunner::Action)
#endif
