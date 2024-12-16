// Copyright (C) 2024 Groveer <guoyao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwayland-treeland-personalization-manager-v1.h"

#include <private/qwaylandwindow_p.h>

#include <qwindow.h>

#include <QApplication>
#include <QPushButton>
#include <QtWaylandClient/QWaylandClientExtension>

class PersonalizationManager : public QWaylandClientExtensionTemplate<PersonalizationManager>,
                               public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    explicit PersonalizationManager();
};

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(1)
{
}

class PersonalizationWindow : public QWaylandClientExtensionTemplate<PersonalizationWindow>,
                              public QtWayland::personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindow(struct ::personalization_window_context_v1 *object);
};

PersonalizationWindow::PersonalizationWindow(struct ::personalization_window_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWindow>(1)
    , QtWayland::personalization_window_context_v1(object)
{
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PersonalizationManager manager;

    QObject::connect(&manager, &PersonalizationManager::activeChanged, &manager, [&manager] {
        qDebug() << "personalzation manager" << manager.isActive();

        if (manager.isActive()) {
            QWidget *widget = new QWidget;
            widget->setAttribute(Qt::WA_TranslucentBackground);
            // widget->setWindowFlags(Qt::FramelessWindowHint); // 可选，去除窗口边框
            widget->resize(640, 480);

            QPushButton *button = new QPushButton("Click Me", widget);
            button->setGeometry(0, 0, 100, 50); // 设置按钮的位置和大小

            widget->show();

            QWindow *window = widget->windowHandle();

            if (window && window->handle()) {
                QtWaylandClient::QWaylandWindow *waylandWindow =
                    static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());

                struct wl_surface *surface = waylandWindow->wlSurface();
                if (surface) {
                    PersonalizationWindow *context =
                        new PersonalizationWindow(manager.get_window_context(surface));

                    QObject::connect(button, &QPushButton::clicked, [context]() {
                        static int state = 0;
                        if (state > 2) {
                            state = 0;
                        }
                        context->set_background_type(state);
                        qDebug() << "===========background state: ==========" << state;
                        state++;
                    });
                }
            }
        }
    });

    return app.exec();
}

#include "main.moc"
