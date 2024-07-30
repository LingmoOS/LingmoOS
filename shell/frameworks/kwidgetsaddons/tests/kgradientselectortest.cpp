/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 David Edmundson  <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KGradientSelector>
#include <QApplication>

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class KSelectorTestWidget : public QWidget
{
public:
    KSelectorTestWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        auto label = new QLabel(QStringLiteral("Gradient box"));
        mainLayout->addWidget(label);

        auto selector = new KGradientSelector(Qt::Horizontal, this);
        selector->setFixedSize(256, 26);
        selector->setIndent(true);
        selector->setArrowDirection(Qt::DownArrow);
        selector->setRange(0, 255);
        mainLayout->addWidget(selector);
    }
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("ktitlewidgettest"));
    QApplication app(argc, argv);

    KSelectorTestWidget *mainWidget = new KSelectorTestWidget;
    mainWidget->show();

    return app.exec();
}
