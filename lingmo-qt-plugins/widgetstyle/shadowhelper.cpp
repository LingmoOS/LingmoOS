/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 * Copyright (C) 2018, 2020 by Vlad Zahorodnii <vlad.zahorodnii@kde.org> *
 * Copyright (C) 2020, 2020 by Reven Martin <revenmartin@gmail.com>      *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "shadowhelper.h"
#include "boxshadowrenderer.h"

#include <QDockWidget>
#include <QEvent>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPlatformSurfaceEvent>
#include <QToolBar>
#include <QTextStream>

#include <KWindowSystem>

const char netWMSkipShadow[] = "_LINGMO_NET_WM_SKIP_SHADOW";
const char netWMForceShadow[] = "_LINGMO_NET_WM_FORCE_SHADOW";
const char netWMFrameRadius[] = "_LINGMO_NET_WM_FRAME_RADIUS";

enum {
    ShadowNone,
    ShadowSmall,
    ShadowMedium,
    ShadowLarge,
    ShadowVeryLarge
};

const CompositeShadowParams s_shadowParams[] = {
    // None
    CompositeShadowParams(),
    // Small
    CompositeShadowParams(
        QPoint(0, 3),
        ShadowParams(QPoint(0, 0), 16, 0.26),
        ShadowParams(QPoint(0, -2), 8, 0.16)),
    // Medium
    CompositeShadowParams(
        QPoint(0, 4),
        ShadowParams(QPoint(0, 0), 20, 0.24),
        ShadowParams(QPoint(0, -2), 10, 0.14)),
    // Large
    CompositeShadowParams(
        QPoint(0, 5),
        ShadowParams(QPoint(0, 0), 24, 0.22),
        ShadowParams(QPoint(0, -3), 12, 0.12)),
    // Very Large
    CompositeShadowParams(
        QPoint(0, 6),
        ShadowParams(QPoint(0, 0), 32, 0.2),
        ShadowParams(QPoint(0, -3), 16, 0.1))
};


ShadowHelper::ShadowHelper(QObject * parent)
    : QObject(parent),
      m_frameRadius(12)
{
}

ShadowHelper::~ShadowHelper()
{
}

CompositeShadowParams ShadowHelper::lookupShadowParams(int shadowSizeEnum)
{
    switch (shadowSizeEnum) {
    case ShadowNone:
        return s_shadowParams[0];
    case ShadowSmall:
        return s_shadowParams[1];
    case ShadowMedium:
        return s_shadowParams[2];
    case ShadowLarge:
        return s_shadowParams[3];
    case ShadowVeryLarge:
        return s_shadowParams[4];
    default:
        // Fallback to the Large size.
        return s_shadowParams[3];
    }
}

bool ShadowHelper::registerWidget(QWidget *widget, bool force)
{
    // make sure widget is not already registered
    if (m_widgets.contains(widget))
        return false;

    // check if widget qualifies
    if (!(force || acceptWidget(widget)))
        return false;

    qreal frameRadius = m_frameRadius;
    const auto frameRadiusProperty = widget->property(netWMFrameRadius);
    if (frameRadiusProperty.isValid())
        frameRadius = frameRadiusProperty.toReal();

    installShadows(widget, shadowTiles(frameRadius));
    m_widgets.insert(widget);

    // install event filter
    widget->removeEventFilter(this);
    widget->installEventFilter(this);

    // connect destroy signal
    connect(widget, &QObject::destroyed, this, &ShadowHelper::objectDeleted);

    return true;
}

void ShadowHelper::unregisterWidget(QWidget *widget)
{
    if (m_widgets.remove(widget)) {
        // uninstall the event filter
        widget->removeEventFilter(this);

        // disconnect all signals
        disconnect(widget, nullptr, this, nullptr);

        // uninstall the shadow
        uninstallShadows(widget);
    }
}

