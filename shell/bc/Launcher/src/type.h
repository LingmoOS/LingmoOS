
#ifndef TYPE_H
#define TYPE_H

#include <QObject>

class QString;

namespace KWayland
{
    namespace Client
    {
        class PlasmaWindowManagement;
        class PlasmaWindow;
    }
}

enum LauncherLocation {
    Grid = 0,
    Favorites,
    Desktop
};

struct ApplicationData {
    QString name;
    QString icon;
    QString storageId;
    QString entryPath;
    LauncherLocation location = LauncherLocation::Grid;
    bool startupNotify = true;
    KWayland::Client::PlasmaWindow *window = nullptr;
};

#endif // TYPE_H
