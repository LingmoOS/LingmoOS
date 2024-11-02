#ifndef X11DEVICES_H
#define X11DEVICES_H

#include <QObject>

#define STR_MOUSE       "MOUSE"
#define STR_TOUCHPAD    "TOUCHPAD"
#define STR_KEYBOARD    "KEYBOARD"
#define STR_TRACKPOINT  "TRACKPOINT"

class x11Devices : public QObject
{
    Q_OBJECT
public:
    explicit x11Devices(QObject *parent = nullptr);
    static unsigned int getSystemCurrentMouseAndTouchPadDevCount();
    static unsigned int getSystemCurrentKeyBoardDevCount();

};

#endif // X11DEVICES_H