bool ShadowHelper::eventFilter(QObject *object, QEvent *event)
{
    if (KWindowSystem::isPlatformX11()) {
        // check event type
        if (event->type() == QEvent::WinIdChange) {
            QWidget *widget = static_cast<QWidget *>(object);

            qreal frameRadius = m_frameRadius;
            const auto frameRadiusProperty = widget->property(netWMFrameRadius);
            if (frameRadiusProperty.isValid())
                frameRadius = frameRadiusProperty.toReal();
            TileSet shadowTileSet = shadowTiles(frameRadius);
            installShadows(widget, shadowTileSet);
        }
    } else {
        if (event->type() != QEvent::PlatformSurface)
            return false;

        // QWidget *widget(static_cast<QWidget *>(object));
        QPlatformSurfaceEvent* surfaceEvent(static_cast<QPlatformSurfaceEvent*>(event));

        switch (surfaceEvent->surfaceEventType()) {
            case QPlatformSurfaceEvent::SurfaceCreated:
                //installShadows(widget);
                break;
            case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                // Don't care.
                break;
        }
    }

    return false;
}

TileSet ShadowHelper::shadowTiles(const qreal frameRadius)
{
    const CompositeShadowParams params = lookupShadowParams(ShadowVeryLarge);

    if (params.isNone())
        return TileSet();
    // } else if (_shadowTiles.isValid()) {
    //     return _shadowTiles;
    // }

    auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
        QColor c(color);
        c.setAlphaF(opacity);
        return c;
    };

    const QColor color = Qt::black;
    // const qreal strength = static_cast<qreal>(255) / 255.0;
    const qreal strength = 1.5;

    const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
        .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

    const qreal dpr = qApp->devicePixelRatio();

    BoxShadowRenderer shadowRenderer;
    shadowRenderer.setBorderRadius(frameRadius);
    shadowRenderer.setBoxSize(boxSize);
    shadowRenderer.setDevicePixelRatio(dpr);

    shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius,
        withOpacity(color, params.shadow1.opacity * strength));
    shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius,
        withOpacity(color, params.shadow2.opacity * strength));

    QImage shadowTexture = shadowRenderer.render();

    const QRect outerRect(QPoint(0, 0), shadowTexture.size() / dpr);

    QRect boxRect(QPoint(0, 0), boxSize);
    boxRect.moveCenter(outerRect.center());

    // Mask out inner rect.
    QPainter painter(&shadowTexture);
    painter.setRenderHint(QPainter::Antialiasing);

    int Shadow_Overlap = 3;
    const QMargins margins = QMargins(
        boxRect.left() - outerRect.left() - Shadow_Overlap - params.offset.x(),
        boxRect.top() - outerRect.top() - Shadow_Overlap - params.offset.y(),
        outerRect.right() - boxRect.right() - Shadow_Overlap + params.offset.x(),
        outerRect.bottom() - boxRect.bottom() - Shadow_Overlap + params.offset.y());

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter.drawRoundedRect(
        outerRect - margins,
        frameRadius,
        frameRadius);

    // We're done.
    painter.end();

    const QPoint innerRectTopLeft = outerRect.center();
    TileSet tiles = TileSet(
        QPixmap::fromImage(shadowTexture),
        innerRectTopLeft.x(),
        innerRectTopLeft.y(),
        1, 1);

    return tiles;
}

void ShadowHelper::objectDeleted(QObject *object)
{
    QWidget *widget(static_cast<QWidget *>(object));
    m_widgets.remove(widget);
    m_shadows.remove(widget);
}

bool ShadowHelper::isMenu(QWidget *widget) const
{
    return qobject_cast<QMenu*>(widget);
}

bool ShadowHelper::isToolTip(QWidget *widget) const
{
    return widget->inherits("QTipLabel") || (widget->windowFlags() & Qt::WindowType_Mask) == Qt::ToolTip;
}

bool ShadowHelper::isDockWidget(QWidget *widget) const
{
    return qobject_cast<QDockWidget*>(widget);
}

bool ShadowHelper::isToolBar(QWidget *widget) const
{
    return qobject_cast<QToolBar*>(widget);
}

