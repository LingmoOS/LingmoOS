/*
    SPDX-FileCopyrightText: 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Self
#include "activityinfo.h"

namespace KActivities
{
namespace Imports
{
ActivityInfo::ActivityInfo(QObject *parent)
    : QObject(parent)
    , m_showCurrentActivity(false)
{
    connect(&m_service, &KActivities::Controller::currentActivityChanged, this, &ActivityInfo::setCurrentActivity);
}

ActivityInfo::~ActivityInfo()
{
}

void ActivityInfo::setCurrentActivity(const QString &id)
{
    if (!m_showCurrentActivity) {
        return;
    }

    setIdInternal(id);

    Q_EMIT nameChanged(m_info->name());
    Q_EMIT descriptionChanged(m_info->description());
    Q_EMIT iconChanged(m_info->icon());
}

void ActivityInfo::setActivityId(const QString &id)
{
    m_showCurrentActivity = (id == QLatin1String(":current"));

    setIdInternal(m_showCurrentActivity ? m_service.currentActivity() : id);
}

void ActivityInfo::setIdInternal(const QString &id)
{
    using namespace KActivities;

    // We are killing the old info object, if any
    m_info.reset(new KActivities::Info(id));

    auto ptr = m_info.get();

    connect(ptr, &Info::nameChanged, this, &ActivityInfo::nameChanged);
    connect(ptr, &Info::descriptionChanged, this, &ActivityInfo::descriptionChanged);
    connect(ptr, &Info::iconChanged, this, &ActivityInfo::iconChanged);
}
// clang-format off
#define CREATE_GETTER_AND_SETTER(WHAT, What)                                   \
    QString ActivityInfo::What() const                                         \
    {                                                                          \
        return m_info ? m_info->What() : QString();                            \
    }                                                                          \
                                                                               \
    void ActivityInfo::set##WHAT(const QString &value)                         \
    {                                                                          \
        if (!m_info)                                                           \
            return;                                                            \
                                                                               \
        m_service.setActivity##WHAT(m_info->id(), value);                      \
    }
// clang-format on

CREATE_GETTER_AND_SETTER(Name, name)
CREATE_GETTER_AND_SETTER(Description, description)
CREATE_GETTER_AND_SETTER(Icon, icon)

#undef CREATE_GETTER_AND_SETTER

QString ActivityInfo::activityId() const
{
    return m_info ? m_info->id() : QString();
}

bool ActivityInfo::valid() const
{
    return true;
}

} // namespace Imports
} // namespace KActivities

#include "moc_activityinfo.cpp"
