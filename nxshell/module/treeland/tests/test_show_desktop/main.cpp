// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwayland-treeland-window-management-v1.h"

#include <QApplication>
#include <QtWaylandClient/QWaylandClientExtension>

class WindowManager : public QWaylandClientExtensionTemplate<WindowManager>,
                              public QtWayland::window_management_v1
{
    Q_OBJECT
public:
    explicit WindowManager();

    void treeland_window_management_v1_show_desktop(uint32_t state)
    {
        qInfo() << "-------Show Desktop State----- " << state;
    }
};

WindowManager::WindowManager()
    : QWaylandClientExtensionTemplate<WindowManager>(1)
{
}

//显示桌面: ./test-show-desktop 1

//预览桌面: ./test-show-desktop 2

//恢复显示: ./test-show-desktop 0

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QApplication app(argc, argv);
    WindowManager manager;

    QObject::connect(&manager, &WindowManager::activeChanged, &manager, [&manager, argc, argv] {
        if (manager.isActive()) {
            if (argc == 2) {
                switch (std::stoi(argv[1])) {
                case manager.desktop_state_normal:
                    manager.set_desktop(manager.desktop_state_normal);
                    break;
                case manager.desktop_state_show:
                    manager.set_desktop(manager.desktop_state_show);
                    break;
                case manager.desktop_state_preview_show:
                    manager.set_desktop(manager.desktop_state_preview_show);
                    break;
                default:
                    break;
                }
            }
        }
    });

    return app.exec();
}

#include "main.moc"
