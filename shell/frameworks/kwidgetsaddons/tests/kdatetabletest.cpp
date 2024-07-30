#include <QApplication>

#include "kdatetable_p.h"
#include <QDate>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app(argc, argv);

    KDateTable widget;
    widget.setCustomDatePainting(QDate::currentDate().addDays(-3), QColor("green"), KDateTable::CircleMode, QColor("yellow"));
    widget.show();

    return app.exec();
}
