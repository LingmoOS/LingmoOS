// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scissorwindow.h"
#include "effects.h"
#include "scene/surfaceitem.h"

#include <kwineffects.h>
#include <kwineffectsex.h>
#include <kwinglplatform.h>
#include <kwinglutils.h>
#include <kwindowsystem.h>

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QTextStream>

#include <algorithm>

Q_DECLARE_METATYPE(QPainterPath)

static void ensureResources()
{
    // Must initialize resources manually because the effect is a static lib.
    Q_INIT_RESOURCE(scissor);
}

namespace KWin {

ScissorWindow::ScissorWindow() : Effect() {
    ensureResources();

    m_maskShader = nullptr;
    m_filletOptimizeShader = nullptr;

    reconfigure(ReconfigureAll);

    m_maskShader = ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture,
                                                                     QByteArray(),
                                                                     ":/effects/scissor/mask.frag");
    m_filletOptimizeShader = ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture,
                                                                               QByteArray(),
                                                                               ":/effects/scissor/fillet.frag");

    {
        for (int i = 0; i < KWindowSystem::windows().count(); ++i) {
            if (EffectWindow *win = effects->findWindow(KWindowSystem::windows().at(i)))
                windowAdded(win);
        }
        connect(effects, &EffectsHandler::windowAdded, this, &ScissorWindow::windowAdded);
        connect(effects, &EffectsHandler::windowDeleted, this, &ScissorWindow::windowDeleted);
    }
}

ScissorWindow::~ScissorWindow() {
    for (auto itr = m_texMaskMap.begin(); itr != m_texMaskMap.end(); itr++) {
        delete itr->second;
    }
    m_texMaskMap.clear();
}

void ScissorWindow::reconfigure(ReconfigureFlags flags) {
    Q_UNUSED(flags)
}

void ScissorWindow::buildTextureMask(const QString& key, const QPointF& radius) {
    QImage img(QSize(radius.x() * 2, radius.y() * 2), QImage::Format_RGBA8888);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 255));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawEllipse(0, 0, radius.x() * 2, radius.y() * 2);
    painter.end();

    m_texMaskMap[key] = new GLTexture(img.copy(0, 0, radius.x(), radius.y()));
    m_texMaskMap[key]->setFilter(GL_LINEAR);
    m_texMaskMap[key]->setWrapMode(GL_CLAMP_TO_EDGE);
}

void ScissorWindow::prePaintWindow(EffectWindow *w, WindowPrePaintData &data,
                                   std::chrono::milliseconds time) {
    if (effects->hasActiveFullScreenEffect() ||
        w->isDesktop() || w->isOutline() || w->isSplitBar() || effectsEx->isSplitWin(w) || isMaximized(w)) {
        return effects->prePaintWindow(w, data, time);
    }

    QPointF cornerRadius;
    const QVariant valueRadius = w->data(WindowRadiusRole);
    if (valueRadius.isValid()) {
        cornerRadius = valueRadius.toPointF();
        const qreal xMin{ std::min(cornerRadius.x(), w->width() / 2.0) };
        const qreal yMin{ std::min(cornerRadius.y(), w->height() / 2.0) };
        const qreal minRadius{ std::min(xMin, yMin) };
        cornerRadius = QPointF(minRadius, minRadius);
    }
    if (!cornerRadius.isNull()) {
        int radiusX = qRound(cornerRadius.x());
        int radiusY = qRound(cornerRadius.y());
        QRect corner1(w->frameGeometry().topLeft().toPoint(), QSize(radiusX, radiusY));
        QRect corner2(w->frameGeometry().topRight().x()- radiusX, w->frameGeometry().topRight().y(), radiusX, radiusY);
        QRect corner3(w->frameGeometry().bottomLeft().x() , w->frameGeometry().bottomLeft().y() - radiusY, radiusX, radiusY);
        QRect corner4(w->frameGeometry().bottomRight().x() - radiusX, w->frameGeometry().bottomRight().y()- radiusY, radiusX, radiusY);
        data.paint = data.paint | corner1 | corner2 | corner3 | corner4;
        data.opaque = data.opaque - corner1 - corner2 -corner3 - corner4;
    }
    effects->prePaintWindow(w, data, time);
}

