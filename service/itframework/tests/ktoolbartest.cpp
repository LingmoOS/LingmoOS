#include <QAction>
#include <QApplication>
#include <QMainWindow>

#include <kactioncollection.h>
#include <ktoolbar.h>

// This is a test for "Automatically hide extra toolbar separators"
// If several separators are next to each other, only one should show up.

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kactiontest"));
    QApplication app(argc, argv);

    KActionCollection coll(static_cast<QObject *>(nullptr));

    QAction *action1 = coll.addAction(QStringLiteral("test1"));
    action1->setText(QStringLiteral("test1"));
    QAction *action2 = coll.addAction(QStringLiteral("test2"));
    action2->setText(QStringLiteral("test2"));
    QAction *action3 = coll.addAction(QStringLiteral("test3"));
    action3->setText(QStringLiteral("test3"));
    QAction *action4 = coll.addAction(QStringLiteral("test4"));
    action4->setText(QStringLiteral("test4"));
    QAction *action5 = coll.addAction(QStringLiteral("test5"));
    action5->setText(QStringLiteral("test5"));
    QAction *action6 = coll.addAction(QStringLiteral("test6"));
    action6->setText(QStringLiteral("test6"));
    QAction *action7 = coll.addAction(QStringLiteral("test7"));
    action7->setText(QStringLiteral("test7"));

    QMainWindow *mw = new QMainWindow();
    KToolBar *tb = new KToolBar(mw);
    mw->addToolBar(tb);

    action2->setSeparator(true);
    action3->setSeparator(true);
    action7->setSeparator(true);

    coll.addAssociatedWidget(tb);

    mw->show();

    app.exec();

    mw->show();

    action2->setVisible(false);

    app.exec();

    mw->show();

    action1->setVisible(false);

    return app.exec();
}
