#ifndef MYPLUGIN_H
#define MYPLUGIN_H
#include <QtPlugin>
#include "filedialoginterface.h"
class FileDialogPlugin : public QObject,public FileDialogInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QGenericPluginFactoryInterface" FILE "filedialogplugin.json")
    Q_INTERFACES(FileDialogInterface);
#endif // QT_VERSION >= 0x050000
public:
    QPlatformDialogHelper *create();
};
#endif
