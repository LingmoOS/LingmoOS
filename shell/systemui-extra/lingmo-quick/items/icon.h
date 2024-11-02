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

#ifndef LINGMO_QUICK_ITEMS_ICON_H
#define LINGMO_QUICK_ITEMS_ICON_H

#include <QObject>
#include <QQuickItem>
#include <QImage>
#include <QRect>

#include "types.h"
#include "lingmo/lingmo-theme-proxy.h"

class QSGTexture;
class QSGImageNode;

namespace LingmoUIQuick {

class IconPrivate;

/**
 * @class Icon "icon/Icon"
 *
 * Usage:
 *
 * Icon {
 *  mode: Icon.Highlight
 *  source: "xxx"
 * }
 *
 */
class Icon : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(LingmoUIQuick::Icon::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QString pointText READ pointText WRITE setPointText NOTIFY pointTextChanged)
    Q_PROPERTY(LingmoUIQuick::Types::Pos pointPos READ pointPos WRITE setPointPos NOTIFY pointPosChanged)
    Q_PROPERTY(LingmoUIQuick::Icon::PointType pointType READ pointType WRITE setPointType NOTIFY pointTypeChanged)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorRole pointColor READ pointColor WRITE setPointColor NOTIFY pointColorChanged)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorRole highlightColor READ highlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)
public:
    enum Mod {
        Normal = 0x01,
        Highlight = 0x02,
        AutoHighlight = 0x04, /* 高亮 */
        ForceHighlight = Highlight | 0x08, /* 高亮 */
        Disabled = 0x80 /* 灰色 */
    };
    Q_DECLARE_FLAGS(Mode, Mod)
    Q_FLAG(Mode)

    enum PointType {
        Null,
        Point,
        Text
    };
    Q_ENUM(PointType)

    explicit Icon(QQuickItem *parent = nullptr);
    ~Icon() override;

    Icon::Mode mode() const;
    void setMode(const Icon::Mode &mode);

    Icon::PointType pointType() const;
    void setPointType(Icon::PointType type);

    Types::Pos pointPos() const;
    void setPointPos(Types::Pos pos);

    Theme::ColorRole pointColor() const;
    void setPointColor(const Theme::ColorRole &color);

    QString pointText() const;
    void setPointText(const QString &text);

    Theme::ColorRole highlightColor() const;
    void setHighlightColor(const Theme::ColorRole &color);

    int radius() const;
    void setRadius(int radius);

    QVariant source() const;
    void setSource(const QVariant &source);

    // 在该函数中修改图像的细节
    virtual void imageCorrection(QImage &image);
    virtual QSGTexture *createTexture();
    virtual void drawPoint(QImage &image);
    QPoint posPoint(const QSize &imageSize, const QSize &pointSize) const;

    inline void markTextureChanged();

Q_SIGNALS:
    void modeChanged();
    void sourceChanged();
    void radiusChanged();
    void highlightColorChanged();
    void pointTypeChanged();
    void pointTextChanged();
    void pointPosChanged();
    void pointColorChanged();

private Q_SLOTS:
    void onSizeChanged();
    void updateMode();

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

private:
    IconPrivate *d {nullptr};
};

} // LingmoUIQuick

Q_DECLARE_METATYPE(LingmoUIQuick::Icon::PointType)
Q_DECLARE_OPERATORS_FOR_FLAGS(LingmoUIQuick::Icon::Mode)

#endif //LINGMO_QUICK_ITEMS_ICON_H
