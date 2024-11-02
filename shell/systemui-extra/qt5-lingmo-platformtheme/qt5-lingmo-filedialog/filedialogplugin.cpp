#include "filedialogplugin.h"
#include <QDebug>
#include "kyfiledialog.h"
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plugin, myPlugin)
#endif // QT_VERSION < 0x050000

QPlatformDialogHelper *FileDialogPlugin::create()
{
    return new LINGMOFileDialog::KyFileDialogHelper;
}

