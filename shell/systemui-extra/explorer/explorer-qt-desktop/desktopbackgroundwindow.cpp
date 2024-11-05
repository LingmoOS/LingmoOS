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

#include "desktopbackgroundwindow.h"
#include "desktop-background-manager.h"
#include "explorer-desktop-application.h"
#include "desktop-menu.h"

#include <QScreen>
#include <QPainter>
#include <QVariantAnimation>
#include <QTimeLine>
#include <KWindowSystem>
#include <QPlatformSurfaceEvent>
#include "plasma-shell-manager.h"

#include <QRegion>

#include <QX11Info>
#include <X11/Xlib.h>

static QTimeLine *gTimeLine = nullptr;

static bool startup = false;

DesktopBackgroundWindow::DesktopBackgroundWindow(const KScreen::OutputPtr &output, int desktopWindowId, QWidget *parent) : m_output(output), QMainWindow(parent)
{
    if (!gTimeLine) {
        gTimeLine = new QTimeLine(100);
    }
    connect(gTimeLine, &QTimeLine::finished, this, &DesktopBackgroundWindow::updateWindowGeometry);

    QString title = QString("desktop%1").arg(desktopWindowId);
    setWindowTitle(title);

    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
    KWindowSystem::setType(this->winId(), NET::Desktop);
    KWindowSystem::setState(this->winId(), NET::SkipTaskbar|NET::SkipPager|NET::SkipSwitcher);

    setContextMenuPolicy(Qt::CustomContextMenu);

    //m_screen = screen;
    m_desktopIconView = new Peony::DesktopIconView(this);
    m_desktopIconView->setId(desktopWindowId);
    m_id = desktopWindowId;
    move(getLogicalGeometryFromScreen().topLeft());
    setFixedSize(getLogicalGeometryFromScreen().size());
    setContentsMargins(0, 0, 0, 0);

    auto manager = DesktopBackgroundManager::globalInstance();
    connect(manager, &DesktopBackgroundManager::screensUpdated, this, QOverload<>::of(&DesktopBackgroundWindow::update));

    if (QGSettings::isSchemaInstalled("org.lingmo.panel.settings")) {
        m_panelSetting = new QGSettings("org.lingmo.panel.settings", QByteArray(), this);
    }

    connect(this, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos){
        if (centralWidget() != m_desktopIconView)
            return;
        QPoint relativePos = getRelativePos(pos);
        qInfo()<<pos;
        // fix #115384, context menu key issue
//        auto index = PeonyDesktopApplication::getIconView()->indexAt(relativePos);
//        if (!index.isValid()) {
//            PeonyDesktopApplication::getIconView()->clearSelection();
//        } else {
//            if (!PeonyDesktopApplication::getIconView()->selectionModel()->selection().indexes().contains(index)) {
//                PeonyDesktopApplication::getIconView()->clearSelection();
//                PeonyDesktopApplication::getIconView()->selectionModel()->select(index, QItemSelectionModel::Select);
//            }
//        }

        QTimer::singleShot(1, [=]() {
           //task#74174  扩展屏设置菜单
            if (m_menu) {
                delete m_menu;
                m_menu = nullptr;
            }
            m_menu = new DesktopMenu(m_desktopIconView, this);
            connect(m_menu, &DesktopMenu::setDefaultZoomLevel, this, &DesktopBackgroundWindow::setDefaultZoomLevel);
            connect(m_menu, &DesktopMenu::setSortType, this, &DesktopBackgroundWindow::setSortType);

            if (m_desktopIconView->getSelections().isEmpty()) {
                auto action = m_menu->addAction(QObject::tr("Set Background"));
                connect(action, &QAction::triggered, [=]() {
                    //go to control center set background
                    PeonyDesktopApplication::gotoSetBackground();
                });
                auto action1 = m_menu->addAction(QObject::tr("Display Settings"));
                connect(action1, &QAction::triggered, [=]() {
                    //go to control center set resolution ratio
                    PeonyDesktopApplication::gotoSetResolution();
                });

            }

            for (auto screen : qApp->screens()) {
                if (screen->geometry().contains(relativePos));
                //menu.windowHandle()->setScreen(screen);
            }

            /* 菜单执行弹出操作时停止更新，超过1s或者结束菜单都启用更新; 解决：点击鼠标右键，右键菜单会闪烁（偶现） */
            setUpdatesEnabled(false);
            QTimer::singleShot(1000, this, [=](){
                if(!updatesEnabled()){
                    setUpdatesEnabled(true);
                }
            });
            m_menu->exec(mapToGlobal(pos));
            setUpdatesEnabled(true);//end

            auto urisToEdit = m_menu->urisToEdit();
            m_desktopIconView->UpdateToEditUris(urisToEdit);
//            if (urisToEdit.count() >= 1) {
//                QTimer::singleShot(
//                            100, this, [=]() {
//                    m_desktopIconView->editUri(urisToEdit.first());
//                    qDebug() << "editUri count >=1:"<<urisToEdit.first();
//                });
//            }
        });
    });


    connect(m_output.data(), &KScreen::Output::posChanged,
            this, [=](){
        qDebug() << "output posChanged:" << output.data()->name()<< output->id() << output->geometry();
        setWindowGeometry(getLogicalGeometryFromScreen());
    });

    connect(m_output.data(), &KScreen::Output::currentModeIdChanged,
            this, [=](){
        qDebug() << "output currentModeIdChanged:" << output.data()->name()<< output->id() << output->geometry();
        setWindowGeometry(getLogicalGeometryFromScreen());
    });

    connect(m_output.data(), &KScreen::Output::rotationChanged,
            this, [=](){
        qDebug() << "output rotationChanged:" << output.data()->name()<< output->id() << output->geometry();
        setWindowGeometry(getLogicalGeometryFromScreen());
    });

    connect(m_output.data(), &KScreen::Output::sizeChanged,
            this, [=](){
        qDebug() << "output sizeChanged:" << output.data()->name()<< output->id() << output->geometry();
        setWindowGeometry(getLogicalGeometryFromScreen());
    });

    connect(m_output.data(), &KScreen::Output::scaleChanged,
            this, [=](){
        qDebug() << "output scaleChanged:" << m_output.data()->name()<< m_output->id() << m_output->geometry();
        setWindowGeometry(m_output->geometry());
    });
}

