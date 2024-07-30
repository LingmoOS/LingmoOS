#include <QMainWindow>

class Window : public QMainWindow
{
    Q_OBJECT
public:
    Window();
};

#include <QDir>

#include <KPluginFactory>
#include <QApplication>
#include <kde_terminal_interface.h>
#include <kparts/readonlypart.h>

Window::Window()
{
    this->resize(800, 600);

    auto part = KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(KPluginMetaData(QStringLiteral("konsolepart")), this).plugin;
    Q_ASSERT(part);

    setCentralWidget(part->widget());

    TerminalInterface *interface = qobject_cast<TerminalInterface *>(part);
    interface->showShellInDir(QDir::home().path());

    connect(part, &QObject::destroyed, this, &QObject::deleteLater);
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("tetest"));
    QApplication app(argc, argv);
    Window *window = new Window();
    window->show();
    return app.exec();
}

#include "terminal_test.moc"
