/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014-2015 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2017-2018 Adriaan de Groot <groot@kde.org>
 *   SPDX-FileCopyrightText: 2018 Raul Rodrigo Segura (raurodse)
 *   SPDX-FileCopyrightText: 2019 Collabora Ltd <arnaud.ferraris@collabora.com>
 *   SPDX-FileCopyrightText: 2020 Anubhav Choudhary <ac.10edu@gmail.com>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "CalamaresWindow.h"

#include "Branding.h"
#include "CalamaresConfig.h"
#include "DebugWindow.h"
#include "Settings.h"
#include "ViewManager.h"
#include "progresstree/ProgressTreeView.h"
#include "utils/Gui.h"
#include "utils/Logger.h"
#include "utils/Qml.h"
#include "utils/Retranslator.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <QDesktopWidget>
#endif
#include <QFile>
#include <QFileInfo>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#ifdef WITH_QML
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#endif
#include <QTreeView>

static QSize
desktopSize( QWidget* w )
{
    Q_UNUSED( w )
    return qApp->primaryScreen()->geometry().size();
}

/** @brief Expected orientation of the panels, based on their side
 *
 * Panels on the left and right are expected to be "vertical" style,
 * top and bottom should be "horizontal bars". This function maps
 * the sides to expected orientation.
 */
static inline Qt::Orientation
orientation( const Calamares::Branding::PanelSide s )
{
    using Side = Calamares::Branding::PanelSide;
    return ( s == Side::Left || s == Side::Right ) ? Qt::Orientation::Vertical : Qt::Orientation::Horizontal;
}

/** @brief Get a button-sized icon. */
static inline QPixmap
getButtonIcon( const QString& name )
{
    return Calamares::Branding::instance()->image( name, QSize( 22, 22 ) );
}

static inline void
setButtonIcon( QPushButton* button, const QString& name )
{
    auto icon = getButtonIcon( name );
    if ( button && !icon.isNull() )
    {
        button->setIcon( icon );
    }
}

