/*

    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/
#ifndef QDBUSMENUBARWRAPPER_H
#define QDBUSMENUBARWRAPPER_H

#include <QString>
#include <qpa/qplatformmenu.h>

#include <memory>

class QDBusMenuBarWrapper : public QPlatformMenuBar
{
    Q_OBJECT
public:
    QDBusMenuBarWrapper(QPlatformMenuBar *wrap)
        : wrap{wrap}
    {
    }
    ~QDBusMenuBarWrapper()
    {
    }
    QPlatformMenu *createMenu() const override
    {
        return wrap->createMenu();
    }
    void insertMenu(QPlatformMenu *menu, QPlatformMenu *before) override
    {
        wrap->insertMenu(menu, before);
    }
    void removeMenu(QPlatformMenu *menu) override
    {
        wrap->removeMenu(menu);
    }
    void syncMenu(QPlatformMenu *menuItem) override
    {
        wrap->syncMenu(menuItem);
    }
    QPlatformMenu *menuForTag(quintptr tag) const override
    {
        return wrap->menuForTag(tag);
    }
    void handleReparent(QWindow *newParentWindow) override
    {
        wrap->handleReparent(newParentWindow);
        // matching QDBusMenuBar, it increments the id everytime it gets a new valid parent
        if (newParentWindow) {
            ++menuBarId;
        }
        window = newParentWindow;
        Q_EMIT windowChanged(newParentWindow, window);
    }
    QString objectPath()
    {
        return QStringLiteral("/MenuBar/%1").arg(menuBarId);
    }
    Q_SIGNAL void windowChanged(QWindow *newWindow, QWindow *oldWindow);
    QWindow *window = nullptr;

private:
    static inline uint menuBarId = 0;
    std::unique_ptr<QPlatformMenuBar> wrap;
};

#endif