DesktopBackgroundWindow::~DesktopBackgroundWindow()
{

}

void DesktopBackgroundWindow::paintEvent(QPaintEvent *event)
{
    auto manager = DesktopBackgroundManager::globalInstance();
    if (!manager->getPaintBackground())
        return;

    if (m_output.isNull())
        return;

    QPainter p(this);
    if (manager->getUsePureColor()) {
        p.fillRect(this->rect(), manager->getColor());
    } else {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        auto animation = manager->getAnimation();
        QPixmap frontPixmap = manager->getFrontPixmap();
        QSize screenSize = getLogicalGeometryFromScreen().size();
        if (animation->state() == QVariantAnimation::Running) {
            auto opacity = animation->currentValue().toReal();
            QPixmap backPixmap = manager->getBackPixmap();

            if (manager->getBackgroundOption() == "centered") {
                //居中
                p.drawPixmap((screenSize.width() - backPixmap.rect().width()) / 2,
                             (screenSize.height() - backPixmap.rect().height()) / 2,
                             backPixmap);
                p.setOpacity(opacity);
                p.drawPixmap((screenSize.width() - frontPixmap.rect().width()) / 2,
                             (screenSize.height() - frontPixmap.rect().height()) / 2,
                             frontPixmap);
            } else if (manager->getBackgroundOption() == "stretched") {
                //拉伸
                p.drawPixmap(this->rect(), backPixmap, backPixmap.rect());
                p.setOpacity(opacity);
                p.drawPixmap(this->rect(), frontPixmap, frontPixmap.rect());
            } else if (manager->getBackgroundOption() == "scaled") {
                //填充
                p.drawPixmap(this->rect(), backPixmap, getSourceRect(backPixmap));
                p.setOpacity(opacity);
                p.drawPixmap(this->rect(), frontPixmap, getSourceRect(frontPixmap));
            } else if (manager->getBackgroundOption() == "wallpaper") {
                //平铺
                int drawedWidth = 0;
                int drawedHeight = 0;
                while (1) {
                    drawedWidth = 0;
                    while (1) {
                        p.drawPixmap(drawedWidth, drawedHeight, backPixmap);
                        drawedWidth += backPixmap.width();
                        if (drawedWidth >= screenSize.width()) {
                            break;
                        }
                    }
                    drawedHeight += backPixmap.height();
                    if (drawedHeight >= screenSize.height()) {
                        break;
                    }
                }
                p.setOpacity(opacity);
                drawedWidth = 0;
                drawedHeight = 0;
                while (1) {
                    drawedWidth = 0;
                    while (1) {
                        p.drawPixmap(drawedWidth, drawedHeight, frontPixmap);
                        drawedWidth += frontPixmap.width();
                        if (drawedWidth >= screenSize.width()) {
                            break;
                        }
                    }
                    drawedHeight += frontPixmap.height();
                    if (drawedHeight >= screenSize.height()) {
                        break;
                    }
                }
            } else if (manager->getBackgroundOption() == "zoom") {
                //适应
                p.drawPixmap(getDestRect(backPixmap), backPixmap, backPixmap.rect());
                p.setOpacity(opacity);
                p.drawPixmap(getDestRect(frontPixmap), frontPixmap, frontPixmap.rect());
            } else if (manager->getBackgroundOption() == "spanned") {
                //跨区
                auto geometry = getLogicalGeometryFromScreen();
                p.setOpacity(opacity);
                p.drawPixmap(this->rect(), frontPixmap, getSourceRect(frontPixmap, geometry));
            } else {
                p.drawPixmap(rect().adjusted(0, 0, -1, -1), backPixmap, backPixmap.rect());
                p.setOpacity(opacity);
                p.drawPixmap(rect().adjusted(0, 0, -1, -1), frontPixmap, frontPixmap.rect());
            }

        } else {
            if (manager->getBackgroundOption() == "centered") {
                p.drawPixmap((screenSize.width() - frontPixmap.rect().width()) / 2,
                             (screenSize.height() - frontPixmap.rect().height()) / 2,
                             frontPixmap);
            } else if (manager->getBackgroundOption() == "stretched") {
                p.drawPixmap(this->rect(), frontPixmap, frontPixmap.rect());

            } else if (manager->getBackgroundOption() == "scaled") {
                p.drawPixmap(this->rect(), frontPixmap, getSourceRect(frontPixmap));

            } else if (manager->getBackgroundOption() == "wallpaper") {
                int drawedWidth = 0;
                int drawedHeight = 0;
                while (1) {
                    drawedWidth = 0;
                    while (1) {
                        p.drawPixmap(drawedWidth, drawedHeight, frontPixmap);
                        drawedWidth += frontPixmap.width();
                        if (drawedWidth >= screenSize.width()) {
                            break;
                        }
                    }
                    drawedHeight += frontPixmap.height();
                    if (drawedHeight >= screenSize.height()) {
                        break;
                    }
                }
            } else if (manager->getBackgroundOption() == "zoom") {
                p.drawPixmap(getDestRect(frontPixmap), frontPixmap, frontPixmap.rect());
            } else if (manager->getBackgroundOption() == "spanned") {
                auto geometry = getLogicalGeometryFromScreen();
                p.drawPixmap(this->rect(), frontPixmap, getSourceRect(frontPixmap, geometry));
            } else {
                p.drawPixmap(rect().adjusted(0, 0, -1, -1), frontPixmap, frontPixmap.rect());
            }
        }
        p.restore();
    }

    if (!startup) {
        startup = true;
        QTimer::singleShot(1000, []{
            if (QX11Info::isPlatformX11()) {
                XSetWindowBackground(QX11Info::display(), QX11Info::appRootWindow(), 0);
                XSync(QX11Info::display(), false);
            }
        });
    }
}

