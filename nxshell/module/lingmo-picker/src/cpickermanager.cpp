// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpickermanager.h"
#include <QWindow>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QKeyEvent>
#include <DRegionMonitor>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QPainterPath>
#include <QProcessEnvironment>
#include <QTimer>

#include "settings.h"
#include "utils.h"

DGUI_USE_NAMESPACE

CScreenshotWidget::CScreenshotWidget(CPickerManager *parent): QWidget(nullptr),
    _parentManager(parent)
{
    //为顶层窗口
    this->setWindowFlags(this->windowFlags() | Qt::Window | Qt::FramelessWindowHint
                         | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    winId();
}

void CScreenshotWidget::setPixmap(const QPixmap &pixMap)
{
    _scrennshotPixmap = pixMap;
}

QPixmap CScreenshotWidget::pixMap()
{
    return _scrennshotPixmap;
}

void CScreenshotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(rect(), pixMap(), pixMap().rect());
    QPoint centerPos = mapFromGlobal(QCursor::pos());
    if (rect().contains(centerPos)) {
        if (!_parentManager->_cursorPix.isNull())
            painter.drawPixmap(QRect(centerPos - QPoint(_parentManager->_cursorPix.width() / 2, _parentManager->_cursorPix.height() / 2),
                                     _parentManager->_cursorPix.size()), _parentManager->_cursorPix, _parentManager->_cursorPix.rect());
    }
}

void CScreenshotWidget::keyPressEvent(QKeyEvent *event)
{
    //wayland下 DRegionMonitor::keyPress会失效，这里会响应
    if (event->matches(QKeySequence::Cancel)) {
        QApplication::quit();
    }
    QWidget::keyPressEvent(event);
}

void CScreenshotWidget::wheelEvent(QWheelEvent *event)
{
//暂时不支持动态变化缩放比值
//    if (event->modifiers()& Qt::ControlModifier) {
//        qreal &scaleFactor = _parentManager->_scaleFactor;
//        if (event->delta() < 0) {
//            --scaleFactor;
//            scaleFactor = qMax(2.0, scaleFactor);
//        } else {
//            ++scaleFactor;
//            scaleFactor = qMin(50.0, scaleFactor);
//        }
//    }
    QWidget::wheelEvent(event);
}

