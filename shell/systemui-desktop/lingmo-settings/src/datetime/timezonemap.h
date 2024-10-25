#ifndef TIMEZONEMAP_H
#define TIMEZONEMAP_H

#include <QObject>

class TimeZoneItem {
public:
    QString country;
    QString timeZone;
    double latitude;
    double longitude;
};

class TimeZoneMap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableList READ availableList NOTIFY availableListChanged)
    Q_PROPERTY(QString currentTimeZone READ currentTimeZone NOTIFY currentTimeZoneChanged)

public:
    explicit TimeZoneMap(QObject *parent = nullptr);

    Q_INVOKABLE void clicked(int x, int y, int width, int height);
    Q_INVOKABLE void setTimeZone(QString value);

    QString currentTimeZone() const;
    QStringList availableList();

    Q_INVOKABLE QString localeTimeZoneName(const QString &timeZone) const;

signals:
    void availableListChanged();
    void currentTimeZoneChanged();

private:
    void initDatas();

private:
    QList<TimeZoneItem *> m_list;
    QString m_currentTimeZone;
    QStringList m_currentList;
};

#endif // TIMEZONEMAP_H