KScreen::OutputPtr DesktopBackgroundWindow::screen() const
{
    return m_output;
}

QRect DesktopBackgroundWindow::getLogicalGeometryFromScreen()
{
    if (!m_output.isNull() && m_output->isEnabled()) {
        if (QApplication::platformName().toLower().contains("wayland")) {
            return m_output->geometry();
        }
        qreal ratio = this->windowHandle()->devicePixelRatio();
        QRect rect = QRect(m_output->geometry().topLeft()/ratio, m_output->geometry().size()/ratio);
        return rect;
    } else {
        return this->geometry();
    }
}

void DesktopBackgroundWindow::invaidScreen()
{
    m_output = nullptr;
}

bool DesktopBackgroundWindow::event(QEvent *event)
{
    if (event->type() == QEvent::PlatformSurface) {
        auto e = static_cast<QPlatformSurfaceEvent *>(event);
        switch (e->surfaceEventType()) {
        case QPlatformSurfaceEvent::SurfaceCreated: {
            m_shellSurface = PlasmaShellManager::getInstance()->createSurface(this->windowHandle());
            if (m_shellSurface) {
                m_shellSurface->setRole(KWayland::Client::PlasmaShellSurface::Role::Desktop);
                m_shellSurface->setSkipSwitcher(true);
                m_shellSurface->setSkipTaskbar(true);
                // wayland中构造函数的move只能在这里生效
                if (!m_output.isNull()) {
                    m_shellSurface->setPosition(getLogicalGeometryFromScreen().topLeft());
                }
            }
            break;
        }
        case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed: {
            if (m_shellSurface) {
                m_shellSurface->deleteLater();
                m_shellSurface = nullptr;
            }
            break;
        }
        default:
            break;
        }
    }
    return QMainWindow::event(event);
}

