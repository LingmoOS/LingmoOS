/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalization>

#include <QApplication>
#include <QDoubleSpinBox>
#include <QSpinBox>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSpinBox spinBox;
    KLocalization::setupSpinBoxFormatString(&spinBox, ki18np("Baking %v pizza", "Baking %v pizzas"));
    KLocalization::setupSpinBoxFormatString(&spinBox, ki18np("Baking %v cake", "Baking %v cakes"));
    KLocalization::retranslateSpinBoxFormatString(&spinBox);
    spinBox.show();

    QSpinBox doubleSpinBox;
    KLocalization::setupSpinBoxFormatString(&doubleSpinBox, ki18n("%v%"));
    KLocalization::retranslateSpinBoxFormatString(&doubleSpinBox);
    doubleSpinBox.show();

    return app.exec();
}
