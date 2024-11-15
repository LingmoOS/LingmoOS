#include "timezonemap.h"
#include "timedated_interface.h"

#include <QFile>
#include <QDebug>
#include <QTimeZone>

#include <math.h>
#include <locale.h>

// gnome-control-center: cc-timezone-map.c
static double radians(double degrees) {
    return (degrees / 360.0) * M_PI * 2;
}

static double convert_latitude_to_y(double latitude) {
    const double bottom_lat = -59;
    const double top_lat = 81;
    const double full_range = 4.6068250867599998;
    double top_per, y, top_offset, map_range;

    top_per = top_lat / 180.0;
    y = 1.25 * log(tan(M_PI_4 + 0.4 * radians(latitude)));
    top_offset = full_range * top_per;
    map_range = fabs(1.25 * log(tan(M_PI_4 + 0.4 * radians(bottom_lat))) - top_offset);
    y = fabs(y - top_offset);
    y = y / map_range;

    return y;
}

static double convert_longitude_to_x(double longitude) {
    const double xdeg_offset = -6;
    return ((180.0 + longitude) / 360.0 + xdeg_offset / 180.0);
}

// Parse latitude and longitude of the zone's principal location.
// See https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
// |pos| is in ISO 6709 sign-degrees-minutes-seconds format,
// either +-DDMM+-DDDMM or +-DDMMSS+-DDDMMSS.
// |digits| 2 for latitude, 3 for longitude.
double convert_pos(const QString &pos, int digits) {
    if (pos.length() < 4 || digits > 9) {
        return 0.0;
    }

    const QString integer = pos.left(digits + 1);
    const QString fraction = pos.mid(digits + 1);
    const double t1 = integer.toDouble();
    const double t2 = fraction.toDouble();
    if (t1 > 0.0) {
        return t1 + t2 / pow(10.0, fraction.length());
    } else {
        return t1 - t2 / pow(10.0, fraction.length());
    }
}

TimeZoneMap::TimeZoneMap(QObject *parent)
    : QObject(parent)
{
    initDatas();
}

void TimeZoneMap::clicked(int x, int y, int width, int height)
{
    double minimum_distance = width * width + height * height;
    int nearest_zone_index = -1;

    QList<TimeZoneItem *> items;

    for (int i = 0; i < m_list.size(); ++i) {
        TimeZoneItem *item = m_list.at(i);
        const double point_x = convert_longitude_to_x(item->longitude) * width;
        const double point_y = convert_latitude_to_y(item->latitude) * height;
        const double dx = point_x - x;
        const double dy = point_y - y;
        const double distance = dx * dx + dy * dy;

        if (distance < minimum_distance) {
            minimum_distance = distance;
            nearest_zone_index = i;
        }

        if (distance <= 20) {
            items << item;
        }
    }

    if (items.isEmpty() && nearest_zone_index != -1) {
        items.append(m_list.at(nearest_zone_index));
    }

    m_currentList.clear();
    for (TimeZoneItem *item : items) {
        m_currentList.append(item->timeZone);
    }

    std::sort(m_currentList.begin(), m_currentList.end());

    emit availableListChanged();
}

void TimeZoneMap::setTimeZone(QString value)
{
    if (value.isEmpty())
        return;

    OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                                   QStringLiteral("/org/freedesktop/timedate1"),
                                                   QDBusConnection::systemBus());

    auto reply = iface.SetTimezone(value, true);
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "Failed to set timezone" << reply.error().message();
        return;
    }

    m_currentTimeZone = QTimeZone(value.toLatin1()).id();
    emit currentTimeZoneChanged();
}

QString TimeZoneMap::currentTimeZone() const
{
    return localeTimeZoneName(m_currentTimeZone);
}

QStringList TimeZoneMap::availableList()
{
    return m_currentList;
}

QString TimeZoneMap::localeTimeZoneName(const QString &timeZone) const
{
    const QString locale = QLocale::system().name();
    setlocale(LC_ALL, QString(locale + ".UTF-8").toStdString().c_str());
    int index = timeZone.lastIndexOf('/');
    setlocale(LC_ALL, "en_US.UTF-8");
    return (index > -1) ? timeZone.mid(index + 1) : timeZone;
}

void TimeZoneMap::initDatas()
{
    QFile zoneTab("/usr/share/zoneinfo/zone.tab");
    if (!zoneTab.open(QIODevice::ReadOnly)) {
        return;
    }

    const auto lines = QString::fromUtf8(zoneTab.readAll()).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (line.startsWith('#'))
            continue;

        const QStringList parts = line.split('\t');
        if (parts.size() < 3)
            continue;

        const QString coordinate = parts.at(1);
        int index = coordinate.indexOf('+', 3);
        if (index == -1) {
            index = coordinate.indexOf('-', 3);
        }

        const double latitude = convert_pos(coordinate.left(index), 2);
        const double longitude = convert_pos(coordinate.mid(index), 3);

        TimeZoneItem *item = new TimeZoneItem;
        item->country = parts.at(0);
        item->timeZone = parts.at(2);
        item->latitude = latitude;
        item->longitude = longitude;
        m_list.append(item);
    }
    std::sort(m_list.begin(), m_list.end());

    // currentTimeZoneChanged
    m_currentTimeZone = QTimeZone(QTimeZone::systemTimeZoneId()).id();
    emit currentTimeZoneChanged();
}
