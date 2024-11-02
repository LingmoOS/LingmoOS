/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPBACKGROUNDMANAGER_H
#define DESKTOPBACKGROUNDMANAGER_H

#include <QObject>
#include <QPixmap>

class QGSettings;
class QVariantAnimation;
class QTimeLine;

class DesktopBackgroundManager : public QObject
{
    friend class DesktopWindow;
    Q_OBJECT
public:
    static DesktopBackgroundManager *globalInstance();

    QPixmap getBackPixmap() const;

    QPixmap getFrontPixmap() const;

    QVariantAnimation *getAnimation() const;

    bool getUsePureColor() const;

    QColor getColor() const;

    bool getPaintBackground() const;

    const QString &getBackgroundOption();

Q_SIGNALS:
    void screensUpdated();

private:
    explicit DesktopBackgroundManager(QObject *parent = nullptr);
    void initGSettings();
    void updateScreens();
    void initBackground();
    void setBackground();
    QString getAccountBackground();
    void setAccountBackground();

protected:
    void switchBackground();

    QPixmap m_backPixmap;
    QPixmap m_frontPixmap;
    QPixmap m_pendingPixmap;

    QGSettings *m_backgroundSettings = nullptr;
    QVariantAnimation *m_animation = nullptr;

    bool m_usePureColor = false;
    QColor m_color = Qt::black;

    bool m_paintBackground = false;

    QTimeLine *m_timeLine = nullptr;

    QString m_current_bg_path;
    QString m_backgroundOption;

    bool m_shouldSyncAccountBackground = true;
};

#endif // DESKTOPBACKGROUNDMANAGER_H