static QWidget*
getWidgetSidebar( Calamares::DebugWindowManager* debug,
                  Calamares::ViewManager* viewManager,
                  QWidget* parent,
                  Qt::Orientation,
                  int desiredWidth )
{
    const Calamares::Branding* const branding = Calamares::Branding::instance();

    QWidget* sideBox = new QWidget( parent );
    sideBox->setObjectName( "sidebarApp" );

    QBoxLayout* sideLayout = new QVBoxLayout;
    sideBox->setLayout( sideLayout );
    // Set this attribute into qss file
    sideBox->setFixedWidth( desiredWidth );
    sideBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QHBoxLayout* logoLayout = new QHBoxLayout;
    sideLayout->addLayout( logoLayout );
    logoLayout->addStretch();
    QLabel* logoLabel = new QLabel( sideBox );
    logoLabel->setObjectName( "logoApp" );
    //Define all values into qss file
    {
        QPalette plt = sideBox->palette();
        sideBox->setAutoFillBackground( true );
        plt.setColor( sideBox->backgroundRole(), branding->styleString( Calamares::Branding::SidebarBackground ) );
        plt.setColor( sideBox->foregroundRole(), branding->styleString( Calamares::Branding::SidebarText ) );
        sideBox->setPalette( plt );
        logoLabel->setPalette( plt );
    }
    logoLabel->setAlignment( Qt::AlignCenter );
    logoLabel->setFixedSize( 80, 80 );
    logoLabel->setPixmap( branding->image( Calamares::Branding::ProductLogo, logoLabel->size() ) );
    logoLayout->addWidget( logoLabel );
    logoLayout->addStretch();

    ProgressTreeView* tv = new ProgressTreeView( sideBox );
    tv->setModel( viewManager );
    tv->setFocusPolicy( Qt::NoFocus );
    sideLayout->addWidget( tv );

    QHBoxLayout* extraButtons = new QHBoxLayout;
    sideLayout->addLayout( extraButtons );

    const int defaultFontHeight = Calamares::defaultFontHeight();

    if ( /* About-Calamares Button enabled */ true )
    {
        QPushButton* aboutDialog = new QPushButton;
        aboutDialog->setObjectName( "aboutButton" );
        aboutDialog->setIcon( Calamares::defaultPixmap(
            Calamares::Information, Calamares::Original, 2 * QSize( defaultFontHeight, defaultFontHeight ) ) );
        CALAMARES_RETRANSLATE_FOR(
            aboutDialog, aboutDialog->setText( QCoreApplication::translate( "calamares-sidebar", "About", "@button" ) );
            aboutDialog->setToolTip(
                QCoreApplication::translate( "calamares-sidebar", "Show information about Calamares", "@tooltip" ) ); );
        extraButtons->addWidget( aboutDialog );
        aboutDialog->setFlat( true );
        aboutDialog->setCheckable( true );
        QObject::connect( aboutDialog, &QPushButton::clicked, debug, &Calamares::DebugWindowManager::about );
    }
    if ( debug && debug->enabled() )
    {
        QPushButton* debugWindowBtn = new QPushButton;
        debugWindowBtn->setObjectName( "debugButton" );
        debugWindowBtn->setIcon( Calamares::defaultPixmap(
            Calamares::Bugs, Calamares::Original, 2 * QSize( defaultFontHeight, defaultFontHeight ) ) );
        CALAMARES_RETRANSLATE_FOR(
            debugWindowBtn,
            debugWindowBtn->setText( QCoreApplication::translate( "calamares-sidebar", "Debug", "@button" ) );
            debugWindowBtn->setToolTip(
                QCoreApplication::translate( "calamares-sidebar", "Show debug information", "@tooltip" ) ); );
        extraButtons->addWidget( debugWindowBtn );
        debugWindowBtn->setFlat( true );
        debugWindowBtn->setCheckable( true );
        QObject::connect( debugWindowBtn, &QPushButton::clicked, debug, &Calamares::DebugWindowManager::show );
        QObject::connect(
            debug, &Calamares::DebugWindowManager::visibleChanged, debugWindowBtn, &QPushButton::setChecked );
    }

    Calamares::unmarginLayout( sideLayout );
    return sideBox;
}

