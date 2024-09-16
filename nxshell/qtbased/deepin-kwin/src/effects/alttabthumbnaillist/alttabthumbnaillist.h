// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <kwineffects.h>
#include <kwineffectsex.h>

namespace KWin
{

class ItemView
{
public:
    ItemView(EffectWindow *w, int xInList);
    ItemView(ItemView &&info);
    ItemView(const ItemView &info) = delete;
    ~ItemView();

    ItemView &operator=(ItemView &&info);
    ItemView &operator=(const ItemView &info) = delete;

    void updateTexture();
    void clearTexture();

    EffectWindow *window() const { return m_window; }
    QRect frameRect() const { return m_frameRect; }
    QRect thumbnailRect() const { return m_thumbnailRect; }
    GLTexture *windowTexture() const { return m_windowTexture.get(); }
    GLTexture *filledTexture() const { return m_filledTexture.get(); }

private:
    EffectWindow *m_window = nullptr;
    QRect m_frameRect;
    QRect m_thumbnailRect;
    std::unique_ptr<GLFramebuffer> m_fbo = nullptr;
    std::unique_ptr<GLTexture> m_windowTexture = nullptr;
    std::unique_ptr<GLTexture> m_filledTexture = nullptr;
};

class AltTabThumbnailListEffect : public Effect
{
    Q_OBJECT

public:
    AltTabThumbnailListEffect();
    ~AltTabThumbnailListEffect() override;

    bool isActive() const override;
    void reconfigure(ReconfigureFlags flags) override;

    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;
    void postPaintScreen() override;

    static EffectWindow *dockWindow() { return s_dockWindow; }

public Q_SLOTS:
    void slotTabboxAdded(int);
    void slotTabboxClosed();
    void slotTabboxUpdated();

    void slotMouseChanged(const QPoint &pos, const QPoint &old,
                          Qt::MouseButtons buttons, Qt::MouseButtons oldbuttons,
                          Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldmodifiers);

    void slotWindowRemoved(EffectWindow *w);

private:
    void setActive(bool active);

    void updateItemList();
    void updateSelected();
    void updateViewRect();
    void updateVisibleRect();

    void renderSelectedFrame(ScreenPaintData &data);
    void renderItemView(ItemView &item, ScreenPaintData &data);

    QRect rectOnScreen(const QRect &rectInList, const QRect &visible) const;

private:
    struct Animation
    {
        bool enable;
        bool active;
        TimeLine timeLine;
    };
    Animation m_scrollAnimation;
    Animation m_selectAnimation;

    bool m_isActive = false;

    QRect m_viewRect;
    QRect m_visibleRect, m_prevVisibleRect, m_nextVisibleRect;
    QRect m_selectedRect, m_prevSelectedRect, m_nextSelectedRect;

    EffectWindow *m_selectedWindow = nullptr;

    std::unique_ptr<GLShader> m_clipShader = nullptr;
    std::unique_ptr<GLShader> m_scissorShader = nullptr;

    std::unique_ptr<GLTexture> m_scissorMask = nullptr;
    std::unique_ptr<GLTexture> m_selectedFrame = nullptr;

    std::vector<ItemView> m_itemList;

    static EffectWindow *s_dockWindow;
};

}