CPickerManager::CPickerManager(): QObject(nullptr)
{
    const int windowHeight = 236;
    const int windowWidth = 236;
    qreal radio = qApp->devicePixelRatio();
    _shadowPixmap = QPixmap(Utils::getQrcPath("shadow.png"));
    if (isWaylandPlatform()) {
        _shadowPixmap = _shadowPixmap.scaled(windowWidth, windowHeight);
    } else {
        _shadowPixmap = _shadowPixmap.scaled(windowWidth / radio, windowHeight / radio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    initShotScreenWidgets();

    DRegionMonitor *eventMonitor = new DRegionMonitor;
    //设置捕获区域
    eventMonitor->setWatchedRegion(QRegion(INT_MIN, INT_MIN, INT_MAX * 2, INT_MAX * 2));

    QObject::connect(eventMonitor, &DRegionMonitor::keyPress, this, [ = ](const QString & name) {
        if (name == "Escape")
            QApplication::quit();
    });


    // Binding handler to xrecord signal.
    QObject::connect(eventMonitor, &DRegionMonitor::cursorMove, this, [ = ](const QPoint & p) {
        onMouseMove(p);
    });
    QObject::connect(eventMonitor, &DRegionMonitor::buttonPress, this, [ = ](const QPoint & p, const int flag) {
        onMousePress(p, flag);
    });

    eventMonitor->setCoordinateType(DRegionMonitor::Original);
    eventMonitor->registerRegion();

    if (!eventMonitor->registered()) {
        qWarning() << "Failed on register monitor";
        QApplication::quit();
    }
    ensureDeskTopPixmap();

    _updateScreenshotTimer = new QTimer(this);
    _updateScreenshotTimer->setSingleShot(true);
    connect(_updateScreenshotTimer, SIGNAL(timeout()), this, SLOT(handleMouseMove()));


    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    qApp->setOverrideCursor(QCursor(pix));
}

CPickerManager::~CPickerManager()
{
}

void CPickerManager::setLanchFlag(CPickerManager::ELanchType tp, const QString &appName)
{
    _isLaunchByDBus = tp;
    if (_isLaunchByDBus == ELanchedByOtherApp)
        _appid = appName;
}

//QString CPickerManager::caller()
//{
//    return _appid;
//}

//QColor CPickerManager::currentColor() const
//{
//    return _curColor;
//}

void CPickerManager::StartPick(const QString &id)
{
    _appid = id;
}

void CPickerManager::onMouseMove(const QPoint &pos)
{
    if (_updateScreenshotTimer->isActive()) {
        _updateScreenshotTimer->stop();
    }
    _pos = pos;
    _updateScreenshotTimer->start(5);
}

void CPickerManager::onMousePress(const QPoint &p, const int flag)
{
    Q_UNUSED(p)
    int button = flag;

    if (isWaylandPlatform()) {
        //wayland触摸屏下的点击是DRegionMonitor::Button_Middle，鼠标左键点击DRegionMonitor::Button_Left。无法区分是鼠标左键还是触摸屏，故支持中键点击
        //最佳方案是TDK将触摸屏点击修改成左键点击。。。。。。。
        if (button != DRegionMonitor::Button_Left && button != DRegionMonitor::Button_Middle) {
            return;
        }
    } else {
        if (button != DRegionMonitor::Button_Left)
            return;
    }

    //立即更新坐标
    _pos = p;
    handleMouseMove();

    //if (!displayCursorDot && isVisible())
    {
        // Rest cursor and hide window.
        // NOTE: Don't call hide() at here, let process die,
        // Otherwise mouse event will pass to application window under picker.
        QApplication::setOverrideCursor(Qt::ArrowCursor);

        // Rest color type to hex if config file not exist.
        Settings settings;

        // Emit copyColor signal to copy color to system clipboard.
        copyColor(_curColor, settings.getOption("color_type", "HEX").toString());

        // Send colorPicked signal when call by DBus and no empty appid.
        if (_appid != "") {
            colorPicked(_appid, Utils::colorToHex(_curColor));
        }
    }
}

void CPickerManager::handleMouseMove()
{
    ensureDeskTopPixmap();
    updateCursor(_desktopPixmap, _pos);
}

void CPickerManager::initShotScreenWidgets()
{
    ensureDeskTopPixmap();
    auto screens = QApplication::screens();
    //去出复制屏
    int i = 0;
    while (i < screens.size()) {
        for (int j = screens.size() - 1; j > i; j--) {
            if (screens.at(i)->geometry().topLeft() == screens.at(j)->geometry().topLeft()) {
                screens.removeAt(j);
            }
        }
        i++;
    }

    foreach (auto screen, screens) {
        auto pix = getScreenShotPixmap(screen);
        //auto geometry = QRect(screen->geometry().topLeft(), screen->geometry().size() * screen->devicePixelRatio());
        CScreenshotWidget *pWidget = new CScreenshotWidget(this);
        pWidget->setPixmap(pix);
        pWidget->setGeometry(screen->geometry());
        _widgets[screen] = pWidget;
        pWidget->show();
        pWidget->raise();

        pWidget->activateWindow();
        pWidget->setFocus();

        ensureDeskTopPixmap();
        updateCursor(_desktopPixmap, QCursor::pos(screen));
    }

    // 解决 wayland 下调出取色器后点击 Esc 无法退出取色器
    if (isWaylandPlatform()) {
        // 延迟 200ms 后取得当前背景窗口的焦点，用以捕获 Esc 按键退出
        QTimer::singleShot(200, this, [ = ]() {
            auto currentWidget = _widgets.value(qApp->screenAt(QCursor::pos()));
            if (nullptr != currentWidget) {
                currentWidget->activateWindow();
            }
        });
    }
}

void CPickerManager::updateCursor(const QPixmap &pixMap, const QPoint &posInPixmap)
{
    qreal scalFactor = _scaleFactor;

    const QSize const_focusSize = QSize(200, 200) * qApp->devicePixelRatio() / scalFactor;
    //必须保证聚焦区域的宽高都是奇数(这样才能均分像素点，保证中心焦点像素和显示像素重合)
    QSize focusSize = QSize(const_focusSize.width() % 2 == 0 ? (const_focusSize.width() + 1) : const_focusSize.width(),
                            const_focusSize.height() % 2 == 0 ? (const_focusSize.height() + 1) : const_focusSize.height());
    QRectF focusRect = QRectF(posInPixmap, focusSize).translated(-focusSize.width() / 2, -focusSize.height() / 2);

    QPixmap focusPixmap = pixMap.copy(focusRect.toRect());

    //获取焦点区域图形
    focusPixmap = focusPixmap.scaled(focusSize * scalFactor);

    QRect focusPixmapVaildRect = focusPixmap.rect();
    {
        //生成不规则的鼠标图(要不焦点区域的放大图再增大20个像素以保证边界阴影能显示出来)
        QSize cusrorPixSize = QSize(24, 24) * qApp->devicePixelRatio() + focusPixmapVaildRect.size() ;
        QPixmap cursorPix(cusrorPixSize);
        cursorPix.fill(Qt::transparent);
        QPainter painter(&cursorPix);

        painter.setRenderHint(QPainter::Antialiasing);

        //0.绘制阴影
        painter.save();
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(cursorPix.rect(), _shadowPixmap);
        painter.restore();

        painter.save();
        QRectF vaildRect = QRectF(QPointF((cusrorPixSize.width() - focusPixmapVaildRect.width()) / 2.0,
                                          (cusrorPixSize.height() - focusPixmapVaildRect.height()) / 2.0), focusPixmapVaildRect.size());

        painter.translate(vaildRect.topLeft());
        vaildRect.translate(-vaildRect.topLeft());

        //1.设置有效的裁剪区域(可见区域是一个圆形)
        QPainterPath clipPath;
        clipPath.addEllipse(vaildRect);
        painter.setClipPath(clipPath);
        painter.drawPixmap(vaildRect.toRect(), focusPixmap, focusPixmapVaildRect);

        painter.setRenderHint(QPainter::Antialiasing, false);

        //2.绘制圆形的外边线
        QPen p(QColor(255, 255, 255, 87));
        p.setWidth(6 * qApp->devicePixelRatio());
        painter.setPen(p);
        painter.drawEllipse(vaildRect);

        //3.绘制中心像素点;放大了的焦点图像的像素大小对应也放大了scalFactor倍
        qreal logicPixelWidth = 1  * scalFactor;
        QPointF innerCenter = vaildRect.center();
        QRectF innerRect(innerCenter, QSizeF(1, 1));
        qreal offset = (logicPixelWidth - 1) / 2;
        innerRect.adjust(-offset, -offset, offset, offset);

        painter.setPen(QColor(255, 255, 255, 89));
        painter.drawRect(innerRect);

        painter.setPen(QColor(0, 0, 0, 89));
        painter.drawRect(innerRect.adjusted(-1, -1, 1, 1));

        painter.restore();

        _curColor = focusPixmap.toImage().pixelColor(focusPixmap.rect().center());

//        painter.setPen(QColor(255, 0, 0, 100));
//        QFont f;
//        painter.setFont(f);
//        QString text = _curColor.name() + QString("w=%1lw=%2").arg(focusPixmap.width()).arg(logicPixelWidth);
//        QSize colorNameSz   = QSize(QFontMetrics(f).width(text), QFontMetrics(f).height()) ;

//        QRectF colorNameRect = QRectF(QPointF((cursorPix.width() - colorNameSz.width()) / 2,
//                                              (cursorPix.height() - colorNameSz.height()) / 2 + colorNameSz.height() + 6), colorNameSz);
//        painter.drawText(colorNameRect, text);

        //设置当前窗口鼠标样式
        _cursorPix = cursorPix;
        this->autoUpdate();

    }
}


void CPickerManager::ensureDeskTopPixmap()
{
    if (_desktopPixmapDirty) {
        _desktopPixmap = getDesktopPixmap();
        _desktopPixmapDirty = false;
    }
}

QPixmap CPickerManager::getScreenShotPixmap(QScreen *pScreen)
{
    QPixmap result;
    bool iswayLand = isWaylandPlatform();
    if (iswayLand) {
        auto geometry = QRect(pScreen->geometry().topLeft(), pScreen->geometry().size() * pScreen->devicePixelRatio());
        result = _desktopPixmap.copy(geometry);
    } else {
        result = pScreen->grabWindow(0);
    }
    return result;
}

QRect getDeskTopRect()
{
    QRect deskTopRect(0, 0, 0, 0);
    auto screens = QApplication::screens();
    foreach (auto screen, screens) {

        auto geometry = QRect(screen->geometry().topLeft(), screen->geometry().size() *
                              screen->devicePixelRatio());
        deskTopRect = deskTopRect.united(geometry);
    }
    return deskTopRect;
}
QPixmap CPickerManager::getDesktopPixmap()
{
    QPixmap result;
    bool iswayLand = isWaylandPlatform();
    if (iswayLand) {
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
        res = QPixmap(reply.value());
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        }
        result = res;
    } else {
        QPixmap pixs(getDeskTopRect().size());
        pixs.fill(Qt::transparent);
        QPainter painter(&pixs);
        auto screens = QApplication::screens();
        foreach (auto screen, screens) {
            auto pix = screen->grabWindow(0);
            auto geometry = QRect(screen->geometry().topLeft(), screen->geometry().size() * screen->devicePixelRatio());
            painter.drawPixmap(geometry, pix, pix.rect());
        }
        painter.end();
        result = pixs;
    }

    return result;
}

bool CPickerManager::isWaylandPlatform()
{
    static auto e = QProcessEnvironment::systemEnvironment();
    static auto XDG_CURRENT_DESKTOP = e.value(QStringLiteral("XDG_CURRENT_DESKTOP"));
    static auto XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    static auto WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    static auto KDE_FULL_SESSION = e.value(QStringLiteral("KDE_FULL_SESSION"));
    static auto GNOME_DESKTOP_SESSION_ID = e.value(QStringLiteral("GNOME_DESKTOP_SESSION_ID"));
    static auto DESKTOP_SESSION = e.value(QStringLiteral("DESKTOP_SESSION"));

    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}

void CPickerManager::autoUpdate()
{
    if (nullptr == qApp->screenAt(QCursor::pos()))
        return;

    static QWidget *lastWidget = nullptr;
    static QRect lastRect;
    auto currentWidget = _widgets.value(qApp->screenAt(QCursor::pos()));

    if (lastWidget != nullptr) {
        lastWidget->update(lastRect);
    }

    if (currentWidget != nullptr) {
        QRect currentRect = QRect(currentWidget->mapFromGlobal(QCursor::pos()) - QPoint(_cursorPix.width() / 2, _cursorPix.height() / 2), QSize(_cursorPix.size()));
        if (lastWidget != currentWidget)
            currentWidget->update(lastRect);
        currentWidget->update(currentRect);
        lastWidget = currentWidget;
        lastRect = currentRect;
    }
}
