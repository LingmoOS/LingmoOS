#include <QApplication>

#include "kcharselect.h"

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kcharselecttest"));

    QApplication app(argc, argv);

    KCharSelect selector(nullptr, nullptr);
    selector.setAllPlanesEnabled(true);
    selector.resize(selector.sizeHint());
    selector.show();
    selector.setWindowTitle(QStringLiteral("KCharSelect Test"));

    return app.exec();
}
