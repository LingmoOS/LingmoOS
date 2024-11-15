#ifndef TIMEZONEDATA_H
#define TIMEZONEDATA_H

#include <QString>

class TimeZoneData
{
public:
    QString id;
    QString region;
    QString city;
    QString comment;
    bool checked;
    int offsetFromUtc;
};

#endif // TIMEZONEDATA_H
