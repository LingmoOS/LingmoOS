#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QSettings>

#include <KF5/KWindowSystem/kwindoweffects.h>


class QSettings;
class QGSettings;

namespace Global {

// 错误类型
enum KERROR_TYPE{
    NO_ERROR = 0
};

//! Read and store application settings
extern bool isWayland;

void global_init();
void global_end();

}
#endif // GLOBAL_H
