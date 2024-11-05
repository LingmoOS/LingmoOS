#ifndef TIMEZONEMODEL_H
#define TIMEZONEMODEL_H

#include <QAbstractListModel>

struct TimeZoneStruct
{
    QString country;//国家
    QString zone;//洲
    QString TZcity;//时区城市
    QString latitude;//纬度
    QString longitude;//经度
    QString encity;//

};

typedef QList<TimeZoneStruct> TimeZoneStructList;

class TimeZoneModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TimeZoneModel(QObject *parent = nullptr);


    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setCurZone(int index);

    TimeZoneStruct getLatLongByTZCity(QModelIndex index);
    TimeZoneStruct getcurTimeZoneStruct();

//    QStringList getTZCityByLatLong(QString lat, QString lon);

    QModelIndex getIndexByCountryCode(QString code);

    QModelIndex getIndexByTZCity(QString tzcity);
    QModelIndex getIndexByLatLong(QStringList latlon);
    TimeZoneStructList getZoneList(){return m_TZLists;}
    void flushZone();

    QString filestr;
signals:
    void curZoneChanged();

private:
    TimeZoneStructList m_TZLists;
    TimeZoneStruct m_curZone;
};
QString getZoneList();
TimeZoneStructList loadZoneFile();
#endif // TIMEZONEMODEL_H
