#include <QDBusArgument>

#ifndef DBUSTYPES_H
#define DBUSTYPES_H

struct IconPixmap {
    int width;
    int height;
    QByteArray bytes;
};

typedef QList<IconPixmap> IconPixmapList;

Q_DECLARE_METATYPE(IconPixmap)
Q_DECLARE_METATYPE(IconPixmapList)

struct ToolTip {
    QString iconName;
    QList<IconPixmap> iconPixmap;
    QString title;
    QString description;
};

Q_DECLARE_METATYPE(ToolTip)

QDBusArgument &operator<<(QDBusArgument &argument, const IconPixmap &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, IconPixmap &icon);

QDBusArgument &operator<<(QDBusArgument &argument, const ToolTip &toolTip);
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolTip &toolTip);

#endif // DBUSTYPES_H