static QWidget*
getWidgetNavigation( Calamares::DebugWindowManager*,
                     Calamares::ViewManager* viewManager,
                     QWidget* parent,
                     Qt::Orientation,
                     int )
{
    QWidget* navigation = new QWidget( parent );
    QBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();

    // Create buttons and sets an initial icon; the icons may change
    {
        auto* back = new QPushButton(
            getButtonIcon( QStringLiteral( "go-previous" ) ),
            QCoreApplication::translate( CalamaresWindow::staticMetaObject.className(), "&Back", "@button" ),
            navigation );
        back->setObjectName( "view-button-back" );
        back->setEnabled( viewManager->backEnabled() );
        QObject::connect( back, &QPushButton::clicked, viewManager, &Calamares::ViewManager::back );
        QObject::connect( viewManager, &Calamares::ViewManager::backEnabledChanged, back, &QPushButton::setEnabled );
        QObject::connect( viewManager, &Calamares::ViewManager::backLabelChanged, back, &QPushButton::setText );
        QObject::connect(
            viewManager, &Calamares::ViewManager::backIconChanged, [ = ]( QString n ) { setButtonIcon( back, n ); } );
        QObject::connect(
            viewManager, &Calamares::ViewManager::backAndNextVisibleChanged, back, &QPushButton::setVisible );
        bottomLayout->addWidget( back );
    }
    {
        auto* next = new QPushButton(
            getButtonIcon( QStringLiteral( "go-next" ) ),
            QCoreApplication::translate( CalamaresWindow::staticMetaObject.className(), "&Next", "@button" ),
            navigation );
        next->setObjectName( "view-button-next" );
        next->setEnabled( viewManager->nextEnabled() );
        QObject::connect( next, &QPushButton::clicked, viewManager, &Calamares::ViewManager::next );
        QObject::connect( viewManager, &Calamares::ViewManager::nextEnabledChanged, next, &QPushButton::setEnabled );
        QObject::connect( viewManager, &Calamares::ViewManager::nextLabelChanged, next, &QPushButton::setText );
        QObject::connect(
            viewManager, &Calamares::ViewManager::nextIconChanged, [ = ]( QString n ) { setButtonIcon( next, n ); } );
        QObject::connect(
            viewManager, &Calamares::ViewManager::backAndNextVisibleChanged, next, &QPushButton::setVisible );
        bottomLayout->addWidget( next );
    }
    bottomLayout->addSpacing( 12 );
    {
        auto* quit = new QPushButton(
            getButtonIcon( QStringLiteral( "dialog-cancel" ) ),
            QCoreApplication::translate( CalamaresWindow::staticMetaObject.className(), "&Cancel", "@button" ),
            navigation );
        quit->setObjectName( "view-button-cancel" );
        QObject::connect( quit, &QPushButton::clicked, viewManager, &Calamares::ViewManager::quit );
        QObject::connect( viewManager, &Calamares::ViewManager::quitEnabledChanged, quit, &QPushButton::setEnabled );
        QObject::connect( viewManager, &Calamares::ViewManager::quitLabelChanged, quit, &QPushButton::setText );
        QObject::connect(
            viewManager, &Calamares::ViewManager::quitIconChanged, [ = ]( QString n ) { setButtonIcon( quit, n ); } );
        QObject::connect( viewManager, &Calamares::ViewManager::quitTooltipChanged, quit, &QPushButton::setToolTip );
        QObject::connect( viewManager, &Calamares::ViewManager::quitVisibleChanged, quit, &QPushButton::setVisible );
        bottomLayout->addWidget( quit );
    }

    bottomLayout->setContentsMargins( 0, 0, 6, 6 );
    navigation->setLayout( bottomLayout );
    return navigation;
}

#ifdef WITH_QML

static inline void
setDimension( QQuickWidget* w, Qt::Orientation o, int desiredWidth )
{
    w->setSizePolicy( o == Qt::Orientation::Vertical ? QSizePolicy::MinimumExpanding : QSizePolicy::Expanding,
                      o == Qt::Orientation::Horizontal ? QSizePolicy::MinimumExpanding : QSizePolicy::Expanding );
    if ( o == Qt::Orientation::Vertical )
    {
        w->setFixedWidth( desiredWidth );
    }
    else
    {
        // If the QML itself sets a height, use that, otherwise go to 48 pixels
        // which seems to match what the widget navigation would use for height
        // (with *my* specific screen, style, etc. so YMMV).
        //
        // Bound between (16, 64) with a default of 48.
        qreal minimumHeight = qBound( qreal( 16 ), w->rootObject() ? w->rootObject()->height() : 48, qreal( 64 ) );
        w->setMinimumHeight( int( minimumHeight ) );
        w->setFixedHeight( int( minimumHeight ) );
    }
    w->setResizeMode( QQuickWidget::SizeRootObjectToView );
}

static QWidget*
getQmlSidebar( Calamares::DebugWindowManager* debug,
               Calamares::ViewManager*,
               QWidget* parent,
               Qt::Orientation o,
               int desiredWidth )
{
    Calamares::registerQmlModels();
    QQuickWidget* w = new QQuickWidget( parent );
    if ( debug )
    {
        w->engine()->rootContext()->setContextProperty( "debug", debug );
    }

    w->setSource(
        QUrl( Calamares::searchQmlFile( Calamares::QmlSearch::Both, QStringLiteral( "calamares-sidebar" ) ) ) );
    setDimension( w, o, desiredWidth );
    return w;
}

