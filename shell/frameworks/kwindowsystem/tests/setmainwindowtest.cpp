/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kwindowsystem.h>

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "kx11extras.h"

class Window : public QWidget
{
public:
    Window();

private:
    void showWindow();
    QLabel *m_label;
};

Window::Window()
{
    QPushButton *button = new QPushButton("Start Test");
    connect(button, &QPushButton::clicked, this, &Window::showWindow);

    m_label = new QLabel;
    m_label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(button);
    layout->addWidget(m_label);

    setMinimumSize(200, 150);
}

void Window::showWindow()
{
    // Wait for user to select another window
    m_label->setText("Click on another window to show a dialog on it");
    WId us = winId();
    while (KX11Extras::activeWindow() == us) {
        QApplication::processEvents();
    }

    // Get the id of the selected window
    WId id = KX11Extras::activeWindow();
    m_label->setText(QString("Showing dialog on window with id: %1.").arg(id));

    // Create test dialog
    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    QHBoxLayout *layout = new QHBoxLayout(dialog);
    layout->addWidget(new QLabel("Test Dialog.\nYou should not be able to bring the parent window on top of me."));

    // Show it
    dialog->setAttribute(Qt::WA_NativeWindow, true);
    KWindowSystem::setMainWindow(dialog->windowHandle(), id);
    dialog->exec();

    m_label->setText(QString());
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Window window;
    window.show();
    return app.exec();
}
