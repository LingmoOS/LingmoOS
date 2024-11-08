/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_LINGMO_STYLE_WINDOW_H
#define LINGMO_QUICK_LINGMO_STYLE_WINDOW_H

#include <QRect>
#include <QRegion>
#include <QQuickWindow>

#include "window-helper.h"

namespace LingmoUIQuick {

class LingmoUIStyleWindowPrivate;

class LingmoUIStyleWindow : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool enableWindowBlur READ enableWindowBlur WRITE setEnableWindowBlur NOTIFY enableWindowBlurChanged)
    Q_PROPERTY(QRegion blurRegion READ blurRegion WRITE setBlurRegion NOTIFY blurRegionChanged)

    // reimpl
    Q_PROPERTY(int x READ x WRITE setX NOTIFY windowXChanged FINAL)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY windowYChanged FINAL)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY windowWidthChanged FINAL)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY windowHeightChanged FINAL)

    Q_PROPERTY(QRect windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)
    Q_PROPERTY(QString graphBackend READ graphBackend NOTIFY graphBackendChanged)
    Q_PROPERTY(LingmoUIQuick::WindowType::Type windowType READ windowType WRITE setWindowType NOTIFY windowTypeChanged)
public:
    explicit LingmoUIStyleWindow(QWindow *parent = nullptr);
    ~LingmoUIStyleWindow() override;

    Q_INVOKABLE quint64 getWinId();

    bool enableWindowBlur() const;
    void setEnableWindowBlur(bool enable);

    QRegion blurRegion() const;
    void setBlurRegion(const QRegion& region);

    WindowType::Type windowType() const;

    virtual void setWindowType(WindowType::Type windowType);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    QRect windowGeometry() const;
    void setWindowGeometry(const QRect &rect);

    QString graphBackend() const;

Q_SIGNALS:
    void enableWindowBlurChanged();
    void blurRegionChanged();

    // geometry
    void windowXChanged();
    void windowYChanged();
    void windowWidthChanged();
    void windowHeightChanged();
    void windowGeometryChanged();
    void graphBackendChanged();
    void windowTypeChanged();

protected:
    bool event(QEvent *event) override;
    void updateBlurRegion();
    void updateGeometry();

private:
    LingmoUIStyleWindowPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_LINGMO_STYLE_WINDOW_H
