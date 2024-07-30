/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorcombotest.h"

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <kcolorcombo.h>

KColorComboTest::KColorComboTest(QWidget *widget)
    : QWidget(widget)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);

    // Standard color list
    QLabel *lbl = new QLabel(QStringLiteral("&Standard colors:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    mStandard = new KColorCombo(this);
    mStandard->setObjectName(QStringLiteral("StandardColors"));
    lbl->setBuddy(mStandard);
    QLabel *lblPreset = new QLabel(QStringLiteral("Preset to green (0,255,0)"), this);

    // add to box layout
    hboxLayout->addWidget(lbl);
    hboxLayout->addWidget(mStandard);
    hboxLayout->addWidget(lblPreset);
    vbox->addLayout(hboxLayout);

    hboxLayout = new QHBoxLayout(this);

    // Custom color list
    lbl = new QLabel(QStringLiteral("&Reds, greens, blues:"), this);
    lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    mCustom = new KColorCombo(this);
    mCustom->setObjectName(QStringLiteral("CustomColors"));
    lbl->setBuddy(mCustom);
    lblPreset = new QLabel(QStringLiteral("Preset to green (0,192,0)"), this);

    // add to box layout
    hboxLayout->addWidget(lbl);
    hboxLayout->addWidget(mCustom);
    hboxLayout->addWidget(lblPreset);
    vbox->addLayout(hboxLayout);

    // Create an exit button
    mExit = new QPushButton(QStringLiteral("E&xit"), this);
    QObject::connect(mExit, &QAbstractButton::clicked, this, &KColorComboTest::quitApp);

    vbox->addWidget(mExit);

    // Populate the custom list
    QList<QColor> standardList;
    standardList << Qt::red << Qt::green << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow << Qt::darkRed << Qt::darkGreen << Qt::darkBlue << Qt::darkCyan
                 << Qt::darkMagenta << Qt::darkYellow << Qt::white << Qt::lightGray << Qt::gray << Qt::darkGray << Qt::black;
    QList<QColor> list;
    list << QColor(255, 0, 0) << QColor(192, 0, 0) << QColor(128, 0, 0) << QColor(64, 0, 0) << QColor(0, 255, 0) << QColor(0, 192, 0) << QColor(0, 128, 0)
         << QColor(0, 64, 0) << QColor(0, 0, 255) << QColor(0, 0, 192) << QColor(0, 0, 128) << QColor(0, 0, 64);
    mCustom->setColors(list);
    if (mCustom->colors() != list) {
        qCritical() << "Custom combo: setColors() != colors()";
    }
    mCustom->setColors(QList<QColor>());
    if (mCustom->colors() != standardList) {
        qCritical() << "Custom combo: setColors(empty) != standard colors";
    }
    mCustom->setColors(list);
    if (mCustom->colors() != list) {
        qCritical() << "Custom combo: setColors() != colors()";
    }

    if (mStandard->colors() != standardList) {
        qCritical() << "Standard combo: colors()";
    }

    QColor col = QColor(1, 2, 3);
    mStandard->setColor(col);
    if (mStandard->color() != col) {
        qCritical() << "Standard combo: set custom color -> " << mStandard->color().red() << "," << mStandard->color().green() << ","
                    << mStandard->color().blue();
    }
    if (!mStandard->isCustomColor()) {
        qCritical() << "Standard combo: custom color: isCustomColor() -> false";
    }
    mStandard->setColor(Qt::green);
    if (mStandard->color() != Qt::green) {
        qCritical() << "Standard combo: color() -> " << mStandard->color().red() << "," << mStandard->color().green() << "," << mStandard->color().blue();
    }
    if (mStandard->isCustomColor()) {
        qCritical() << "Standard combo: standard color: isCustomColor() -> true";
    }

    col = QColor(1, 2, 3);
    mCustom->setColor(col);
    if (mCustom->color() != col) {
        qCritical() << "Custom combo: set custom color -> " << mCustom->color().red() << "," << mCustom->color().green() << "," << mCustom->color().blue();
    }
    if (!mCustom->isCustomColor()) {
        qCritical() << "Custom combo: custom color: isCustomColor() -> false";
    }
    col = QColor(0, 192, 0);
    mCustom->setColor(col);
    if (mCustom->color() != col) {
        qCritical() << "Custom combo: color() -> " << mCustom->color().red() << "," << mCustom->color().green() << "," << mCustom->color().blue();
    }
    if (mCustom->isCustomColor()) {
        qCritical() << "Custom combo: standard color: isCustomColor() -> true";
    }
}

KColorComboTest::~KColorComboTest()
{
}

void KColorComboTest::quitApp()
{
    qApp->closeAllWindows();
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    KColorComboTest *t = new KColorComboTest;
    t->show();
    return a.exec();
}

#include "moc_kcolorcombotest.cpp"
