// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "constant.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#include "utils/waylandmousesimulator.h"
#endif
#include <DDialog>
#include <DSysInfo>
#include <DWindowManagerHelper>
#include <DForeignWindow>


#include <QString>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>
#include <QtX11Extras/QX11Info>
#include <QStandardPaths>
#include <QProcess>
#include <QKeyEvent>
#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_sink.h>

#include <X11/extensions/shape.h>
#include <dlfcn.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

bool Utils::is3rdInterfaceStart = false;
bool Utils::isTabletEnvironment = false;
bool Utils::isWaylandMode = false;
bool Utils::isRootUser = false;
qreal Utils::pixelRatio = 0.0;
bool Utils::isFFmpegEnv = true;
int Utils::themeType = 0;
QString Utils::appName = "";

QString Utils::getQrcPath(QString imageName)
{
    return QString(":/%1").arg(imageName);
}

QSize Utils::getRenderSize(int fontSize, QString string)
{
    QFont font;
    font.setPointSize(fontSize);
    QFontMetrics fm(font);

    int width = 0;
    int height = 0;
    foreach (auto line, string.split("\n")) {
        int lineWidth = fm.width(line);
        int lineHeight = fm.height();

        if (lineWidth > width) {
            width = lineWidth;
        }
        height += lineHeight;
    }

    return QSize(width, height);
}

void Utils::setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font() ;
    font.setPointSize(textSize);
    painter.setFont(font);
}


void Utils::drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), Constant::RECTANGLE_RADIUS, Constant::RECTANGLE_RADIUS);
    painter.fillPath(path, QColor(textColor));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void Utils::drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect)
{
    Q_UNUSED(textColor);

    Utils::setFontSize(painter, textSize);
    painter.setOpacity(1);
    painter.setPen(QPen(QGuiApplication::palette().color(QPalette::BrightText)));
    painter.drawText(rect, Qt::AlignCenter, text);
}

void Utils::passInputEvent(int wid)
{
    // Wayland 事件穿透
    if (Utils::isWaylandMode) {
        QWidget *widget = QWidget::find(static_cast<WId>(wid));
        if (widget && widget->windowHandle()) {
            widget->windowHandle()->setMask(QRegion(0, 0, 1, 1));
        }
        return;
    }


    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = 0;
    reponseArea->y = 0;
    reponseArea->width = 0;
    reponseArea->height = 0;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    delete reponseArea;

}

void Utils::setAccessibility(DPushButton *button, const QString name)
{
    button->setObjectName(name);
    button->setAccessibleName(name);
}
void Utils::setAccessibility(DImageButton *button, const QString name)
{
    button->setObjectName(name);
    button->setAccessibleName(name);
}
void Utils::setAccessibility(QAction *action, const QString name)
{
    action->setObjectName(name);
    //action->setAccessibleName(name);
}

void Utils::getInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    if (Utils::isWaylandMode == true)
        return;
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

    /*
     *XShapeCombineRectangles:
     * param1:
     * param2:
     * param3:
     * param4:
     * param5:
     * param6:为NULL（参数为XRectangle*类型），那整个窗口都将被穿透
     * param7:控制设置穿透的，为0时表示设置鼠标穿透，为1时表示取消鼠标穿透，当取消设置鼠标穿透的时候，必须设置区域，
     * param8:
     * param9:
     */
    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSubtract, YXBanded);
    delete reponseArea;

}

void Utils::cancelInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    if (Utils::isWaylandMode == true)
        return;
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    delete reponseArea;

}

void Utils::cancelInputEvent1(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 0, ShapeSubtract, YXBanded);
    delete reponseArea;

}


// 录屏显示录制时长， 依赖dde-dock。
// 判断系统是否是大于1040, 大于1040才显示录制时长功能。
bool Utils::isSysHighVersion1040()
{
    if (DSysInfo::isDeepin()) {
        bool correct = false;
        QString sysVersion = DSysInfo::minorVersion();
        float version = sysVersion.toFloat(&correct);
        qDebug() << DSysInfo::uosEditionName() << DSysInfo::uosEditionType() << " System Version:" << sysVersion << correct << sysVersion.split('.');
        if (DSysInfo::UosProfessional == DSysInfo::uosEditionType() || DSysInfo::UosEducation == DSysInfo::uosEditionType()) {
            const float versionProfessional = 1040;
            if (correct && (version >= versionProfessional)) {
                return true;
            }
        } else if (DSysInfo::UosHome == DSysInfo::uosEditionType()) {
            const float versionHome = 21.3f;
            if (correct && (version >= versionHome)) {
                return true;
            }
        } else if (DSysInfo::UosCommunity == DSysInfo::uosEditionType()) {
            const float versionCommunity = 20.6f;
            QStringList v = sysVersion.split('.');
            if (!correct && v.size() > 2) {
                // 社区版，版本号可能有20.6.1，只取前面的两位大版本号
                version = (QString("%1.%2").arg(v[0]).arg(v[1])).toFloat(&correct);
                qDebug() << "Fix System Version:" << version << correct;
            }
            if (correct && (version >= versionCommunity)) {
                return true;
            }
        }
    }
    return false;
}

