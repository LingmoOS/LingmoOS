// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "alttabthumbnaillist.h"

#include <QPainter>

#include <effects.h>
#include <kwinglutils.h>
#include "scene/itemrenderer.h"
#include "scene/windowitem.h"
#include "workspace.h"

#define THUMBNAIL_HEIGHT        (118 * effectsEx->getOsScale())
#define THUMBNAIL_HOVER_WIDTH   (4 * effectsEx->getOsScale())
#define THUMBNAIL_HOVER_MARGIN  (7 * effectsEx->getOsScale())
#define THUMBNAIL_ICON_SIZE     (24 * effectsEx->getOsScale())
#define THUMBNAIL_RADIUS        (12 * effectsEx->getOsScale())
#define THUMBNAIL_SPACING       (14 * effectsEx->getOsScale())
#define THUMBNAIL_SIDE_MARGIN   (20 * effectsEx->getOsScale())
#define THUMBNAIL_TITLE_HEIGHT  (40 * effectsEx->getOsScale())
#define THUMBNAIL_TITLE_MARGIN  (8 * effectsEx->getOsScale())
#define THUMBNAIL_TEXT_SIZE     (14 * effectsEx->getOsScale())

#define THUMBNAIL_VIEW_BOTTOM_MARGIN    (100 * effectsEx->getOsScale())
#define THUMBNAIL_VIEW_HEIGHT           (198 * effectsEx->getOsScale())
#define THUMBNAIL_VIEW_SIDE_MARGIN      (10 * effectsEx->getOsScale())

Q_LOGGING_CATEGORY(KWIN_ALTTABTHUMBNAILLIST, "kwin_effect_alttabthumbnaillist", QtWarningMsg)

static void ensureResources()
{
    Q_INIT_RESOURCE(alttabthumbnaillist);
}

static QRect heightScaledRect(const QRect &r, const int height)
{
    if (r.height() < height || r.height() <= 0)
        return QRect(0, 0, r.width(), r.height());

    const int width = std::ceil(r.width() * (float(height) / r.height()));
    return QRect(0, 0, width, height);
}

