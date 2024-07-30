/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "achievement.h"

using namespace Attica;

Achievement::Type Achievement::stringToAchievementType(const QString &achievementTypeString)
{
    if (achievementTypeString == QLatin1String("flowing")) {
        return Achievement::FlowingAchievement;
    } else if (achievementTypeString == QLatin1String("stepped")) {
        return Achievement::SteppedAchievement;
    } else if (achievementTypeString == QLatin1String("namedsteps")) {
        return Achievement::NamedstepsAchievement;
    } else if (achievementTypeString == QLatin1String("set")) {
        return Achievement::SetAchievement;
    }

    Q_ASSERT(false);
    return Achievement::FlowingAchievement;
}

QString Achievement::achievementTypeToString(const Achievement::Type type)
{
    switch (type) {
    case Achievement::FlowingAchievement:
        return QStringLiteral("flowing");
    case Achievement::SteppedAchievement:
        return QStringLiteral("stepped");
    case Achievement::NamedstepsAchievement:
        return QStringLiteral("namedsteps");
    case Achievement::SetAchievement:
        return QStringLiteral("set");
    }

    Q_ASSERT(false);
    return QString();
}

Achievement::Visibility Achievement::stringToAchievementVisibility(const QString &achievementVisibilityString)
{
    if (achievementVisibilityString == QLatin1String("visible")) {
        return Achievement::VisibleAchievement;
    } else if (achievementVisibilityString == QLatin1String("dependents")) {
        return Achievement::DependentsAchievement;
    } else if (achievementVisibilityString == QLatin1String("secret")) {
        return Achievement::SecretAchievement;
    }

    Q_ASSERT(false);
    return Achievement::VisibleAchievement;
}

QString Achievement::achievementVisibilityToString(const Achievement::Visibility visibility)
{
    switch (visibility) {
    case Achievement::VisibleAchievement:
        return QStringLiteral("visible");
    case Achievement::DependentsAchievement:
        return QStringLiteral("dependents");
    case Achievement::SecretAchievement:
        return QStringLiteral("secret");
    }

    Q_ASSERT(false);
    return QString();
}

class Q_DECL_HIDDEN Achievement::Private : public QSharedData
{
public:
    QString m_id;
    QString m_contentId;
    QString m_name;
    QString m_description;
    QString m_explanation;
    int m_points;
    QUrl m_image;
    QStringList m_dependencies;
    Achievement::Visibility m_visibility;
    Achievement::Type m_type;
    QStringList m_options;
    int m_steps;
    QVariant m_progress;

    Private()
        : m_points(0)
        , m_steps(0)
    {
    }
};

Achievement::Achievement()
    : d(new Private)
{
}

Achievement::Achievement(const Achievement &other)
    : d(other.d)
{
}

Achievement &Achievement::operator=(const Attica::Achievement &other)
{
    d = other.d;
    return *this;
}

Achievement::~Achievement()
{
}

void Achievement::setId(const QString &id)
{
    d->m_id = id;
}

QString Achievement::id() const
{
    return d->m_id;
}

void Achievement::setContentId(const QString &contentId)
{
    d->m_contentId = contentId;
}

QString Achievement::contentId() const
{
    return d->m_contentId;
}

void Achievement::setName(const QString &name)
{
    d->m_name = name;
}

QString Achievement::name() const
{
    return d->m_name;
}

void Achievement::setDescription(const QString &description)
{
    d->m_description = description;
}

QString Achievement::description() const
{
    return d->m_description;
}

void Achievement::setExplanation(const QString &explanation)
{
    d->m_explanation = explanation;
}

QString Achievement::explanation() const
{
    return d->m_explanation;
}

void Achievement::setPoints(const int points)
{
    d->m_points = points;
}

int Achievement::points() const
{
    return d->m_points;
}

void Achievement::setImage(const QUrl &image)
{
    d->m_image = image;
}

QUrl Achievement::image() const
{
    return d->m_image;
}

void Achievement::setDependencies(const QStringList &dependencies)
{
    d->m_dependencies = dependencies;
}

void Achievement::addDependency(const QString &dependency)
{
    d->m_dependencies.append(dependency);
}

void Achievement::removeDependency(const QString &dependency)
{
    d->m_dependencies.removeOne(dependency);
}

QStringList Achievement::dependencies() const
{
    return d->m_dependencies;
}

void Achievement::setVisibility(Achievement::Visibility visibility)
{
    d->m_visibility = visibility;
}

Achievement::Visibility Achievement::visibility() const
{
    return d->m_visibility;
}

void Achievement::setType(Achievement::Type type)
{
    d->m_type = type;
}

Achievement::Type Achievement::type() const
{
    return d->m_type;
}

void Achievement::setOptions(const QStringList &options)
{
    d->m_options = options;
}

void Achievement::addOption(const QString &option)
{
    d->m_options.append(option);
}

void Achievement::removeOption(const QString &option)
{
    d->m_options.removeOne(option);
}

QStringList Achievement::options() const
{
    return d->m_options;
}

void Achievement::setSteps(const int steps)
{
    d->m_steps = steps;
}

int Achievement::steps() const
{
    return d->m_steps;
}

void Achievement::setProgress(const QVariant &progress)
{
    d->m_progress = progress;
}

QVariant Achievement::progress() const
{
    return d->m_progress;
}

bool Achievement::isValid() const
{
    return !(d->m_id.isEmpty());
}