void Utils::showCurrentSys()
{
    qInfo() << "isDeepin: " << DSysInfo::isDeepin();
    qInfo() << "isDDE: " << DSysInfo::isDDE();
    qInfo() << "SystemName: " << DSysInfo::uosSystemName();
    qInfo() << "EditionName: " << DSysInfo::uosEditionName();
    qInfo() << "ProductTypeName: " << DSysInfo::uosProductTypeName();
    qInfo() << "SystemVersion: " << DSysInfo::minorVersion();
    //qDebug() << "spVersion: " << DSysInfo::spVersion();
    //qDebug() << "udpateVersion: " << DSysInfo::udpateVersion();
    //qDebug() << "majorVersion: " << DSysInfo::majorVersion();
    //qDebug() << "majorVersion: " << DSysInfo::majorVersion();
}

void Utils::enableXGrabButton()
{
    if (Utils::isWaylandMode == true)
        return;
    //extern int XGrabButton(
    //    Display *      /* display */,
    //    unsigned int  /* button */,
    //    unsigned int  /* modifiers */,
    //    Window        /* grab_window */,
    //    Bool      /* owner_events */,
    //    unsigned int  /* event_mask */,
    //    int           /* pointer_mode */,
    //    int           /* keyboard_mode */,
    //    Window        /* confine_to */,
    //    Cursor        /* cursor */
    //);
    XGrabButton(QX11Info::display(), Button1, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(QX11Info::display(), Button2, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(QX11Info::display(),  Button3, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
}

void Utils::disableXGrabButton()
{
    if (Utils::isWaylandMode == true)
        return;
    XUngrabButton(QX11Info::display(), true, AnyModifier, DefaultRootWindow(QX11Info::display()));

}

void Utils::getAllWindowInfo(const quint32 winId, const int width, const int height, QList<QRect> &windowRects, QList<QString> &windowNames)
{
    Dtk::Gui::DForeignWindow *prewindow = nullptr;
    for (quint32 wid : Dtk::Gui::DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        if (wid == winId) continue;
        if (prewindow && isSysHighVersion1040()) {
            delete prewindow;
            prewindow = nullptr;
        }

        Dtk::Gui::DForeignWindow *window = Dtk::Gui::DForeignWindow::fromWinId(wid);//sanitizer dtk

        prewindow = window;

        //判断窗口是否被最小化
        if (window->windowState() == Qt::WindowState::WindowMinimized) {
            continue;
        }

        if (window) {
            int t_tempWidth = 0;
            int t_tempHeight = 0;
            //window:后面代码有使用
            //window->deleteLater();
            //修改部分窗口显示不全，截图框识别问题
//            qDebug()  << "\n窗口名称: >>>> " << window->wmClass()
//                      << "\nx: " <<  window->frameGeometry().x()
//                      << "\ny: " <<  window->frameGeometry().y()
//                      << "\nwidth: " <<  window->frameGeometry().width()
//                      << "\nheight: " <<  window->frameGeometry().height()   ;
            //x坐标小于0时
            if (window->frameGeometry().x() < 0) {
                if (window->frameGeometry().y() < 0) {

                    //x,y为负坐标情况
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //windowRects << Dtk::Wm::WindowRect {0, 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //x为负坐标，y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //x为负坐标，y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            //x坐标位于正常屏幕区间时
            else if (window->frameGeometry().x() >= 0 && window->frameGeometry().x() <= width - window->frameGeometry().width()) {
                if (window->frameGeometry().y() < 0) {
                    //y为负坐标情况
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            //x方向窗口超出屏幕右侧区域
            else if (window->frameGeometry().x() > width - window->frameGeometry().width()) {
                if (window->frameGeometry().y() < 0) {
                    //y为负坐标情况
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //y方向窗口超出屏幕底部
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }
        }
    }
    if (prewindow && isSysHighVersion1040()) {
        delete prewindow;
        prewindow = nullptr;
    }
}


bool Utils::checkCpuIsZhaoxin()
{
    QStringList options;
    options << "-c";
    options << "lscpu | grep 'CentaurHauls'";
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    if (str_output.length() == 0) {
        return false;
    }
    return true;
}

//获取处理器名称
QString Utils::getCpuModelName()
{
    if (DSysInfo::cpuModelName().isNull()) {
        return "";
    }
    return DSysInfo::cpuModelName();
}

void Utils::notSupportWarn()
{
    DDialog warnDlg;
    warnDlg.setIcon(QIcon::fromTheme("deepin-screen-recorder"));
    warnDlg.setMessage(tr("Screen recording is not supported at present"));
    warnDlg.addSpacing(20);
    warnDlg.addButton(tr("Exit"));
    warnDlg.exec();
}


//传入屏幕上理论未经缩放的点，获取缩放后实际的点
QPoint Utils::getPosWithScreen(QPoint pos)
{
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    //获取所有实际屏幕的信息
    screensInfo = getScreensInfo();    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    for (int i = 0; i < screensInfo.size(); i++) {
        //判断当前点在哪块屏幕上
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + screensInfo[i].width &&
                pos.y() > screensInfo[i].y && pos.y() < screensInfo[i].y + screensInfo[i].height) {

            qInfo() << "screenInfo: " << screensInfo[i].toString();
            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x) / pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y) / pixelRatio) + + screensInfo[i].y);
        }
    }
    return dpos;
}

//传入屏幕上已经缩放后的点，获取理论上实际的点
QPoint Utils::getPosWithScreenP(QPoint pos)
{
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    //获取所有实际屏幕的信息
    screensInfo = getScreensInfo();
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    for (int i = 0; i < screensInfo.size(); i++) {
        //判断当前点在哪块屏幕上
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + screensInfo[i].width &&
                pos.y() > screensInfo[i].y && pos.y() < screensInfo[i].y + screensInfo[i].height) {

            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x)*pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y)*pixelRatio + screensInfo[i].y));
        }
    }

    return dpos;
}

