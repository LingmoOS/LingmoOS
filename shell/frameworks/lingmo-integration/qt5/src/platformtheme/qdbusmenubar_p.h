/*

    SPDX-FileCopyrightText: 2016 Dmitry Shachnev <mitya57@gmail.com>
    Contact: https://www.qt.io/licensing/

    This file is part of the QtGui module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial

*/

#ifndef QDBUSMENUBAR_P_H
#define QDBUSMENUBAR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "kdeplatformtheme.h"

#include <QHash>
#include <QString>
#include <QWindow>

#include <QtThemeSupport/private/qdbusmenuadaptor_p.h>
#include <QtThemeSupport/private/qdbusmenuconnection_p.h>
#include <QtThemeSupport/private/qdbusmenuregistrarproxy_p.h>
#include <QtThemeSupport/private/qdbusplatformmenu_p.h>

QT_BEGIN_NAMESPACE

class QDBusMenuBar : public QPlatformMenuBar
{
    Q_OBJECT

public:
    QDBusMenuBar(KdePlatformTheme *platformTheme);
    ~QDBusMenuBar() override;

    void insertMenu(QPlatformMenu *menu, QPlatformMenu *before) override;
    void removeMenu(QPlatformMenu *menu) override;
    void syncMenu(QPlatformMenu *menu) override;
    void handleReparent(QWindow *newParentWindow) override;
    QPlatformMenu *menuForTag(quintptr tag) const override;
    QPlatformMenu *createMenu() const override;
    static QDBusMenuBar *globalMenuBar();
    static QDBusMenuBar *menuBarForWindow(QWindow *window);

    QWindow *window() const
    {
        return m_window;
    }
    QString objectPath() const
    {
        return m_objectPath;
    }

Q_SIGNALS:
    void windowChanged(QWindow *newWindow, QWindow *oldWindow);

private:
    QDBusPlatformMenu *m_menu = nullptr;
    QDBusMenuAdaptor *m_menuAdaptor = nullptr;
    QHash<quintptr, QDBusPlatformMenuItem *> m_menuItems;
    QPointer<QWindow> m_window;
    QString m_objectPath;
    bool m_initted = false;
    KdePlatformTheme *m_platformTheme;
    static QDBusMenuBar *s_globalMenuBar;
    static QMap<QWindow *, QDBusMenuBar *> s_menuBars;

    QDBusPlatformMenuItem *menuItemForMenu(QPlatformMenu *menu);
    static void updateMenuItem(QDBusPlatformMenuItem *item, QPlatformMenu *menu);
    bool createDBusMenuBar();
    void uncreateDBusMenuBar();

    static void registerMenuBarX11(QWindow *window, const QString &objectPath);
    static void unregisterMenuBarX11(QWindow *window);
};

QT_END_NAMESPACE

#endif // QDBUSMENUBAR_P_H
