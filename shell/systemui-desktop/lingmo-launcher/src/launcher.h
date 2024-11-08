/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QGuiApplication>
#include <QQuickView>
#include <QTimer>

#include <QDBusInterface>

class Launcher : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QRect screenRect READ screenRect NOTIFY screenRectChanged)
    Q_PROPERTY(bool showed READ showed NOTIFY showedChanged)
    Q_PROPERTY(int leftMargin READ leftMargin NOTIFY marginsChanged)
    Q_PROPERTY(int rightMargin READ rightMargin NOTIFY marginsChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin NOTIFY marginsChanged)

public:
    Launcher(bool firstShow = false, QQuickView *w = nullptr);

    int leftMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

    bool showed();

    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    Q_INVOKABLE void toggle();

    Q_INVOKABLE bool dockAvailable();
    Q_INVOKABLE bool isPinedDock(const QString &desktop);

    Q_INVOKABLE void clearPixmapCache();

    QRect screenRect();

signals:
    void screenRectChanged();
    void showedChanged();
    void marginsChanged();

private slots:
    void updateMargins();
    void updateSize();
    void onGeometryChanged();

protected:
    void showEvent(QShowEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void onActiveChanged();

private:
    QDBusInterface m_dockInterface;
    QRect m_screenRect;
    QTimer *m_hideTimer;
    bool m_showed;

    int m_leftMargin;
    int m_rightMargin;
    int m_bottomMargin;
};

#endif // LAUNCHER_H
