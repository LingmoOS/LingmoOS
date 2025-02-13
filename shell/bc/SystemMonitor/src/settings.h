#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(int graphPoints READ graphPoints WRITE setGraphPoints NOTIFY graphPointsChanged)
    Q_PROPERTY(QString defaultSortBy READ defaultSortBy WRITE setDefaultSortBy NOTIFY defaultSortByChanged)
    Q_PROPERTY(bool sortAscending READ sortAscending WRITE setSortAscending NOTIFY sortAscendingChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    int updateInterval() const;
    void setUpdateInterval(int interval);

    int graphPoints() const;
    void setGraphPoints(int points);

    QString defaultSortBy() const;
    void setDefaultSortBy(const QString &field);

    bool sortAscending() const;
    void setSortAscending(bool ascending);

signals:
    void updateIntervalChanged();
    void graphPointsChanged();
    void defaultSortByChanged();
    void sortAscendingChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGS_H 