namespace KWin
{

ItemView::ItemView(EffectWindow *w, int xInList)
    : m_window(w)
{
    if (window()) {
        const QRect scaled = heightScaledRect(window()->geometry().toRect(), THUMBNAIL_HEIGHT);
        m_frameRect = QRect(xInList, 0, scaled.width(), THUMBNAIL_TITLE_HEIGHT + THUMBNAIL_HEIGHT);
        m_thumbnailRect = scaled.translated(xInList,
                THUMBNAIL_TITLE_HEIGHT + (THUMBNAIL_HEIGHT - scaled.height()) / 2);
    }
}

ItemView::ItemView(ItemView &&info)
{
    *this = std::move(info);
}

ItemView &ItemView::operator=(ItemView &&info)
{
    if (this != &info) {
        m_window = info.m_window;
        m_frameRect = info.m_frameRect;
        m_thumbnailRect = info.m_thumbnailRect;
        m_fbo = std::move(info.m_fbo);
        m_windowTexture = std::move(info.m_windowTexture);
        m_filledTexture = std::move(info.m_filledTexture);
    }
    return *this;
}

ItemView::~ItemView()
{
    clearTexture();
}

void ItemView::updateTexture()
{
    if (!window() || thumbnailRect().isEmpty())
        return;

    if (!m_windowTexture || m_windowTexture->size() != thumbnailRect().size()) {
        m_windowTexture = std::make_unique<GLTexture>(GL_RGBA8, thumbnailRect().size());
        m_windowTexture->setFilter(GL_LINEAR);
        m_windowTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        m_windowTexture->setYInverted(true);
        m_fbo = std::make_unique<GLFramebuffer>(m_windowTexture.get());
    }

    // update window thumbnail
    if (m_fbo) {
        GLFramebuffer::pushFramebuffer(m_fbo.get());

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        ItemRenderer *renderer = static_cast<EffectsHandlerImpl *>(effects)->scene()->renderer();
        const qreal scale = renderer->renderTargetScale();
        QMatrix4x4 projectionMatrix;
        const QRectF geometry = window()->frameGeometry();
        projectionMatrix.ortho(geometry.x() * scale, (geometry.x() + geometry.width()) * scale,
                               geometry.y() * scale, (geometry.y() + geometry.height()) * scale, -1, 1);
        WindowPaintData data;
        data.setProjectionMatrix(projectionMatrix);
        const int mask = Scene::PAINT_WINDOW_TRANSFORMED;
        renderer->renderItem(static_cast<EffectWindowImpl *>(window())->windowItem(), mask, infiniteRegion(), data);

        if (window()->isDesktop()) {
            EffectWindow *dock = AltTabThumbnailListEffect::dockWindow();
            if (dock && geometry.contains(dock->frameGeometry()))
                renderer->renderItem(static_cast<EffectWindowImpl *>(dock)->windowItem(), mask, infiniteRegion(), data);
        }

        GLFramebuffer::popFramebuffer();
    }

    // update filled
    const QRect filled_rect(QPoint(0, 0), frameRect().size() + QSize(2, 2));  // consider border
    if (!m_filledTexture || m_filledTexture->size() != filled_rect.size()) {
        QPixmap pixmap(filled_rect.size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const float radius = THUMBNAIL_RADIUS;
        // inner rect
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#CCFFFFFF"));
        painter.drawRoundedRect(frameRect().translated(1 - frameRect().x(), 1), radius, radius);
        if (m_thumbnailRect.height() >= THUMBNAIL_HEIGHT) {
            QPainter::CompositionMode mode = painter.compositionMode();
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.setBrush(Qt::transparent);
            painter.drawRect(thumbnailRect().translated(1 - thumbnailRect().x(), 1));
            painter.setCompositionMode(mode);
        }
        // border
        painter.setPen(QPen(QColor("#19000000"), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(filled_rect, radius + 1, radius + 1);
        // split line
        painter.setPen(QPen(QColor("#0D000000"), 1));
        painter.drawLine(QPointF(0, THUMBNAIL_TITLE_HEIGHT),
                         QPointF(filled_rect.width(), THUMBNAIL_TITLE_HEIGHT));
        // icon
        const qreal verical_margin = (THUMBNAIL_TITLE_HEIGHT - THUMBNAIL_ICON_SIZE) / 2;
        const QRectF icon_rect(QPointF(THUMBNAIL_TITLE_MARGIN, verical_margin),
                               QSizeF(THUMBNAIL_ICON_SIZE, THUMBNAIL_ICON_SIZE));
        const QIcon icon = window()->isDesktop() ? QIcon::fromTheme(QStringLiteral("deepin-toggle-desktop"))
                                                 : window()->icon();
        painter.drawPixmap(icon_rect.topLeft(), icon.pixmap(THUMBNAIL_ICON_SIZE, THUMBNAIL_ICON_SIZE));
        // title
        QFont font;
        font.setPixelSize(THUMBNAIL_TEXT_SIZE);
        const QFontMetricsF metrics(font);
        const QRectF text_rect(icon_rect.x() + icon_rect.width() + THUMBNAIL_TITLE_MARGIN, 0,
                               filled_rect.width() - icon_rect.width() - 3 * THUMBNAIL_TITLE_MARGIN,
                               THUMBNAIL_TITLE_HEIGHT);
        const QString elided = metrics.elidedText(window()->caption(), Qt::ElideRight, text_rect.width());
        painter.setPen(Qt::black);
        painter.setFont(font);
        painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, elided);

        m_filledTexture = std::make_unique<GLTexture>(pixmap);
    }
}

void ItemView::clearTexture()
{
    m_fbo.reset();
    m_windowTexture.reset();
    m_filledTexture.reset();
}

//----------------------------thumbnail list effect-----------------------------

EffectWindow *AltTabThumbnailListEffect::s_dockWindow = nullptr;

AltTabThumbnailListEffect::AltTabThumbnailListEffect()
{
    ensureResources();
    reconfigure(ReconfigureFlag::ReconfigureAll);

    m_clipShader = ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture,
                                                                     QByteArray(),
                                                                     ":/effects/alttabthumbnaillist/shaders/clip.frag");
    if (!m_clipShader)
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to load filled clip shader";

    m_scissorShader = ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture,
                                                                        QByteArray(),
                                                                        ":/effects/alttabthumbnaillist/shaders/scissor.frag");
    if (!m_scissorShader)
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to load thumbnail scissor shader";

    connect(effects, &EffectsHandler::tabBoxAdded, this, &AltTabThumbnailListEffect::slotTabboxAdded);
    connect(effects, &EffectsHandler::tabBoxClosed, this, &AltTabThumbnailListEffect::slotTabboxClosed);
    connect(effects, &EffectsHandler::tabBoxUpdated, this, &AltTabThumbnailListEffect::slotTabboxUpdated);

    connect(effects, &EffectsHandler::mouseChanged, this, &AltTabThumbnailListEffect::slotMouseChanged);

    connect(effects, &EffectsHandler::windowClosed, this, &AltTabThumbnailListEffect::slotWindowRemoved);
    connect(effects, &EffectsHandler::windowDeleted, this, &AltTabThumbnailListEffect::slotWindowRemoved);
}

AltTabThumbnailListEffect::~AltTabThumbnailListEffect()
{
    setActive(false);

    m_scissorShader.reset();
    m_clipShader.reset();
    m_scissorMask.reset();
    m_selectedFrame.reset();
}

bool AltTabThumbnailListEffect::isActive() const
{
    return m_isActive;
}

void AltTabThumbnailListEffect::reconfigure(ReconfigureFlags flags)
{
    const bool enable = effects->waylandDisplay() || effectsEx->effectType() == EffectType::OpenGLComplete;

    m_scrollAnimation.enable = enable;
    m_scrollAnimation.active = false;
    m_scrollAnimation.timeLine.setDuration(std::chrono::milliseconds(static_cast<int>(animationTime(400))));
    m_scrollAnimation.timeLine.setEasingCurve(QEasingCurve::OutExpo);

    m_selectAnimation.enable = enable;
    m_selectAnimation.active = false;
    m_selectAnimation.timeLine.setDuration(std::chrono::milliseconds(static_cast<int>(animationTime(400))));
    m_selectAnimation.timeLine.setEasingCurve(QEasingCurve::OutExpo);
}

void AltTabThumbnailListEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (w->isDock()) {
        s_dockWindow = w;
    } else if (w->isDesktop()) {
        data.setBrightness(0.4);
    }
    effects->paintWindow(w, mask, region, data);
}

void AltTabThumbnailListEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    if (m_scrollAnimation.active)
        m_scrollAnimation.timeLine.advance(presentTime);

    if (m_selectAnimation.active)
        m_selectAnimation.timeLine.advance(presentTime);

    effects->prePaintScreen(data, presentTime);
}

void AltTabThumbnailListEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);

    if (m_scrollAnimation.active) {
        QPoint pos_diff = m_nextVisibleRect.topLeft() - m_prevVisibleRect.topLeft();
        pos_diff *= m_scrollAnimation.timeLine.value();
        m_visibleRect = m_prevVisibleRect.translated(pos_diff);
    }

    if (m_selectAnimation.active) {
        // repaint last frame
        if (m_selectedRect.isValid())
            effects->addRepaint(m_selectedRect);

        QPoint pos_diff = m_nextSelectedRect.topLeft() - m_prevSelectedRect.topLeft();
        pos_diff *= m_selectAnimation.timeLine.value();
        QSize size_diff = m_nextSelectedRect.size() - m_prevSelectedRect.size();
        size_diff *= m_selectAnimation.timeLine.value();
        m_selectedRect = QRect(m_prevSelectedRect.topLeft() + pos_diff,
                               m_prevSelectedRect.size() + size_diff);
    }

    renderSelectedFrame(data);

    workspace()->forceDisableRadius(true);
    for (ItemView &item : m_itemList) {
        if (item.frameRect().intersects(m_visibleRect)) {
            renderItemView(item, data);
        } else {
            item.clearTexture();
        }
    }
    workspace()->forceDisableRadius(false);
}

