/*
 *   SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *   SPDX-FileCopyrightText: 2012 Makis Marimpis <makhsm@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "GlobalShortcutsPlugin.h"

#include <QAction>
#include <QString>
#include <QStringList>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>

K_PLUGIN_CLASS(GlobalShortcutsPlugin)

const auto objectNamePattern = QStringLiteral("switch-to-activity-%1");
const auto objectNamePatternLength = objectNamePattern.length() - 2;

GlobalShortcutsPlugin::GlobalShortcutsPlugin(QObject *parent)
    : Plugin(parent)
    , m_activitiesService(nullptr)
    , m_actionCollection(new KActionCollection(this))
{
    m_actionCollection->setComponentName("ActivityManager");
    m_actionCollection->setComponentDisplayName(i18n("Activity Manager"));
}

GlobalShortcutsPlugin::~GlobalShortcutsPlugin()
{
    m_actionCollection->clear();
}

bool GlobalShortcutsPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_activitiesService = modules["activities"];

    m_activitiesList = Plugin::retrieve<QStringList>(m_activitiesService, "ListActivities");

    for (const auto &activity : m_activitiesList) {
        activityAdded(activity);
    }
    connect(this, SIGNAL(currentActivityChanged(QString)), m_activitiesService, SLOT(SetCurrentActivity(QString)), Qt::QueuedConnection);

    connect(m_activitiesService, SIGNAL(ActivityAdded(QString)), this, SLOT(activityAdded(QString)));
    connect(m_activitiesService, SIGNAL(ActivityRemoved(QString)), this, SLOT(activityRemoved(QString)));

    m_actionCollection->readSettings();

    return true;
}

void GlobalShortcutsPlugin::activityAdded(const QString &activity)
{
    if (activity == "00000000-0000-0000-0000-000000000000") {
        return;
    }

    if (!m_activitiesList.contains(activity)) {
        m_activitiesList << activity;
    }

    const auto action = m_actionCollection->addAction(objectNamePattern.arg(activity));

    action->setText(i18nc("@action", "Switch to activity \"%1\"", activityName(activity)));
    KGlobalAccel::setGlobalShortcut(action, QList<QKeySequence>{});

    connect(action, &QAction::triggered, this, [this, activity]() {
        Q_EMIT currentActivityChanged(activity);
    });

    // m_actionCollection->writeSettings();
}

QString GlobalShortcutsPlugin::activityForAction(QAction *action) const
{
    return action->objectName().mid(objectNamePatternLength);
}

void GlobalShortcutsPlugin::activityRemoved(const QString &deletedActivity)
{
    m_activitiesList.removeAll(deletedActivity);

    // Removing all shortcuts that refer to an unknown activity
    for (const auto &action : m_actionCollection->actions()) {
        const auto actionActivity = activityForAction(action);
        if ((deletedActivity.isEmpty() && !m_activitiesList.contains(actionActivity)) //
            || deletedActivity == actionActivity) {
            KGlobalAccel::self()->removeAllShortcuts(action);
            m_actionCollection->removeAction(action);
        }
    }

    m_actionCollection->writeSettings();
}

void GlobalShortcutsPlugin::activityChanged(const QString &activity)
{
    for (const auto &action : m_actionCollection->actions()) {
        if (activity == activityForAction(action)) {
            action->setText(i18nc("@action", "Switch to activity \"%1\"", activityName(activity)));
        }
    }
}

QString GlobalShortcutsPlugin::activityName(const QString &activity) const
{
    return Plugin::retrieve<QString>(m_activitiesService, "ActivityName", Q_ARG(QString, activity));
}

#include "GlobalShortcutsPlugin.moc"

#include "moc_GlobalShortcutsPlugin.cpp"
