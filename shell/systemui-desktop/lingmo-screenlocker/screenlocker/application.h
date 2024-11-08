/*
 * Copyright (C) 2021 CuteOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>
#include <QQuickView>

#include <QVariantAnimation>
#include "authenticator.h"

class Application : public QGuiApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    ~Application();

    void initialViewSetup();

public slots:
    void desktopResized();
    void onScreenAdded(QScreen *screen);

private slots:
    void onSucceeded();
    void getFocus();
    void markViewsAsVisible(QQuickView *view);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QWindow *getActiveScreen();
    void shareEvent(QEvent *e, QQuickView *from);
    void screenGeometryChanged(QScreen *screen, const QRect &geo);

private:
    Authenticator *m_authenticator;
    QList<QQuickView *> m_views;

    bool m_testing = false;
};

#endif // APPLICATION_H
