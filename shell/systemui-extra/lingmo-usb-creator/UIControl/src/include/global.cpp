#include "global.h"

#include <QSettings>
#include <QGSettings>
#include <QApplication>
#include <QFile>
#include <QDebug>

bool Global::isWayland = false;

using namespace Global;

void Global::global_init() {

    // 运行环境
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        qputenv("QT_QPA_PLATFORM", "wayland");
        isWayland = true;
    } else {
        isWayland = false;
    }

}

void Global::global_end() {

}
