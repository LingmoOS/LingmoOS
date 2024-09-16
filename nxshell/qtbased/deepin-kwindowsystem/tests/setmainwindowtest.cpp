/* This file is part of the KDE libraries

    Copyright 2013 Aurélien Gâteau <agateau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <kwindowsystem.h>

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

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
    while (KWindowSystem::activeWindow() == us) {
        QApplication::processEvents();
    }

    // Get the id of the selected window
    WId id = KWindowSystem::activeWindow();
    m_label->setText(QString("Showing dialog on window with id: %1.").arg(id));

    // Create test dialog
    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    QHBoxLayout *layout = new QHBoxLayout(dialog);
    layout->addWidget(new QLabel("Test Dialog.\nYou should not be able to bring the parent window on top of me."));

    // Show it
    KWindowSystem::setMainWindow(dialog, id);
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
