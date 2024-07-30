/*
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdatecombobox.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDate>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app{argc, argv};

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({QStringLiteral("min-date"), QStringLiteral("Minimum date in ISO 8601 format, e.g. 2020-02-02"), QStringLiteral("min-date")});
    parser.addOption({QStringLiteral("max-date"), QStringLiteral("Maximum date in ISO 8601 format, e.g. 2121-12-12"), QStringLiteral("max-date")});
    parser.process(app.arguments());

    const auto minDate = QDate::fromString(parser.value(QStringLiteral("min-date")), Qt::ISODate);
    const auto maxDate = QDate::fromString(parser.value(QStringLiteral("max-date")), Qt::ISODate);

    KDateComboBox dateComboBox;
    dateComboBox.setOptions(KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords
                            | KDateComboBox::WarnOnInvalid);
    dateComboBox.setMinimumDate(minDate);
    dateComboBox.setMaximumDate(maxDate);
    QObject::connect(&dateComboBox, &KDateComboBox::dateEntered, [](const QDate &d) {
        qDebug() << "dateEntered" << d;
    });
    QObject::connect(&dateComboBox, &KDateComboBox::dateChanged, [](const QDate &d) {
        qDebug() << "dateChanged" << d;
    });
    QObject::connect(&dateComboBox, &KDateComboBox::dateEdited, [](const QDate &d) {
        qDebug() << "dateEdited" << d;
    });
    dateComboBox.resize(200, dateComboBox.sizeHint().height());
    dateComboBox.show();
    // dateComboBox.setEnabled(false);
    return app.exec();
}