bool ShadowHelper::acceptWidget(QWidget *widget) const
{
    // flags
    if (widget->property(netWMSkipShadow).toBool())
        return false;
    if (widget->property(netWMForceShadow).toBool())
        return true;

    // menus
    if (isMenu(widget))
        return true;

    // combobox dropdown lists
    if (widget->inherits("QComboBoxPrivateContainer"))
        return true;

    // tooltips
    if (isToolTip(widget) && !widget->inherits("Plasma::ToolTip"))
        return true;

    // detached widgets
    if (isDockWidget(widget) || isToolBar(widget))
        return true;

    // reject
    return false;
}

KWindowShadowTile::Ptr ShadowHelper::createTile(const QPixmap& source)
{
    KWindowShadowTile::Ptr tile = KWindowShadowTile::Ptr::create();
    tile->setImage(source.toImage());
    return tile;
}

void ShadowHelper::installShadows(QWidget *widget, TileSet shadowTiles)
{
    if (!widget)
        return;

    // only toplevel widgets can cast drop-shadows
    if (!widget->isWindow())
        return;

    // widget must have valid native window
    if (!widget->testAttribute(Qt::WA_WState_Created))
        return;

    // create platform shadow tiles
    QVector<KWindowShadowTile::Ptr> tiles = {
        createTile(shadowTiles.pixmap(1)),
        createTile(shadowTiles.pixmap(2)),
        createTile(shadowTiles.pixmap(5)),
        createTile(shadowTiles.pixmap(8)),
        createTile(shadowTiles.pixmap(7)),
        createTile(shadowTiles.pixmap(6)),
        createTile(shadowTiles.pixmap(3)),
        createTile(shadowTiles.pixmap(0))
    };
    if (tiles.count() != numTiles)
        return;

    // find a shadow associated with the widget
    KWindowShadow*& shadow = m_shadows[ widget ];

    if (!shadow)
        shadow = new KWindowShadow(widget);

    if (shadow->isCreated())
        shadow->destroy();

    shadow->setTopTile(tiles[ 0 ]);
    shadow->setTopRightTile(tiles[ 1 ]);
    shadow->setRightTile(tiles[ 2 ]);
    shadow->setBottomRightTile(tiles[ 3 ]);
    shadow->setBottomTile(tiles[ 4 ]);
    shadow->setBottomLeftTile(tiles[ 5 ]);
    shadow->setLeftTile(tiles[ 6 ]);
    shadow->setTopLeftTile(tiles[ 7 ]);
    shadow->setPadding(shadowMargins(widget, shadowTiles));
    shadow->setWindow(widget->windowHandle());
    shadow->create();
}

QMargins ShadowHelper::shadowMargins(QWidget *widget, TileSet shadowTiles) const
{
    const CompositeShadowParams params = lookupShadowParams(ShadowVeryLarge);
    if (params.isNone())
        return QMargins();

    const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
        .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

    const QSize shadowSize = BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow1.radius, params.shadow1.offset)
        .expandedTo(BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow2.radius, params.shadow2.offset));

    const QRect shadowRect(QPoint(0, 0), shadowSize);

    QRect boxRect(QPoint(0, 0), boxSize);
    boxRect.moveCenter(shadowRect.center());

    int Shadow_Overlap = 4;
    QMargins margins(
        boxRect.left() - shadowRect.left() - Shadow_Overlap - params.offset.x(),
        boxRect.top() - shadowRect.top() - Shadow_Overlap - params.offset.y(),
        shadowRect.right() - boxRect.right() - Shadow_Overlap + params.offset.x(),
        shadowRect.bottom() - boxRect.bottom() - Shadow_Overlap + params.offset.y());

    if (widget->inherits("QBalloonTip")) {
        // Balloon tip needs special margins to deal with the arrow.
        int top = widget->contentsMargins().top();
        int bottom = widget->contentsMargins().bottom();

        // Need to decrement default size further due to extra hard coded round corner.
        margins -= 1;

        // Arrow can be either to the top or the bottom. Adjust margins accordingly.
        const int diff = qAbs(top - bottom);
        if (top > bottom) {
            margins.setTop(margins.top() - diff);
        } else {
            margins.setBottom(margins.bottom() - diff);
        }
    }

    margins *= shadowTiles.pixmap(0).devicePixelRatio();

    return margins;
}

void ShadowHelper::uninstallShadows(QWidget *widget)
{
    delete m_shadows.take(widget);
}