void DesktopBackgroundWindow::setWindowGeometry(const QRect &geometry)
{
    qInfo()<<"bg window geometry changed"<<geometry;
    if (gTimeLine->state() != QTimeLine::Running) {
        gTimeLine->start();
    } else {
        gTimeLine->setCurrentTime(0);
    }
}

void DesktopBackgroundWindow::updateWindowGeometry()
{
    if (m_output.isNull()) {
        return;
    }
    int mode = ((PeonyDesktopApplication*)qApp)->checkScreenMode(m_output->geometry());
    ((PeonyDesktopApplication*)qApp)->changeMode(mode);
    auto geometry = getLogicalGeometryFromScreen();
    move(geometry.topLeft());
    if (m_shellSurface) {
        m_shellSurface->setPosition(geometry.topLeft());
    }
    setFixedSize(geometry.size());

   qInfo()<<"bg window geometry changed slot"<<screen().data()->name()<<geometry;
    // raise primary window to make sure icon view is visible.
    if (centralWidget()) {
        if (screen()->isPrimary()) {
            qInfo()<<"has center widget, raise window";
            KWindowSystem::raiseWindow(this->winId());
            kdk::WindowManager::activateWindow(m_windowId);
        } else {
            qCritical()<<"raise a window which not in primary screen, but has central widget";
        }
    }
    update();
}

int DesktopBackgroundWindow::id() const
{
    return m_id;
}

void DesktopBackgroundWindow::setId(int id)
{
    m_id = id;
    m_desktopIconView->setId(id);
}

void DesktopBackgroundWindow::setWindowId(kdk::WindowId  windowId)
{
    m_windowId = windowId;
}

kdk::WindowId DesktopBackgroundWindow::getWindowId()
{
    return m_windowId;
}