void AltTabThumbnailListEffect::postPaintScreen()
{
    if (m_scrollAnimation.timeLine.done()) {
        m_visibleRect = m_prevVisibleRect = m_nextVisibleRect;
        m_scrollAnimation.active = false;
    }

    if (m_selectAnimation.timeLine.done()) {
        m_selectedRect = m_prevSelectedRect = m_nextSelectedRect;
        m_selectAnimation.active = false;
    }

    effects->postPaintScreen();
}

void AltTabThumbnailListEffect::slotTabboxAdded(int)
{
    updateItemList();
    if (m_itemList.empty() || (m_itemList.size() == 1 && m_itemList.front().window()->isDesktop())) {
        effects->closeTabBox();
        return;
    }

    // init visible rect
    const int list_width = (m_itemList.back().frameRect().right() + 1) + THUMBNAIL_SIDE_MARGIN;
    m_visibleRect = QRect(0, 0, m_viewRect.width(), m_viewRect.height());
    if (m_visibleRect.width() < list_width) {
        for (ItemView &item : m_itemList) {
            const QRect rect = item.frameRect();
            const int offset = rect.width() < 40 ? rect.width() / 2 : 20;
            if (rect.right() < m_visibleRect.right() + offset)
                continue;
            // ensure the last visible thumbnail is truncated
            if (rect.left() > (m_visibleRect.right() + 1) - offset)
                m_visibleRect.translate(rect.left() - ((m_visibleRect.right() + 1) - offset), 0);
            break;
        }
    }
    m_prevVisibleRect = m_nextVisibleRect = m_visibleRect;

    updateSelected();
    setActive(true);
}

void AltTabThumbnailListEffect::slotTabboxClosed()
{
    if (!isActive())
        return;

    setActive(false);
}

void AltTabThumbnailListEffect::slotTabboxUpdated()
{
    if (!isActive())
        return;

    updateItemList();
    updateSelected();
}

void AltTabThumbnailListEffect::slotMouseChanged(const QPoint &pos, const QPoint &old,
                                           Qt::MouseButtons buttons, Qt::MouseButtons oldbuttons,
                                           Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldmodifiers)
{
    if (!isActive() || buttons != Qt::MouseButton::LeftButton)
        return;

    for (ItemView &item : m_itemList) {
        if (rectOnScreen(item.frameRect(), m_visibleRect).contains(pos)) {
            effects->setTabBoxWindow(item.window());
            updateSelected();
            return;
        }
    }
}

void AltTabThumbnailListEffect::slotWindowRemoved(EffectWindow *w)
{
    if (!isActive())
        return;
    if (s_dockWindow == w)
        s_dockWindow = nullptr;
}

void AltTabThumbnailListEffect::setActive(bool active)
{
    m_isActive = active;

    effects->addRepaintFull();

    // inactive
    if (!active) {
        m_scrollAnimation.active = false;
        m_scrollAnimation.timeLine.reset();
        m_selectAnimation.active = false;
        m_selectAnimation.timeLine.reset();

        m_visibleRect = m_prevVisibleRect = m_nextVisibleRect = QRect();
        m_selectedRect = m_prevSelectedRect = m_nextSelectedRect = QRect();

        m_selectedWindow = nullptr;
        s_dockWindow = nullptr;

        effects->makeOpenGLContextCurrent();
        m_itemList.clear();
        return;
    }
}

