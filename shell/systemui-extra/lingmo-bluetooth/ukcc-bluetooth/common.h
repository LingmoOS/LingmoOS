#ifndef COMMON_H
#define COMMON_H

#include <QObject>

#define STR_MOUSE       "MOUSE"
#define STR_TOUCHPAD    "TOUCHPAD"
#define STR_KEYBOARD    "KEYBOARD"
#define STR_TRACKPOINT  "TRACKPOINT"

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);
    static unsigned int getSystemCurrentMouseAndTouchPadDevCount();
    static unsigned int getSystemCurrentKeyBoardDevCount();
signals:

};

#endif // COMMON_H
