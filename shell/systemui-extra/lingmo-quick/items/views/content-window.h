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

#ifndef LINGMO_QUICK_ITEMS_CONTENT_WINDOW_H
#define LINGMO_QUICK_ITEMS_CONTENT_WINDOW_H

#include "lingmo-style-window.h"
#include "margin.h"
#include "types.h"

#include "windows/lingmo-window.h"

class QScreen;

namespace LingmoUIQuick {

/**
 * @class ContentWindow
 *
 * 是基于屏幕可用区域进行定位的，需要设置窗口需要显示在屏幕可用区域的哪个位置
 * 窗口的尺寸由显示的内容决定
 * 位置(position)配合边距(margin)一起使用。
    ContentWindow {
        id: dialog
        // 设置显示在哪个屏幕上
        screen: QScreen*
        // 在屏幕上的位置，目前支持：TopLeft，TopRight，BottomLeft，BottomRight
        position: Types.BottomLeft
        // 到屏幕边缘边距
        margin {
            left: 24
            top: 24
            right: 24
            bottom: 24
        }

        // 需要显示的内容，必须定义宽高，窗口尺寸与内容尺寸一致
        Item {
            width: 500
            height: 500

            Text {
                anchors.fill: parent
                anchors.margins: 10
                text: "test dialog"
            }
        }
    }
 *
 */
class ContentWindow : public LINGMOWindow
{
    Q_OBJECT
    Q_PROPERTY(LingmoUIQuick::Margin* margin READ margin CONSTANT)
    Q_PROPERTY(QQuickItem* content READ content WRITE setContent NOTIFY contentChanged FINAL)
    Q_PROPERTY(QScreen* screen READ getScreen WRITE setWindowScreen NOTIFY screenChanged FINAL)
    Q_PROPERTY(LingmoUIQuick::Types::Pos position READ getPos WRITE setPos NOTIFY positionChanged FINAL)
    Q_PROPERTY(bool enableBlurEffect READ enableBlurEffect WRITE setEnableBlurEffect NOTIFY enableBlurEffectChanged)
    Q_PROPERTY(bool useAvailableGeometry READ useAvailableGeometry WRITE setUseAvailableGeometry NOTIFY useAvailableGeometryChanged)
    Q_CLASSINFO("DefaultProperty", "content")
public:
    explicit ContentWindow(QWindow *parent = nullptr);

    QScreen* getScreen() const;
    void setWindowScreen(QScreen* screen);

    bool enableBlurEffect() const;
    void setEnableBlurEffect(bool enable);

    QQuickItem* content() const;
    void setContent(QQuickItem* content);

    // 在屏幕上的位置，目前支持：TopLeft，TopRight，BottomLeft，BottomRight
    Types::Pos getPos() const;
    void setPos(Types::Pos pos);

    Margin* margin() const;
    // 设置窗口是否使用屏幕可用区域进行定位，默认为true
    bool useAvailableGeometry() const;
    void setUseAvailableGeometry(bool use);

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void onContentWidthChanged();
    void onContentHeightChanged();
    void updateLocation();

private:
    bool m_enableBlurEffect {false};
    QQuickItem *m_content {nullptr};
    LingmoUIQuick::Margin *m_margin {nullptr};
    Types::Pos m_pos {Types::NoPosition};
    bool m_useAvailableGeometry {true};

Q_SIGNALS:
    void screenChanged();
    void contentChanged();
    void positionChanged();
    void enableBlurEffectChanged();
    void useAvailableGeometryChanged();
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_CONTENT_WINDOW_H