void AltTabThumbnailListEffect::updateItemList()
{
    EffectWindowList list = effects->currentTabBoxWindowList();
    const bool equal = std::equal(m_itemList.begin(), m_itemList.end(), list.begin(), list.end(),
        [] (ItemView &a, EffectWindow *b) { return a.window() == b; }
    );
    if (equal)
        return;

    m_itemList.clear();
    int x = THUMBNAIL_SIDE_MARGIN;
    for (EffectWindow *w : list) {
        if (Q_UNLIKELY(!w))
            continue;
        m_itemList.emplace_back(w, x);
        x += m_itemList.back().frameRect().width() + THUMBNAIL_SPACING;
    }
    if (m_itemList.empty()) {
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Empty thumbnail window list";
        return;
    }

    updateViewRect();
}

void AltTabThumbnailListEffect::updateSelected()
{
    m_selectedWindow = effects->currentTabBoxWindow();
    if (!m_selectedWindow)
        return;

    updateVisibleRect();

    // process select animation
    m_prevSelectedRect = m_selectedRect;
    auto it = std::find_if(m_itemList.begin(), m_itemList.end(),
            [this] (ItemView &item) { return item.window() == m_selectedWindow; });
    m_nextSelectedRect = (it == m_itemList.end()) ? QRect() : rectOnScreen(it->frameRect(), m_nextVisibleRect);
    if (m_nextSelectedRect.isValid()) {
        const int margin = THUMBNAIL_HOVER_MARGIN;
        m_nextSelectedRect.adjust(-margin, -margin, margin, margin);
        if (m_prevSelectedRect != m_nextSelectedRect) {
            if (m_prevSelectedRect.isValid() && m_selectAnimation.enable) {
                m_selectAnimation.active = true;
                m_selectAnimation.timeLine.reset();
            } else {
                m_selectedRect = m_prevSelectedRect = m_nextSelectedRect;
            }
        }
    } else {
        m_nextSelectedRect = m_selectedRect;
    }
}

void AltTabThumbnailListEffect::updateViewRect()
{
    const QRect client = effects->clientArea(
            clientAreaOption::PlacementArea, effects->activeScreen(), effects->currentDesktop()).toRect();
    const QRect full = effects->activeScreen()->geometry();
    const QRect screen_geometry(client.x(), full.y(), client.width(), full.height());

    // top and bottom of view are fixed
    m_viewRect.setBottom(screen_geometry.bottom() - THUMBNAIL_VIEW_BOTTOM_MARGIN);
    m_viewRect.setTop(m_viewRect.bottom() - THUMBNAIL_VIEW_HEIGHT + 1);

    if (m_itemList.empty())
        return;
    const int list_width = (m_itemList.back().frameRect().right() + 1) + THUMBNAIL_SIDE_MARGIN;
    if (list_width <= screen_geometry.width() - THUMBNAIL_VIEW_SIDE_MARGIN * 2) {
        const int left = screen_geometry.left() + (screen_geometry.width() - list_width) / 2;
        m_viewRect.setLeft(left);
        m_viewRect.setWidth(list_width);
    } else {
        m_viewRect.setLeft(screen_geometry.left() + THUMBNAIL_VIEW_SIDE_MARGIN);
        m_viewRect.setRight((screen_geometry.right() + 1) - THUMBNAIL_VIEW_SIDE_MARGIN);
    }

    effects->setTabBoxViewRect(m_viewRect.adjusted(-1, -1, 1, 1));  // adjust border

    m_visibleRect.setWidth(m_viewRect.width());
    if (m_visibleRect.right() + 1 > list_width)
        m_visibleRect.translate(list_width - (m_visibleRect.right() + 1), 0);
    updateVisibleRect();
}

