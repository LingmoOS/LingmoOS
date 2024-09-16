#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QScreen>
#include <QApplication>

using namespace data_transfer_core;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags /*flags*/)
    : CrossMainWindow(parent), d(new MainWindowPrivate(this))
{
    d->initWindow();
    d->initWidgets();
    d->moveCenter();
}

MainWindow::~MainWindow() { }

#if defined(_WIN32) || defined(_WIN64)
void MainWindow::paintEvent(QPaintEvent *event)
{
    d->paintEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    QApplication::quit();
}
#endif

MainWindowPrivate::MainWindowPrivate(MainWindow *qq) : q(qq) { }

MainWindowPrivate::~MainWindowPrivate() { }

void MainWindowPrivate::moveCenter()
{
    QScreen *cursorScreen = nullptr;
    const QPoint &cursorPos = QCursor::pos();

    QList<QScreen *> screens = qApp->screens();
    QList<QScreen *>::const_iterator it = screens.begin();
    for (; it != screens.end(); ++it) {
        if ((*it)->geometry().contains(cursorPos)) {
            cursorScreen = *it;
            break;
        }
    }

    if (!cursorScreen)
        cursorScreen = qApp->primaryScreen();
    if (!cursorScreen)
        return;

    int x = (cursorScreen->availableGeometry().width() - q->width()) / 2;
    int y = (cursorScreen->availableGeometry().height() - q->height()) / 2;
    q->move(QPoint(x, y) + cursorScreen->geometry().topLeft());
}

