/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_ACHIEVEMENT_H
#define ATTICA_ACHIEVEMENT_H

#include "attica_export.h"

#include <QSharedDataPointer>
#include <QStringList>
#include <QUrl>
#include <QVariant>

namespace Attica
{

/**
 * @class Achievement achievement.h <Attica/Achievement>
 *
 * Represents an achievement.
 */
class ATTICA_EXPORT Achievement
{
public:
    typedef QList<Achievement> List;
    class Parser;

    enum Type {
        FlowingAchievement,
        SteppedAchievement,
        NamedstepsAchievement,
        SetAchievement,
    };
    static Achievement::Type stringToAchievementType(const QString &achievementTypeString);
    static QString achievementTypeToString(const Achievement::Type type);

    enum Visibility {
        VisibleAchievement,
        DependentsAchievement,
        SecretAchievement,
    };
    static Achievement::Visibility stringToAchievementVisibility(const QString &achievementVisibilityString);
    static QString achievementVisibilityToString(const Achievement::Visibility visibility);

    Achievement();
    Achievement(const Achievement &other);
    Achievement &operator=(const Achievement &other);
    ~Achievement();

    void setId(const QString &id);
    QString id() const;

    void setContentId(const QString &contentId);
    QString contentId() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setExplanation(const QString &explanation);
    QString explanation() const;

    void setPoints(const int points);
    int points() const;

    void setImage(const QUrl &image);
    QUrl image() const;

    void setDependencies(const QStringList &dependencies);
    void addDependency(const QString &dependency);
    void removeDependency(const QString &dependency);
    QStringList dependencies() const;

    void setVisibility(Achievement::Visibility visibility);
    Achievement::Visibility visibility() const;

    void setType(Achievement::Type type);
    Achievement::Type type() const;

    void setOptions(const QStringList &options);
    void addOption(const QString &option);
    void removeOption(const QString &option);
    QStringList options() const;

    void setSteps(const int steps);
    int steps() const;

    void setProgress(const QVariant &progress);
    QVariant progress() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
