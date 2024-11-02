#include "xrandrobject.h"
#include <QApplication>
#include <QDesktopWidget>

namespace KServer {
QRect GetScreenRect()
{
    QRect rect = QApplication::desktop()->screenGeometry();
    return (QRect)rect;
}
}