QList<Utils::ScreenInfo> Utils::getScreensInfo()
{
    //获取所有实际屏幕的信息
    QList<QScreen *> screenList = qApp->screens();
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QList<ScreenInfo> screensInfo;
    /**
     * 例如：显示器实际屏幕参数为1920*1080记为 t1，调整缩放比例为1.25记为 p,那么此时显示器显示的屏幕大小将变为1536*864记为 t2，
     * 那么此时在 t2的点要变成t1上的点，需要 t2*p = t1,在多屏的情况且需要考虑究竟在那个屏幕上
     */
    //将实际屏幕信息根据缩放比列转化为理论上屏幕信息
    int hTotal = 0;
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        //qDebug() << (*it)->name() << rect;
        ScreenInfo screenInfo;
        screenInfo.x = rect.x(); //屏幕的起始x坐标始终是正确的，不管是否经过缩放
        screenInfo.y = rect.y(); //屏幕的起始y坐标始终是正确的，不管是否经过缩放
        screenInfo.height =  static_cast<int>(rect.height() * pixelRatio);
        screenInfo.width = static_cast<int>(rect.width() * pixelRatio);
        screenInfo.name = (*it)->name();
        hTotal += screenInfo.height;
        screensInfo.append(screenInfo);
    }
    return screensInfo;
}

QString Utils::getCurrentAudioChannel()
{
    const QString serviceName {"com.deepin.daemon.Audio"};

    QScopedPointer<com::deepin::daemon::Audio> audioInterface;
    QScopedPointer<com::deepin::daemon::audio::Sink> defaultSink;

    audioInterface.reset(
        new com::deepin::daemon::Audio(
            serviceName,
            "/com/deepin/daemon/Audio",
            QDBusConnection::sessionBus(),
            nullptr)
    );

    defaultSink.reset(
        new com::deepin::daemon::audio::Sink(
            serviceName,
            audioInterface->defaultSink().path(),
            QDBusConnection::sessionBus(),
            nullptr)
    );
    QString str_output = "";
    if (defaultSink->isValid()) {
        QString sinkName = defaultSink->name();
        qDebug() << "系统声卡名称: " << sinkName;
        QStringList options;
        options << "-c";
        options << QString("pacmd list-sources | grep -PB 1 %1 | head -n 1 | perl -pe 's/.* //g'").arg(sinkName);

        QProcess process;
        process.start("bash", options);
        process.waitForFinished();
        process.waitForReadyRead();
        str_output = process.readAllStandardOutput();
        qDebug() << "pacmd命令: " << options;
        qDebug() << "通过pacmd命令获取的系统音频通道号: " << str_output;
        if (str_output.isEmpty()) {
            str_output = audioInterface->defaultSink().path().right(1);
            qDebug() << "通过pacmd命令获取的系统音频通道号失败！自动分配通道号:" << str_output;
        }
        return str_output;
    } else {
        str_output = "1";
        qDebug() << "获取系统音频通道号失败！自动分配通道号" << str_output;
    }
    return str_output;
}

//通过键盘控制光标移动
void Utils::cursorMove(QPoint currentCursor, QKeyEvent *keyEvent)
{
    QPoint pos(currentCursor);
    //qDebug() << "pos() >>>> 1 " << pos;
    if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_A) {
        pos.setX(std::max(pos.x() - 1, 0));
    } else if (keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_D) {
        pos.setX(pos.x() + 1);
    } else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_W) {
        pos.setY(std::max(pos.y() - 1, 0));
    } else if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_S) {
        pos.setY(pos.y() + 1);
    } else {
        qInfo() << "the key is " << keyEvent->key() << " (" << keyEvent->text() << ")";
    }
    //qDebug() << "pos() >>>> 2 " << pos;
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
        WaylandMouseSimulator::instance()->setCursorPoint(QPointF(pos.x() * Utils::pixelRatio, pos.y()*Utils::pixelRatio));
#endif
    } else {
        QCursor::setPos(pos);
    }
}
