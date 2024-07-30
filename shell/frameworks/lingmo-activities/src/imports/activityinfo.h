/*
    SPDX-FileCopyrightText: 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACTIVITIES_IMPORTS_ACTIVITY_INFO_H
#define KACTIVITIES_IMPORTS_ACTIVITY_INFO_H

// Qt
#include <QObject>
#include <qqmlregistration.h>

// STL
#include <memory>

// Local
#include <lib/controller.h>
#include <lib/info.h>

namespace KActivities
{
namespace Imports
{
/**
 * ActivityInfo
 */

class ActivityInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * Unique identifier of the activity
     */
    Q_PROPERTY(QString activityId READ activityId WRITE setActivityId NOTIFY activityIdChanged)

    /**
     * Name of the activity
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /**
     * Name of the activity
     */
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)

    /**
     * Activity icon
     */
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

    /**
     * Is the activity a valid one - does it exist?
     */
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    explicit ActivityInfo(QObject *parent = nullptr);
    ~ActivityInfo() override;

public Q_SLOTS:
    void setActivityId(const QString &id);
    QString activityId() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setIcon(const QString &icon);
    QString icon() const;

    bool valid() const;

Q_SIGNALS:
    void activityIdChanged(const QString &id);
    void nameChanged(const QString &name);
    void descriptionChanged(const QString &description);
    void iconChanged(const QString &icon);
    void validChanged(bool valid);

private Q_SLOTS:
    void setCurrentActivity(const QString &id);

private:
    void setIdInternal(const QString &id);

    KActivities::Controller m_service;
    std::unique_ptr<KActivities::Info> m_info;
    bool m_showCurrentActivity;
};

} // namespace Imports
} // namespace KActivities

#endif // KACTIVITIES_IMPORTS_ACTIVITY_INFO_H