void AltTabThumbnailListEffect::updateVisibleRect()
{
    auto it = std::find_if(m_itemList.begin(), m_itemList.end(),
            [this] (ItemView &item) { return item.window() == m_selectedWindow; });
    if (it == m_itemList.end()) {
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to find selected client in list";
    } else {
        const QRect selected_rect = it->frameRect();
        const int spacing = (it == m_itemList.begin() || it == m_itemList.end() - 1) ? THUMBNAIL_SIDE_MARGIN : THUMBNAIL_SPACING;

        m_prevVisibleRect = m_nextVisibleRect = m_visibleRect;
        // scroll left
        if (selected_rect.left() - spacing < m_visibleRect.left()) {
            if (it == m_itemList.begin()) {
                m_nextVisibleRect.moveTo(0, m_nextVisibleRect.top());
            } else {
                m_nextVisibleRect.moveTo((it - 1)->frameRect().center().x(), m_nextVisibleRect.top());
            }
        // scroll right
        } else if (selected_rect.right() + spacing > m_visibleRect.right()) {
            if (it == m_itemList.end() - 1) {
                m_nextVisibleRect.moveTo((selected_rect.right() + 1) + THUMBNAIL_SIDE_MARGIN - m_nextVisibleRect.width(), m_nextVisibleRect.top());
            } else {
                m_nextVisibleRect.moveTo((it + 1)->frameRect().center().x() - m_nextVisibleRect.width(), m_nextVisibleRect.top());
            }
        }

        // process scroll animation
        if (m_nextVisibleRect.isValid() && m_prevVisibleRect != m_nextVisibleRect) {
            if (m_prevVisibleRect.isValid() && m_scrollAnimation.enable) {
                m_scrollAnimation.active = true;
                m_scrollAnimation.timeLine.reset();
            } else {
                m_visibleRect = m_prevVisibleRect = m_nextVisibleRect;
            }
        } else {
            m_nextVisibleRect = m_visibleRect;
        }
    }
}

void AltTabThumbnailListEffect::renderSelectedFrame(ScreenPaintData &data)
{
    // update texture
    if (!m_selectedFrame || m_selectedFrame->size() != m_selectedRect.size()) {
        const QRect r(QPoint(0, 0), m_selectedRect.size());
        QPixmap pixmap(r.size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        const float radius = THUMBNAIL_RADIUS + THUMBNAIL_HOVER_MARGIN;
        painter.setBrush(QColor(workspace()->ActiveColor()));
        painter.drawRoundedRect(r, radius, radius);

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setBrush(Qt::transparent);
        const float width = THUMBNAIL_HOVER_WIDTH;
        painter.drawRoundedRect(r.adjusted(width, width, -width, -width), radius - width, radius - width);

        m_selectedFrame = std::make_unique<GLTexture>(pixmap);
    }

    if (!m_selectedFrame || m_selectedFrame->isNull()) {
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to render selected frame texture";
        return;
    }

    const QRectF scaled = scaledRect(m_selectedRect, effects->renderTargetScale());

    GLShader *shader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture);

    QMatrix4x4 mvp(data.projectionMatrix());
    mvp.translate(scaled.x(), scaled.y());
    shader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    m_selectedFrame->bind();
    m_selectedFrame->render(scaled.toRect(), effects->renderTargetScale());
    m_selectedFrame->unbind();

    glDisable(GL_BLEND);

    ShaderManager::instance()->popShader();
}