static QWidget*
getQmlNavigation( Calamares::DebugWindowManager* debug,
                  Calamares::ViewManager*,
                  QWidget* parent,
                  Qt::Orientation o,
                  int desiredWidth )
{
    Calamares::registerQmlModels();
    QQuickWidget* w = new QQuickWidget( parent );
    if ( debug )
    {
        w->engine()->rootContext()->setContextProperty( "debug", debug );
    }
    w->setSource(
        QUrl( Calamares::searchQmlFile( Calamares::QmlSearch::Both, QStringLiteral( "calamares-navigation" ) ) ) );
    setDimension( w, o, desiredWidth );
    return w;
}
#else
// Bogus to keep the linker happy
//
// Calls to flavoredWidget() still refer to these *names*
// even if they are subsequently not used.
static QWidget*
getQmlSidebar( Calamares::DebugWindowManager*,
               Calamares::ViewManager*,
               QWidget* parent,
               Qt::Orientation,
               int desiredWidth )
{
    return nullptr;
}
static QWidget*
getQmlNavigation( Calamares::DebugWindowManager*,
                  Calamares::ViewManager*,
                  QWidget* parent,
                  Qt::Orientation,
                  int desiredWidth )
{
    return nullptr;
}
#endif

/**@brief Picks one of two methods to call
 *
 * Calls method (member function) @p widget or @p qml with arguments @p a
 * on the given window, based on the flavor.
 */
template < typename widgetMaker, typename... args >
QWidget*
flavoredWidget( Calamares::Branding::PanelFlavor flavor,
                Qt::Orientation o,
                Calamares::DebugWindowManager* w,
                QWidget* parent,
                widgetMaker widget,
                widgetMaker qml,  // Only if WITH_QML is on
                args... a )
{
#ifndef WITH_QML
    Q_UNUSED( qml )
#endif
    auto* viewManager = Calamares::ViewManager::instance();
    switch ( flavor )
    {
    case Calamares::Branding::PanelFlavor::Widget:
        return widget( w, viewManager, parent, o, a... );
#ifdef WITH_QML
    case Calamares::Branding::PanelFlavor::Qml:
        return qml( w, viewManager, parent, o, a... );
#endif
    case Calamares::Branding::PanelFlavor::None:
        return nullptr;
    }
    __builtin_unreachable();
}

CalamaresWindow::CalamaresWindow( QWidget* parent )
    : QWidget( parent )
    , m_debugManager( new Calamares::DebugWindowManager( this ) )
    , m_viewManager( nullptr )
{
    installEventFilter( Calamares::Retranslator::instance() );

    // %1 is the distribution name
    CALAMARES_RETRANSLATE( const auto* branding = Calamares::Branding::instance();
                           setWindowTitle( Calamares::Settings::instance()->isSetupMode()
                                               ? tr( "%1 Setup Program" ).arg( branding->productName() )
                                               : tr( "%1 Installer" ).arg( branding->productName() ) ); );

    const Calamares::Branding* const branding = Calamares::Branding::instance();
    using ImageEntry = Calamares::Branding::ImageEntry;

    this->setObjectName( "mainApp" );

    QSize availableSize = desktopSize( this );
    cDebug() << "Available desktop" << availableSize;

    // Fullscreen frameless window
    setWindowFlags( Qt::FramelessWindowHint | Qt::Window );
    setGeometry( QRect( QPoint( 0, 0 ), availableSize ) );

    // Blurred background
    setupBlurredBackground();

    // View manager (parent = this, centralWidget will be reparented)
    m_viewManager = Calamares::ViewManager::instance( this );
    if ( branding->windowExpands() )
    {
        connect( m_viewManager, &Calamares::ViewManager::ensureSize, this, &CalamaresWindow::ensureSize );
    }

    // Sidebar
    QWidget* sideBox
        = flavoredWidget( branding->sidebarFlavor(),
                          ::orientation( branding->sidebarSide() ),
                          m_debugManager,
                          this,
                          ::getWidgetSidebar,
                          ::getQmlSidebar,
                          qBound( 100,
                                  Calamares::defaultFontHeight() * 12,
                                  availableSize.width() < 800 ? 100 : 190 ) );

    // Navigation bar
    QWidget* navigation = flavoredWidget( branding->navigationFlavor(),
                                          ::orientation( branding->navigationSide() ),
                                          m_debugManager,
                                          this,
                                          ::getWidgetNavigation,
                                          ::getQmlNavigation,
                                          64 );

    // Center card with rounded corners
    QWidget* card = createCardWidget( m_viewManager->centralWidget(), sideBox, navigation );

    int cardW = qMin( static_cast<int>(availableSize.width() * 0.60), availableSize.width() - 100 );
    int cardH = qBound( 480, card->minimumSizeHint().height(), static_cast<int>(availableSize.height() * 0.78) );
    int cardX = ( availableSize.width() - cardW ) / 2;
    int cardY = ( availableSize.height() - cardH ) / 2;
    card->setGeometry( cardX, cardY, cardW, cardH );
    card->show();

    // Forward panel side info to view manager for content margins
    m_viewManager->setPanelSides( Qt::Orientations( Qt::Horizontal ) );

    QString qss = Calamares::Branding::instance()->stylesheet();
    if ( !qss.isEmpty() )
    {
        setStyleSheet( qss );
    }
}

