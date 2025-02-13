#include "settings.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("lingmo", "system-monitor")
{
}

int SettingsManager::updateInterval() const
{
    return m_settings.value("updateInterval", 2000).toInt();
}

void SettingsManager::setUpdateInterval(int interval)
{
    if (updateInterval() != interval) {
        m_settings.setValue("updateInterval", interval);
        emit updateIntervalChanged();
    }
}

int SettingsManager::graphPoints() const
{
    return m_settings.value("graphPoints", 60).toInt();
}

void SettingsManager::setGraphPoints(int points)
{
    if (graphPoints() != points) {
        m_settings.setValue("graphPoints", points);
        emit graphPointsChanged();
    }
}

QString SettingsManager::defaultSortBy() const
{
    return m_settings.value("defaultSortBy", "cpu").toString();
}

void SettingsManager::setDefaultSortBy(const QString &field)
{
    if (defaultSortBy() != field) {
        m_settings.setValue("defaultSortBy", field);
        emit defaultSortByChanged();
    }
}

bool SettingsManager::sortAscending() const
{
    return m_settings.value("sortAscending", false).toBool();
}

void SettingsManager::setSortAscending(bool ascending)
{
    if (sortAscending() != ascending) {
        m_settings.setValue("sortAscending", ascending);
        emit sortAscendingChanged();
    }
} 