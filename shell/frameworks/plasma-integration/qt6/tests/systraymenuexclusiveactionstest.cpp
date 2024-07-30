#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSystemTrayIcon trayIcon(QIcon::fromTheme(QStringLiteral("application-exit")));

    QMenu contextMenu;

    QAction *action1 = contextMenu.addAction(QStringLiteral("Exclusive Item 1"));
    QAction *action2 = contextMenu.addAction(QStringLiteral("Exclusive Item 2"));
    action1->setCheckable(true);
    action1->setChecked(true);
    action2->setCheckable(true);

    QActionGroup *actionGroup = new QActionGroup(&contextMenu);
    actionGroup->addAction(action1);
    actionGroup->addAction(action2);

    QAction quitAction(QIcon::fromTheme(QStringLiteral("application-exit")), QStringLiteral("Quit"));
    QObject::connect(&quitAction, &QAction::triggered, &app, &QApplication::quit);
    contextMenu.addAction(&quitAction);

    trayIcon.setContextMenu(&contextMenu);
    trayIcon.show();

    return app.exec();
}