void
CalamaresWindow::setupBlurredBackground()
{
    const auto* branding = Calamares::Branding::instance();
    using ImageEntry = Calamares::Branding::ImageEntry;

    QSize screenSize = desktopSize( this );

    m_backgroundLabel = new QLabel( this );
    m_backgroundLabel->setGeometry( QRect( QPoint( 0, 0 ), screenSize ) );
    m_backgroundLabel->setObjectName( "installerBackground" );

    QString wallpaperPath = branding->imagePath( ImageEntry::ProductWallpaper );
    if ( !wallpaperPath.isEmpty() )
    {
        QPixmap bgPix( wallpaperPath );
        if ( !bgPix.isNull() )
        {
            bgPix = bgPix.scaled( screenSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
            // Crop to exact screen size
            int x = ( bgPix.width() - screenSize.width() ) / 2;
            int y = ( bgPix.height() - screenSize.height() ) / 2;
            bgPix = bgPix.copy( qMax( 0, x ), qMax( 0, y ), screenSize.width(), screenSize.height() );

            m_backgroundLabel->setPixmap( bgPix );

            auto* blur = new QGraphicsBlurEffect( m_backgroundLabel );
            blur->setBlurRadius( 40.0 );
            blur->setBlurHints( QGraphicsBlurEffect::QualityHint );
            m_backgroundLabel->setGraphicsEffect( blur );
        }
        else
        {
            m_backgroundLabel->setStyleSheet( "background-color: #1a1a2e;" );
        }
    }
    else
    {
        // Dark gradient fallback
        QPixmap gradientPix( screenSize );
        QPainter p( &gradientPix );
        QLinearGradient g( 0, 0, 0, screenSize.height() );
        g.setColorAt( 0.0, QColor( "#0f0c29" ) );
        g.setColorAt( 0.5, QColor( "#302b63" ) );
        g.setColorAt( 1.0, QColor( "#24243e" ) );
        p.fillRect( QRect( QPoint( 0, 0 ), screenSize ), g );
        p.end();
        m_backgroundLabel->setPixmap( gradientPix );
    }

    // Semi-transparent overlay for readability
    QLabel* overlay = new QLabel( this );
    overlay->setGeometry( QRect( QPoint( 0, 0 ), screenSize ) );
    overlay->setStyleSheet( "background: rgba(0, 0, 0, 0.25);" );
    overlay->lower();
    m_backgroundLabel->lower();
}

QWidget*
CalamaresWindow::createCardWidget( QWidget* content, QWidget* sidebar, QWidget* navigation )
{
    m_cardWidget = new QWidget( this );
    m_cardWidget->setObjectName( "installerCard" );

    QVBoxLayout* outerLayout = new QVBoxLayout( m_cardWidget );
    outerLayout->setContentsMargins( 0, 0, 0, 0 );
    outerLayout->setSpacing( 0 );

    // Header bar: logo + product name
    {
        QWidget* header = new QWidget( m_cardWidget );
        header->setObjectName( "cardHeader" );
        QHBoxLayout* headerLayout = new QHBoxLayout( header );
        headerLayout->setContentsMargins( 20, 16, 20, 8 );

        const auto* branding = Calamares::Branding::instance();
        QLabel* logoLabel = new QLabel( header );
        QPixmap logo = branding->image( Calamares::Branding::ProductLogo, QSize( 32, 32 ) );
        if ( !logo.isNull() )
        {
            logoLabel->setPixmap( logo );
        }
        headerLayout->addWidget( logoLabel );

        QLabel* titleLabel = new QLabel( branding->shortVersionedName(), header );
        titleLabel->setObjectName( "cardTitle" );
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize( titleFont.pointSize() + 2 );
        titleFont.setBold( true );
        titleLabel->setFont( titleFont );
        headerLayout->addWidget( titleLabel );

        headerLayout->addStretch();
        outerLayout->addWidget( header );
    }

    // Content row: sidebar + pages
    {
        QHBoxLayout* contentRow = new QHBoxLayout;
        contentRow->setContentsMargins( 0, 0, 0, 0 );
        contentRow->setSpacing( 0 );

        if ( sidebar )
        {
            sidebar->setParent( m_cardWidget );
            contentRow->addWidget( sidebar );
        }

        content->setParent( m_cardWidget );
        contentRow->addWidget( content, 1 );

        outerLayout->addLayout( contentRow, 1 );
    }

    // Bottom navigation
    if ( navigation )
    {
        navigation->setParent( m_cardWidget );
        outerLayout->addWidget( navigation );
    }

    // Card stylesheet: white rounded rect
    m_cardWidget->setStyleSheet(
        "#installerCard {"
        "  background: rgba(255, 255, 255, 0.92);"
        "  border-radius: 12px;"
        "}"
        "#cardHeader {"
        "  background: transparent;"
        "  border-bottom: 1px solid rgba(0, 0, 0, 0.08);"
        "}"
        "#cardTitle {"
        "  color: #1a1a1a;"
        "  background: transparent;"
        "}" );

    // Card drop shadow
    m_cardWidget->setGraphicsEffect( nullptr );
    m_cardWidget->setAutoFillBackground( false );
    m_cardWidget->setAttribute( Qt::WA_TranslucentBackground, false );

    return m_cardWidget;
}

void
CalamaresWindow::ensureSize( QSize size )
{
    if ( !m_cardWidget || !m_viewManager )
    {
        return;
    }
    QSize availableSize = desktopSize( this );

    int embiggenment = qMax( 0, size.height() - m_viewManager->centralWidget()->size().height() );
    if ( embiggenment < 6 )
    {
        return;
    }

    int newCardH = qMin( m_cardWidget->height() + embiggenment, availableSize.height() - 80 );
    int newCardY = ( availableSize.height() - newCardH ) / 2;
    m_cardWidget->setGeometry( m_cardWidget->x(), newCardY, m_cardWidget->width(), newCardH );
}

void
CalamaresWindow::closeEvent( QCloseEvent* event )
{
    if ( m_viewManager )
    {
        m_viewManager->quit();
        event->ignore();
    }
    else
    {
        event->accept();
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
        QApplication::quit();
#else
        QApplication::exit( EXIT_SUCCESS );
#endif
    }
}
