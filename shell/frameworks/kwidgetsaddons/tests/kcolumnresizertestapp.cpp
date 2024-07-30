/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2011-2014 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kcolumnresizertestapp.h>

#include <QApplication>
#include <kcolumnresizer.h>

#include <ui_kcolumnresizertestapp.h>

Window::Window()
{
    m_ui = new Ui_ColumnResizerTestWidget;
    m_ui->setupUi(this);

    KColumnResizer *resizer = new KColumnResizer(this);
    resizer->addWidgetsFromLayout(m_ui->gridGroupBox1->layout(), 0);
    resizer->addWidgetsFromLayout(m_ui->formGroupBox1->layout(), 0);
    resizer->addWidgetsFromLayout(m_ui->formGroupBox2->layout(), 0);
    resizer->addWidgetsFromLayout(m_ui->gridGroupBox2->layout(), 0);

    connect(m_ui->spinBox, &QSpinBox::valueChanged, this, &Window::updateAdjustableLabel);
    updateAdjustableLabel();
}

Window::~Window()
{
    delete m_ui;
}

void Window::updateAdjustableLabel()
{
    QString txt;
    txt.fill(QLatin1Char('#'), m_ui->spinBox->value());
    m_ui->adjustableLabel->setText(QStringLiteral("Spin me %1:").arg(txt));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Window window;
    window.show();
    return app.exec();
}

#include "moc_kcolumnresizertestapp.cpp"