void ScissorWindow::drawWindow(EffectWindow *w, int mask, const QRegion& region, WindowPaintData &data) {
    if (w->isOutline() || w->isSplitBar() || effectsEx->isSplitWin(w) || isMaximized(w)) {
        return effects->drawWindow(w, mask, region, data);
    }

    if (const auto &data_clip_path = w->data(WindowClipPathRole); data_clip_path.isValid()) {
        const QPainterPath path = qvariant_cast<QPainterPath>(data_clip_path);
        static const int extraWindowFrame = 100;

        if (!m_clipMaskMap.count(w) || m_clipMaskMap[w].maskPath != path) {
            QImage maskImage(w->size().toSize() * 2, QImage::Format_RGBA8888);
            maskImage.fill(Qt::transparent);
            QPainter pa(&maskImage);
            pa.setRenderHint(QPainter::Antialiasing);
            pa.scale(2, 2);
            pa.fillPath(path, QColor(255, 255, 255, 255));
            pa.strokePath(path, QPen(QColor(80, 80, 80, 60), 2));
            pa.end();

            m_clipMaskMap[w] = WindowMaskCache {
                .maskPath = path,
                .maskTexture = std::make_shared<GLTexture>(maskImage),
            };

            m_clipMaskMap[w].maskTexture->setFilter(GL_LINEAR);
            m_clipMaskMap[w].maskTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        }

        const WindowMaskCache& cache = m_clipMaskMap[w];

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        QRect rect = w->rect().toRect();
        rect.adjust(-extraWindowFrame, -extraWindowFrame, extraWindowFrame, extraWindowFrame);

        {
            ShaderManager::instance()->pushShader(m_maskShader.get());
            m_maskShader->setUniform("sampler", 0);
            m_maskShader->setUniform("msk1", 2);

            auto old_shader = data.shader;
            data.shader = m_maskShader.get();

            std::shared_ptr<GLTexture> maskTexture = cache.maskTexture;
            glActiveTexture(GL_TEXTURE2); maskTexture->bind();

            glActiveTexture(GL_TEXTURE0);
            effects->drawWindow(w, mask, region, data);

            ShaderManager::instance()->popShader();
            data.shader = old_shader;

            glActiveTexture(GL_TEXTURE2);
            glActiveTexture(GL_TEXTURE0);

            maskTexture->unbind();
        }

        return;
    } else {
        QPointF cornerRadius;
        const QVariant valueRadius = w->data(WindowRadiusRole);
        if (valueRadius.isValid()) {
            cornerRadius = valueRadius.toPointF();
            const qreal xMin{ std::min(cornerRadius.x(), w->width() / 2.0) };
            const qreal yMin{ std::min(cornerRadius.y(), w->height() / 2.0) };
            const qreal minRadius{ std::min(xMin, yMin) };
            cornerRadius = QPointF(minRadius, minRadius);
        }
        if (cornerRadius.isNull()) {
            effects->drawWindow(w, mask, region, data);
        } else {
            Window* window = static_cast<EffectWindowImpl *>(w)->window();
            SurfaceItem *surfaceItem = static_cast<SurfaceItem *>(window->surfaceItem());
            if (surfaceItem)
                surfaceItem->setScissorAlpha(true);
            const QString& key = QString("%1+%2").arg(cornerRadius.toPoint().x()).arg(cornerRadius.toPoint().y()
            );
            if (!m_texMaskMap.count(key)) {
                buildTextureMask(key, cornerRadius);
            }

            ShaderManager::instance()->pushShader(m_filletOptimizeShader.get());
            m_filletOptimizeShader->setUniform("typ1", 1);
            m_filletOptimizeShader->setUniform("sampler", 0);
            m_filletOptimizeShader->setUniform("msk1", 1);
            m_filletOptimizeShader->setUniform("k", QVector2D(w->width() / cornerRadius.x(), w->height() / cornerRadius.y()));
            if (w->hasDecoration()) {
                m_filletOptimizeShader->setUniform("typ2", 0);
            } else {
                m_filletOptimizeShader->setUniform("typ2", 1);
            }

            auto old_shader = data.shader;
            data.shader = m_filletOptimizeShader.get();

            glActiveTexture(GL_TEXTURE1);
            m_texMaskMap[key]->bind();
            glActiveTexture(GL_TEXTURE0);
            effects->drawWindow(w, mask, region, data);
            ShaderManager::instance()->popShader();
            data.shader = old_shader;
            glActiveTexture(GL_TEXTURE1);
            m_texMaskMap[key]->unbind();
            glActiveTexture(GL_TEXTURE0);
            return;
        }
    }
}

bool ScissorWindow::enabledByDefault() { return supported(); }

bool ScissorWindow::supported() {
    return effects->isOpenGLCompositing() && GLFramebuffer::supported();
}

void ScissorWindow::windowAdded(EffectWindow *w) {
    if (!w->hasDecoration())
        return;
}

void ScissorWindow::windowDeleted(EffectWindow *w) {
    m_clipMaskMap.erase(w);
}

bool ScissorWindow::isMaximized(EffectWindow *w) {
    auto geom = effects->findScreen(w->screen()->name())->geometry();
    return (w->x() == geom.x() && w->width() == geom.width()) &&
           (w->y() == geom.y() && w->height() == geom.height());
}

bool ScissorWindow::isMaximized(EffectWindow *w, const PaintData& data)
{
    auto geom = effects->findScreen(w->screen()->name())->geometry();
    return (w->x() + data.xTranslation() == geom.x() && w->width() * data.xScale() == geom.width()) ||
            (w->y() + data.yTranslation() == geom.y() && w->height() * data.yScale() == geom.height());
}

}  // namespace KWin
