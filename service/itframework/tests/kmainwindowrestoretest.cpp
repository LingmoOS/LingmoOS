
#include "kmainwindowrestoretest.h"

#include <QApplication>
#include <QLabel>

// clang-format off
#define MAKE_WINDOW( kind, title ) do { \
        MainWin##kind * m = new MainWin##kind; \
        m->setCaption( title ); \
        m->setCentralWidget( new QLabel( title, m ) ); \
        m->show(); \
    } while ( false )
// clang-format on

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("kmainwindowrestoretest"));
    QApplication app(argc, argv);

    if (qApp->isSessionRestored()) {
        kRestoreMainWindows<MainWin1, MainWin2, MainWin3>();
        kRestoreMainWindows<MainWin4, MainWin5>();
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
        RESTORE(MainWin6);
#else
        kRestoreMainWindows<MainWin6>(); // should be equivalent to RESTORE()
#endif
    } else {
        MAKE_WINDOW(1, QStringLiteral("First 1"));
        MAKE_WINDOW(1, QStringLiteral("Second 1"));
        MAKE_WINDOW(2, QStringLiteral("Only 2"));
        MAKE_WINDOW(3, QStringLiteral("First 3"));
        MAKE_WINDOW(4, QStringLiteral("First 4"));
        MAKE_WINDOW(4, QStringLiteral("Second 4"));
        MAKE_WINDOW(3, QStringLiteral("Second 3"));
        MAKE_WINDOW(4, QStringLiteral("Third 4"));
        MAKE_WINDOW(5, QStringLiteral("First 5"));
        MAKE_WINDOW(5, QStringLiteral("Second 5"));
        MAKE_WINDOW(1, QStringLiteral("Only 6"));
    }

    return app.exec();
}
