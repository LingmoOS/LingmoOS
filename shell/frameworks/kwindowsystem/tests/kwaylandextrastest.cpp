/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kwindowsystem.h>

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <kwaylandextras.h>

class Window : public QWidget
{
public:
    Window();

private:
    void updateSerial();
    void requestToken();

    void exportWindow();
    void unexportWindow();
    void setExportedHandle(const QString &handle);

    QLabel *m_serialLabel;
    QLabel *m_tokenLabel;
    QLabel *m_exportedLabel;
};

Window::Window()
{
    QPushButton *serialButton = new QPushButton("Update serial");
    connect(serialButton, &QPushButton::clicked, this, &Window::updateSerial);

    QPushButton *tokenButton = new QPushButton("Request token");
    connect(tokenButton, &QPushButton::clicked, this, &Window::requestToken);

    m_serialLabel = new QLabel;
    m_serialLabel->setText("Last input serial: " + QString::number(KWaylandExtras::self()->lastInputSerial(windowHandle())));

    m_tokenLabel = new QLabel;
    m_tokenLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_tokenLabel->setText("XDG actvation token:");

    QHBoxLayout *exportLayout = new QHBoxLayout;

    QPushButton *exportButton = new QPushButton("Export window");
    connect(exportButton, &QPushButton::clicked, this, &Window::exportWindow);

    QPushButton *unexportButton = new QPushButton("Unexport window");
    connect(unexportButton, &QPushButton::clicked, this, &Window::unexportWindow);

    m_exportedLabel = new QLabel;
    m_exportedLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    setExportedHandle(QString());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(serialButton);
    layout->addWidget(m_serialLabel);
    layout->addWidget(tokenButton);
    layout->addWidget(m_tokenLabel);

    exportLayout->addWidget(exportButton);
    exportLayout->addWidget(unexportButton);
    layout->addLayout(exportLayout);
    layout->addWidget(m_exportedLabel);
}

void Window::updateSerial()
{
    m_serialLabel->setText("Last input serial: " + QString::number(KWaylandExtras::self()->lastInputSerial(windowHandle())));
}

void Window::requestToken()
{
    connect(
        KWaylandExtras::self(),
        &KWaylandExtras::xdgActivationTokenArrived,
        this,
        [this](int /*serial*/, const QString &token) {
            m_tokenLabel->setText("XDG actvation token: " + token);
        },
        Qt::SingleShotConnection);

    KWaylandExtras::requestXdgActivationToken(windowHandle(), KWaylandExtras::self()->lastInputSerial(windowHandle()), QString());
}

void Window::exportWindow()
{
    connect(
        KWaylandExtras::self(),
        &KWaylandExtras::windowExported,
        this,
        [this](QWindow *window, const QString &handle) {
            Q_UNUSED(window);
            setExportedHandle(handle);
        },
        Qt::SingleShotConnection);
    KWaylandExtras::exportWindow(windowHandle());
}

void Window::unexportWindow()
{
    KWaylandExtras::unexportWindow(windowHandle());
    setExportedHandle(QString());
}

void Window::setExportedHandle(const QString &handle)
{
    m_exportedLabel->setText("XDG foreign handle: " + handle);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Window window;
    window.show();
    return app.exec();
}
