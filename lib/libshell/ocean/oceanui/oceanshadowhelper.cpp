/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 * Copyright (C) 2018, 2020 by Vlad Zahorodnii <vlad.zahorodnii@kde.org> *
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

#include "oceanshadowhelper.h"

#include "ocean.h"
#include "oceanboxshadowrenderer.h"
#include "oceanhelper.h"
#include "oceanpropertynames.h"
#include "oceanstyleconfigdata.h"

#include <QDockWidget>
#include <QEvent>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPlatformSurfaceEvent>
#include <QToolBar>
#include <QTextStream>

namespace
{
    using Ocean::CompositeShadowParams;
    using Ocean::ShadowParams;

    const CompositeShadowParams s_shadowParams[] = {
        // None
        CompositeShadowParams(),
        // Small
        CompositeShadowParams(
            QPoint(0, 8),
            ShadowParams(QPoint(0, 0), 8, 0.8),
            ShadowParams(QPoint(0, -4), 4, 0.16),
            ShadowParams(QPoint(0, -6), 2, 0.12)),
        // Medium
        CompositeShadowParams(
            QPoint(0, 8),
            ShadowParams(QPoint(0, 0), 20, 0.24),
            ShadowParams(QPoint(0, -4), 8, 0.32),
            ShadowParams(QPoint(0, -6), 4, 0.01)),
        // Large
        CompositeShadowParams(
            QPoint(0, 16),
            ShadowParams(QPoint(0, 0), 28, 0.20),
            ShadowParams(QPoint(0, -8), 16, 0.24),
            ShadowParams(QPoint(0, -13), 6, 0.16)),
        // Very Large
        CompositeShadowParams(
            QPoint(0, 32),
            ShadowParams(QPoint(0, 0), 40, 0.12),
            ShadowParams(QPoint(0, -16), 20, 0.20),
            ShadowParams(QPoint(0, -27), 5, 0.24))
    };
}

namespace Ocean
{

    //_____________________________________________________
    CompositeShadowParams ShadowHelper::lookupShadowParams(int shadowSizeEnum)
    {

        switch (shadowSizeEnum) {
        case StyleConfigData::ShadowNone:
            return s_shadowParams[0];
        case StyleConfigData::ShadowSmall:
            return s_shadowParams[1];
        case StyleConfigData::ShadowMedium:
            return s_shadowParams[2];
        case StyleConfigData::ShadowLarge:
            return s_shadowParams[3];
        case StyleConfigData::ShadowVeryLarge:
            return s_shadowParams[4];
        default:
            // Fallback to the Large size.
            return s_shadowParams[3];
        }

    }

    //_____________________________________________________
    ShadowHelper::ShadowHelper( QObject* parent, Helper& helper ):
        QObject( parent ),
        _helper( helper )
    {
    }

    //_______________________________________________________
    ShadowHelper::~ShadowHelper()
    {
        qDeleteAll( _shadows );
    }

    //______________________________________________
    void ShadowHelper::reset()
    {
        _tiles.clear();
        _shadowTiles = TileSet();

    }

    //_______________________________________________________
    bool ShadowHelper::registerWidget( QWidget* widget, bool force )
    {
        // make sure widget is not already registered
        if( _widgets.contains( widget ) ) return false;

        // check if widget qualifies
        if( !( force || acceptWidget( widget ) ) )
        { return false; }

        // try create shadow directly
        installShadows( widget );
        _widgets.insert( widget );

        // install event filter
        widget->removeEventFilter( this );
        widget->installEventFilter( this );

        // connect destroy signal
        connect( widget, &QObject::destroyed, this, &ShadowHelper::widgetDeleted );

        return true;

    }

    //_______________________________________________________
    void ShadowHelper::unregisterWidget( QWidget* widget )
    {
        if( _widgets.remove( widget ) )
        {
            // uninstall the event filter
            widget->removeEventFilter( this );

            // disconnect all signals
            disconnect( widget, nullptr, this, nullptr );

            // uninstall the shadow
            uninstallShadows( widget );
        }
    }

    //_______________________________________________________
    void ShadowHelper::loadConfig()
    {

        // reset
        reset();

        // update property for registered widgets
        for( QWidget* widget : _widgets)
        { installShadows( widget ); }

    }

    //_______________________________________________________
    bool ShadowHelper::eventFilter( QObject* object, QEvent* event )
    {
        if( Helper::isX11() )
        {
            // check event type
            if( event->type() != QEvent::WinIdChange ) return false;

            // cast widget
            QWidget* widget( static_cast<QWidget*>( object ) );

            // install shadows and update winId
            installShadows( widget );

        } else {
            if( event->type() != QEvent::PlatformSurface ) return false;

            QWidget* widget( static_cast<QWidget*>( object ) );
            QPlatformSurfaceEvent* surfaceEvent( static_cast<QPlatformSurfaceEvent*>( event ) );

            switch( surfaceEvent->surfaceEventType() )
            {
                case QPlatformSurfaceEvent::SurfaceCreated:
                    installShadows( widget );
                    break;
                case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                    // Don't care.
                    break;
            }
        }

        return false;

    }

