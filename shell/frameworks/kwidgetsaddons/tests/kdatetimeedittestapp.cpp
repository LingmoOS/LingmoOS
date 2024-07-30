#include "kdatetimeedit.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app(argc, argv);
    KDateTimeEdit dateTimeEdit;
    dateTimeEdit.setOptions(dateTimeEdit.options() | KDateTimeEdit::ShowCalendar | KDateTimeEdit::SelectCalendar | KDateTimeEdit::ShowTimeZone
                            | KDateTimeEdit::SelectTimeZone);
    QObject::connect(&dateTimeEdit, &KDateTimeEdit::dateTimeEntered, [](const QDateTime &dt) {
        qDebug() << "dateTimeEntered" << dt;
    });
    QObject::connect(&dateTimeEdit, &KDateTimeEdit::dateTimeChanged, [](const QDateTime &dt) {
        qDebug() << "dateTimeChanged" << dt;
    });
    QObject::connect(&dateTimeEdit, &KDateTimeEdit::dateTimeEdited, [](const QDateTime &dt) {
        qDebug() << "dateTimeEdited" << dt;
    });
    dateTimeEdit.show();
    // dateTimeEdit.setEnabled(false);
    return app.exec();
}