void AltTabThumbnailListEffect::renderItemView(ItemView &item, ScreenPaintData &data)
{
    const QRect trans_visible = m_visibleRect.translated(m_viewRect.left() - m_visibleRect.left(),
                                                         m_viewRect.top());

    EffectWindow *w = item.window();
    item.updateTexture();

    // render filled
    GLTexture *texture = item.filledTexture();
    if (!texture || texture->isNull()) {
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to render filled texture of " << w->caption();
    } else {
        ShaderManager::instance()->pushShader(m_clipShader.get());

        const QRect filled_rect = rectOnScreen(item.frameRect(), m_visibleRect).adjusted(-1, -1, 1, 1);  // consider border
        int clip = 0;
        if (filled_rect.left() < trans_visible.left()) {
            clip = filled_rect.left() - trans_visible.left();
        } else if (filled_rect.right() > trans_visible.right()) {
            clip = filled_rect.right() - trans_visible.right();
        }
        const float clip_factor = float(clip) / filled_rect.width();

        QMatrix4x4 mvp(data.projectionMatrix());
        mvp.translate(filled_rect.x() * effects->renderTargetScale(),
                      filled_rect.y() * effects->renderTargetScale());

        m_clipShader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);
        m_clipShader->setUniform("clip_left", clip_factor < 0 ? -clip_factor : 0);
        m_clipShader->setUniform("clip_right", clip_factor > 0 ? 1 - clip_factor : 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        texture->bind();
        texture->render(filled_rect, effects->renderTargetScale());
        texture->unbind();

        glDisable(GL_BLEND);

        ShaderManager::instance()->popShader();
    }

    // build scissor mask
    if (!m_scissorMask) {
        QImage img(QSize(THUMBNAIL_RADIUS * 2, THUMBNAIL_RADIUS * 2), QImage::Format_RGBA8888);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 255));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawEllipse(0, 0, THUMBNAIL_RADIUS * 2, THUMBNAIL_RADIUS * 2);
        painter.end();

        m_scissorMask = std::make_unique<GLTexture>(img.copy(0, 0, THUMBNAIL_RADIUS, THUMBNAIL_RADIUS));
        m_scissorMask->setFilter(GL_LINEAR);
        m_scissorMask->setWrapMode(GL_CLAMP_TO_EDGE);
    }

    // render thumbnail
    texture = item.windowTexture();
    if (!texture || texture->isNull()) {
        qWarning(KWIN_ALTTABTHUMBNAILLIST) << "Failed to load thumbnail texture of " << w->caption();
    } else {
        ShaderManager::instance()->pushShader(m_scissorShader.get());

        const QRect thumbnail_rect = rectOnScreen(item.thumbnailRect(), m_visibleRect);
        const float width_factor = float(thumbnail_rect.width()) / w->width();
        const float height_factor = float(thumbnail_rect.height()) / w->height();
        int clip = 0;
        if (thumbnail_rect.left() < trans_visible.left()) {
            clip = thumbnail_rect.left() - trans_visible.left();
        } else if (thumbnail_rect.right() > trans_visible.right()) {
            clip = thumbnail_rect.right() - trans_visible.right();
        }
        const float clip_factor = float(clip) / thumbnail_rect.width();

        QMatrix4x4 mvp(data.projectionMatrix());
        mvp.translate(thumbnail_rect.x() * effects->renderTargetScale(),
                      thumbnail_rect.y() * effects->renderTargetScale());

        m_scissorShader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);
        m_scissorShader->setUniform("sampler", 0);
        m_scissorShader->setUniform("msk1", 1);
        m_scissorShader->setUniform("k",
                                    QVector2D(w->width() * width_factor / THUMBNAIL_RADIUS,
                                              w->height() * height_factor / THUMBNAIL_RADIUS));
        m_scissorShader->setUniform("clip_left", clip_factor < 0 ? -clip_factor : 0);
        m_scissorShader->setUniform("clip_right", clip_factor > 0 ? 1 - clip_factor : 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE1);
        m_scissorMask->bind();
        glActiveTexture(GL_TEXTURE0);
        texture->bind();
        texture->render(thumbnail_rect, effects->renderTargetScale());
        glActiveTexture(GL_TEXTURE1);
        m_scissorMask->unbind();
        glActiveTexture(GL_TEXTURE0);
        texture->unbind();

        glDisable(GL_BLEND);

        ShaderManager::instance()->popShader();
    }
}

QRect AltTabThumbnailListEffect::rectOnScreen(const QRect &rectInList, const QRect &visible) const
{
    if (!rectInList.isValid() || !visible.isValid() || !rectInList.intersects(visible))
        return QRect();

    const int item_top_margin = (m_viewRect.height() - THUMBNAIL_TITLE_HEIGHT - THUMBNAIL_HEIGHT) / 2;
    const int item_y = m_viewRect.top() + item_top_margin;
    QRect item_rect = rectInList.translated(-visible.left() + m_viewRect.left(), item_y);
    return item_rect;
}

}
