/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "controller.h"
#include "consumer_p.h"
#include "manager_p.h"

#include "utils/dbusfuture_p.h"

namespace KActivities
{
class ControllerPrivate
{
};

Controller::Controller(QObject *parent)
    : Consumer(parent)
{
}

Controller::~Controller() = default;

// clang-format off
#define CREATE_SETTER(What)                                                    \
    QFuture<void> Controller::setActivity##What(const QString &id,             \
                                                const QString &value)          \
    {                                                                          \
        return Manager::isServiceRunning()                                     \
                   ? DBusFuture::asyncCall<void>(                              \
                         Manager::activities(),                                \
                         QString::fromLatin1("SetActivity" #What), id, value)  \
                   : DBusFuture::fromVoid();                                   \
    }
// clang-format on

CREATE_SETTER(Name)
CREATE_SETTER(Description)
CREATE_SETTER(Icon)

#undef CREATE_SETTER

QFuture<bool> Controller::setCurrentActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::setCurrentActivity",
    //            "You can not set a non-existent activity to be the current");

    // return Manager::activities()->SetCurrentActivity(id);
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<bool>(Manager::activities(), QStringLiteral("SetCurrentActivity"), id)
                                       : DBusFuture::fromValue(false);
}

QFuture<QString> Controller::addActivity(const QString &name)
{
    Q_ASSERT_X(!name.isEmpty(), "Controller::addActivity", "The activity name can not be an empty string");

    // return Manager::activities()->AddActivity(name);
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<QString>(Manager::activities(), QStringLiteral("AddActivity"), name)
                                       : DBusFuture::fromValue(QString());
}

QFuture<void> Controller::removeActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::removeActivity",
    //            "You can not remove a non-existent activity");

    // Manager::activities()->RemoveActivity(id);
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<void>(Manager::activities(), QStringLiteral("RemoveActivity"), id) : DBusFuture::fromVoid();
}

QFuture<void> Controller::stopActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::stopActivity",
    //            "You can not stop a non-existent activity");

    // Manager::activities()->StopActivity(id);
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<void>(Manager::activities(), QStringLiteral("StopActivity"), id) : DBusFuture::fromVoid();
}

QFuture<void> Controller::startActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::startActivity",
    //            "You can not start an non-existent activity");

    // Manager::activities()->StartActivity(id);
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<void>(Manager::activities(), QStringLiteral("StartActivity"), id) : DBusFuture::fromVoid();
}

QFuture<void> Controller::previousActivity()
{
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<void>(Manager::activities(), QStringLiteral("PreviousActivity")) : DBusFuture::fromVoid();
}

QFuture<void> Controller::nextActivity()
{
    return Manager::isServiceRunning() ? DBusFuture::asyncCall<void>(Manager::activities(), QStringLiteral("NextActivity")) : DBusFuture::fromVoid();
}

} // namespace KActivities

#include "moc_controller.cpp"
