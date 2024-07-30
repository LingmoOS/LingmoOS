/*
  SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KDatePickerPopup>

#include <QApplication>
#include <QDebug>
#include <QToolButton>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QToolButton button;
    button.setIcon(QIcon::fromTheme(QStringLiteral("view-calendar")));
    KDatePickerPopup popup(KDatePickerPopup::DatePicker | KDatePickerPopup::Words | KDatePickerPopup::NoDate);
    QObject::connect(&popup, &KDatePickerPopup::dateChanged, &app, [](QDate date) {
        qInfo() << date;
    });
    button.setMenu(&popup);
    button.show();

    return app.exec();
}
