#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "draganddrop/declarativedragdropevent.h"
#include "draganddrop/declarativedroparea.h"
#include "draganddrop/declarativemimedata.h"

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override {
    #if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        qmlRegisterType<QMimeData>();
    #else
        qmlRegisterAnonymousType<QMimeData>(uri, 1);
    #endif
    qmlRegisterType<DeclarativeDropArea>(uri, 1, 0, "DropArea");
    qmlRegisterUncreatableType<DeclarativeMimeData>(uri, 1, 0, "MimeData", QStringLiteral("MimeData cannot be created from QML."));
    qmlRegisterUncreatableType<DeclarativeDragDropEvent>(uri, 2, 0, "DragDropEvent", QStringLiteral("DragDropEvent cannot be created from QML."));
    }
};

#include "plugin.moc"