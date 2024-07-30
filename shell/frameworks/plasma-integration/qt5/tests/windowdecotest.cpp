/* This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

/*
 * This class is for checking that wayland server side window decorations remain
 * after a window is hidden and shown.
 */

#include <KColorSchemeManager>
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class ATestWindow : public QWidget
{
    Q_OBJECT
public:
    ATestWindow();

private:
    QPushButton *mBtn = nullptr;
    QWidget *m_area = nullptr;
};

ATestWindow::ATestWindow()
{
    mBtn = new QPushButton(QStringLiteral("Hide and Show"));

    m_area = new QWidget;
    m_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mBtn, &QPushButton::clicked, this, [this]() {
        this->hide();
        QTimer::singleShot(1000, this, [this]() {
            this->show();
        });
    });

    QComboBox *colorCombo = new QComboBox();
    KColorSchemeManager *schemes = new KColorSchemeManager(this);
    colorCombo->setModel(schemes->model());

    connect(colorCombo, QOverload<int>::of(&QComboBox::activated), schemes, [=](int row) {
        schemes->activateScheme(colorCombo->model()->index(row, 0));
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mBtn);
    layout->addWidget(colorCombo);
    setLayout(layout);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ATestWindow wnd;
    wnd.show();

    return app.exec();
}

#include "windowdecotest.moc"