//获取iconview中图标的相对位置
QPoint DesktopBackgroundWindow::getRelativePos(const QPoint &pos)
{
    if (m_output.isNull()) {
        return pos;
    }
    QPoint relativePos = pos;
    if (m_output->isPrimary()) {
        if (m_panelSetting) {
            int position = m_panelSetting->get("panelposition").toInt();
            int offset = m_panelSetting->get("panelsize").toInt();

            switch (position) {
                case 1: {
                    relativePos -= QPoint(0, offset);
                    break;
                }
                case 2: {
                    relativePos -= QPoint(offset, 0);
                    break;
                }
                case 3: {
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    return relativePos;
}

QRect DesktopBackgroundWindow::getSourceRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if ((width / height) == screenScale) {
        return pixmap.rect();
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(this->rect().height()) / qreal(this->rect().width());
    }

    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = pixmap.size();
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(isShortX ? shortEdge : longEdge);
        sourceSize.setHeight(isShortX ? longEdge : shortEdge);
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (pixmap.width() > sourceSize.width()) {
        offsetX = (pixmap.width() - sourceSize.width()) / 2;
    }

    if (pixmap.height() > sourceSize.height()) {
        offsetY = (pixmap.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

QRect DesktopBackgroundWindow::getSourceRect(const QPixmap &pixmap, const QRect &screenGeometry)
{
    QRegion virtualScreensRegion;
    for (auto qscreen : qApp->screens()) {
        virtualScreensRegion += qscreen->geometry();
    }
    QRect virtualGeometry = virtualScreensRegion.boundingRect().translated(0, 0);

    qreal pixWidth = pixmap.width();
    qreal pixHeight = pixmap.height();

    QSize sourceSize = pixmap.size();
    sourceSize.setWidth(screenGeometry.width() * 1.0 / virtualGeometry.width() * pixWidth);
    sourceSize.setHeight(screenGeometry.height() * 1.0 / virtualGeometry.height() * pixHeight);

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (screenGeometry.x() > 0) {
        offsetX = (screenGeometry.x() * 1.0 / virtualGeometry.width() * pixWidth);
    }

    if (screenGeometry.y() > 0) {
        offsetY = (screenGeometry.y() * 1.0 / virtualGeometry.height() * pixHeight);
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

QRect DesktopBackgroundWindow::getDestRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal pixmapScale = qreal(pixmap.width() / pixmap.height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if (pixmapScale == screenScale) {
        return this->rect();
    }

    qreal scaleWidth = this->rect().width() / width;
    qreal scaleHeight = this->rect().height() / height;
    qreal realPixmapWidth = 0;
    qreal realPixmapHeight = 0;

    if(pixmapScale < screenScale){
        //图片比例小于屏幕比例时，按照图片和屏幕高度比进行缩放
        realPixmapWidth = width * scaleHeight;
        realPixmapHeight = this->rect().height();
    }else{
        //图片比例大于屏幕比例时，按照图片与屏幕宽度比进行缩放
        realPixmapWidth = this->rect().width();
        realPixmapHeight = height * scaleWidth;
    }

    QSize sourceSize = this->size();
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (this->rect().width() == realPixmapWidth) {
        offsetY = (this->rect().height() - realPixmapHeight) / 2;
        sourceSize.setHeight(realPixmapHeight);
    } else if (this->rect().height() == realPixmapHeight) {
        offsetX = (this->rect().width() - realPixmapWidth) / 2;
        sourceSize.setWidth(realPixmapWidth);
    }

    // 规避xcb下闪线的问题
    sourceSize = sourceSize - QSize(1, 1);

    qDebug() << "=========getDestRect sourceSize:" << sourceSize;
    QPoint offsetPoint = this->rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

Peony::DesktopIconView *DesktopBackgroundWindow::getIconView()
{
    return m_desktopIconView;
}

void DesktopBackgroundWindow::setCentralView()
{
    if (centralWidget()) {
        takeCentralWidget();
    }

    setCentralWidget(m_desktopIconView);
}
