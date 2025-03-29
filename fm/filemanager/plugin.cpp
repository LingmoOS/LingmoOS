#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "model/placesmodel.h"
#include "model/foldermodel.h"
#include "model/pathbarmodel.h"
#include "model/positioner.h"
#include "widgets/rubberband.h"
#include "widgets/itemviewadapter.h"
#include "helper/datehelper.h"
#include "helper/fm.h"
#include "helper/shortcut.h"

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override {
        // qmlRegisterType<OutputModel>(uri, 1, 0, "OutputModel");
        #if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            qmlRegisterType<QAction>();
        #else
            qmlRegisterAnonymousType<QAction>(uri, 1);
        #endif
        qmlRegisterType<PlacesModel>(uri, 1, 0, "PlacesModel");
        qmlRegisterType<FolderModel>(uri, 1, 0, "FolderModel");
        qmlRegisterType<PathBarModel>(uri, 1, 0, "PathBarModel");
        qmlRegisterType<Positioner>(uri, 1, 0, "Positioner");
        qmlRegisterType<RubberBand>(uri, 1, 0, "RubberBand");
        qmlRegisterType<ItemViewAdapter>(uri, 1, 0, "ItemViewAdapter");
        qmlRegisterType<Fm>(uri, 1, 0, "Fm");
        qmlRegisterType<ShortCut>(uri, 1, 0, "ShortCut");
    }
};

#include "plugin.moc"