    //_______________________________________________________
    TileSet ShadowHelper::shadowTiles()
    {
        const CompositeShadowParams params = lookupShadowParams(StyleConfigData::shadowSize());

        if (params.isNone()) {
            return TileSet();
        } else if (_shadowTiles.isValid()) {
            return _shadowTiles;
        }

        auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        const QColor color = StyleConfigData::shadowColor();
        const qreal strength = static_cast<qreal>(StyleConfigData::shadowStrength()) / 255.0;

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius))
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow3.radius));

        const qreal dpr = qApp->devicePixelRatio();
        const qreal frameRadius = _helper.frameRadius(1);

        BoxShadowRenderer shadowRenderer;
        shadowRenderer.setBorderRadius(frameRadius);
        shadowRenderer.setBoxSize(boxSize);
        shadowRenderer.setDevicePixelRatio(dpr);

        shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius,
            withOpacity(color, params.shadow1.opacity * strength));
        shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius,
            withOpacity(color, params.shadow2.opacity * strength));
        shadowRenderer.addShadow(params.shadow3.offset, params.shadow3.radius,
            withOpacity(color, params.shadow3.opacity * strength));

        QImage shadowTexture = shadowRenderer.render();

        const QRect outerRect(QPoint(0, 0), shadowTexture.size() / dpr);

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(outerRect.center());

        // Mask out inner rect.
        QPainter painter(&shadowTexture);
        painter.setRenderHint(QPainter::Antialiasing);

        const QMargins margins = QMargins(
            boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
            boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
            outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
            outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRoundedRect(
            outerRect - margins,
            frameRadius,
            frameRadius);
        
        // Draw outline.
        painter.setPen(withOpacity(Qt::black, 0.1 * strength));
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawRoundedRect(
            outerRect - margins,
            frameRadius-1,
            frameRadius-1);

        // We're done.
        painter.end();

        const QPoint innerRectTopLeft = outerRect.center();
        _shadowTiles = TileSet(
            QPixmap::fromImage(shadowTexture),
            innerRectTopLeft.x(),
            innerRectTopLeft.y(),
            1, 1);

        return _shadowTiles;
    }
    
    //_______________________________________________________
    TileSet ShadowHelper::shadowTiles( const int frameRadius, CustomShadowParams shadow1, CustomShadowParams shadow2 )
    {

        if (shadow1.radius == 0) {
            return TileSet();
        } 

        auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        //const QColor color = StyleConfigData::shadowColor();
        //const qreal strength = static_cast<qreal>(StyleConfigData::shadowStrength()) / 255.0;

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(shadow2.radius));

        const qreal dpr = qApp->devicePixelRatio();

        BoxShadowRenderer shadowRenderer;
        shadowRenderer.setBorderRadius(frameRadius);
        shadowRenderer.setBoxSize(boxSize);
        shadowRenderer.setDevicePixelRatio(dpr);

        shadowRenderer.addShadow(shadow1.offset, shadow1.radius, shadow1.color);
        if (shadow2.radius > 0) 
            shadowRenderer.addShadow(shadow2.offset, shadow2.radius, shadow2.color);

        QImage shadowTexture = shadowRenderer.render();

        const QRect outerRect(QPoint(0, 0), shadowTexture.size() / dpr);

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(outerRect.center());

        // Mask out inner rect.
        if( qMax(shadow1.radius, shadow2.radius) > 3 && frameRadius > 3) {
            QPainter painter(&shadowTexture);
            painter.setRenderHint(QPainter::Antialiasing);

            const QMargins margins = QMargins(
                boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap,
                boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap,
                outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap,
                outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap);

            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            painter.drawRoundedRect(
                outerRect - margins,
                frameRadius,
                frameRadius);
            
            // Draw outline.
            /*painter.setPen(withOpacity(Qt::black, 0.3));
            painter.setBrush(Qt::NoBrush);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawRoundedRect(
                outerRect - margins,
                frameRadius-1,
                frameRadius-1);*/

            // We're done.
            painter.end();
        }

        const QPoint innerRectTopLeft = outerRect.center();
        TileSet shadowTiles = TileSet(
            QPixmap::fromImage(shadowTexture),
            innerRectTopLeft.x(),
            innerRectTopLeft.y(),
            1, 1);

        return shadowTiles;
    }


    //_______________________________________________________
    void ShadowHelper::widgetDeleted( QObject* object )
    {
        QWidget* widget( static_cast<QWidget*>( object ) );
        _widgets.remove( widget );
    }

    //_______________________________________________________
    void ShadowHelper::windowDeleted( QObject* object )
    {
        QWindow* window( static_cast<QWindow*>( object ) );
        _shadows.remove( window );
    }

    //_______________________________________________________
    bool ShadowHelper::isMenu( QWidget* widget ) const
    { return qobject_cast<QMenu*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::isToolTip( QWidget* widget ) const
    { return widget->inherits( "QTipLabel" ) || (widget->windowFlags() & Qt::WindowType_Mask) == Qt::ToolTip; }

    //_______________________________________________________
    bool ShadowHelper::isDockWidget( QWidget* widget ) const
    { return qobject_cast<QDockWidget*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::isToolBar( QWidget* widget ) const
    { return qobject_cast<QToolBar*>( widget ); }

    //_______________________________________________________
    bool ShadowHelper::acceptWidget( QWidget* widget ) const
    {

        // flags
        if( widget->property( PropertyNames::netWMSkipShadow ).toBool() ) return false;
        if( widget->property( PropertyNames::netWMForceShadow ).toBool() ) return true;

        // menus
        if( isMenu( widget ) ) return true;

        // combobox dropdown lists
        if( widget->inherits( "QComboBoxPrivateContainer" ) ) return true;

        // tooltips
        if( isToolTip( widget ) && !widget->inherits( "Plasma::ToolTip" ) )
        { return true; }

        // detached widgets
        if( isDockWidget( widget ) || isToolBar( widget ) )
        { return true; }

        // reject
        return false;
    }

    //______________________________________________
    const QVector<KWindowShadowTile::Ptr>& ShadowHelper::createShadowTiles()
    {

        // make sure size is valid
        if( _tiles.isEmpty() )
        {
            _tiles = {
                createTile( _shadowTiles.pixmap( 1 ) ),
                createTile( _shadowTiles.pixmap( 2 ) ),
                createTile( _shadowTiles.pixmap( 5 ) ),
                createTile( _shadowTiles.pixmap( 8 ) ),
                createTile( _shadowTiles.pixmap( 7 ) ),
                createTile( _shadowTiles.pixmap( 6 ) ),
                createTile( _shadowTiles.pixmap( 3 ) ),
                createTile( _shadowTiles.pixmap( 0 ) )
            };
        }

        // return relevant list of shadow tiles
        return _tiles;

    }

    //______________________________________________
    KWindowShadowTile::Ptr ShadowHelper::createTile( const QPixmap& source )
    {

        KWindowShadowTile::Ptr tile = KWindowShadowTile::Ptr::create();
        tile->setImage( source.toImage() );
        return tile;

    }

    //_______________________________________________________
    void ShadowHelper::installShadows( QWidget* widget )
    {
        if( !widget ) return;

        // only toplevel widgets can cast drop-shadows
        if( !widget->isWindow() ) return;

        // widget must have valid native window
        if( !widget->testAttribute( Qt::WA_WState_Created ) ) return;

        // create shadow tiles if needed
        shadowTiles();
        if( !_shadowTiles.isValid() ) return;

        // create platform shadow tiles if needed
        const QVector<KWindowShadowTile::Ptr>& tiles = createShadowTiles();
        if( tiles.count() != numTiles ) return;

        // get the underlying window for the widget
        QWindow* window = widget->windowHandle();

        // find a shadow associated with the widget
        KWindowShadow*& shadow = _shadows[ window ];

        if( !shadow )
        {
            // if there is no shadow yet, create one
            shadow = new KWindowShadow( window );

            // connect destroy signal
            connect( window, &QWindow::destroyed, this, &ShadowHelper::windowDeleted );
        }

        if( shadow->isCreated() )
        { shadow->destroy(); }

        shadow->setTopTile( tiles[ 0 ] );
        shadow->setTopRightTile( tiles[ 1 ] );
        shadow->setRightTile( tiles[ 2 ] );
        shadow->setBottomRightTile( tiles[ 3 ] );
        shadow->setBottomTile( tiles[ 4 ] );
        shadow->setBottomLeftTile( tiles[ 5 ] );
        shadow->setLeftTile( tiles[ 6 ] );
        shadow->setTopLeftTile( tiles[ 7 ] );
        shadow->setPadding( shadowMargins( widget ) );
        shadow->setWindow( window );
        shadow->create();
    }

    //_______________________________________________________
    QMargins ShadowHelper::shadowMargins( QWidget* widget ) const
    {
        const CompositeShadowParams params = lookupShadowParams(StyleConfigData::shadowSize());
        if (params.isNone()) {
            return QMargins();
        }

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

        const QSize shadowSize = BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow1.radius, params.shadow1.offset)
            .expandedTo(BoxShadowRenderer::calculateMinimumShadowTextureSize(boxSize, params.shadow2.radius, params.shadow2.offset));

        const QRect shadowRect(QPoint(0, 0), shadowSize);

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(shadowRect.center());

        QMargins margins(
            boxRect.left() - shadowRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
            boxRect.top() - shadowRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
            shadowRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
            shadowRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());

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

        margins *= _helper.devicePixelRatio(_shadowTiles.pixmap(0));

        return margins;
    }

    //_______________________________________________________
    void ShadowHelper::uninstallShadows( QWidget* widget )
    {
        delete _shadows.take( widget->windowHandle() );
    }

}
