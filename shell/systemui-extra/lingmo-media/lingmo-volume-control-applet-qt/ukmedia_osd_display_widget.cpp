/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "ukmedia_osd_display_widget.h"
#include <KWindowEffects>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>
#include "xatom/xatom-helper.h"
#include <QIcon>
#include <QPainterPath>

#include <QPainterPath>

UkmediaOsdDisplayWidget::UkmediaOsdDisplayWidget(QWidget *parent) : QWidget (parent)
{
    setFixedSize(OSDWIDGET_SIZE);

    m_frame = new QFrame(this);
    m_frame->setFixedSize(QSize(72,72));
    m_frame->move(10,10);

    iconLabel = new QLabel(m_frame);
    iconLabel->setFixedSize(QSize(ICON_SIZE));
    iconLabel->move((m_frame->width() - iconLabel->width())/2,(
                          m_frame->height() - iconLabel->height())/2);

    initGsettings();
    initAttribute();
    initPanelGSettings();
}


/**
 * @brief UkmediaOsdDisplayWidget::initAttribute
 * 初始化界面属性
 */
void UkmediaOsdDisplayWidget::initAttribute()
{
    mTimer = new QTimer();
    connect(mTimer,SIGNAL(timeout()),this,SLOT(timeoutHandle()));

    connect(QApplication::primaryScreen(), &QScreen::geometryChanged, this, &UkmediaOsdDisplayWidget::geometryChangedHandle);

    connect(static_cast<QApplication *>(QCoreApplication::instance()),
            &QApplication::primaryScreenChanged, this, &UkmediaOsdDisplayWidget::geometryChangedHandle);

    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool |
                   Qt::WindowStaysOnTopHint |
                   Qt::X11BypassWindowManagerHint |
                   Qt::Popup);

    setAttribute(Qt::WA_TranslucentBackground,true);
    setAutoFillBackground(true);
}


/**
 * @brief UkmediaOsdDisplayWidget::initGsettings
 * 初始化配置文件
 */
void UkmediaOsdDisplayWidget::initGsettings()
{
    if (QGSettings::isSchemaInstalled(THEME)) {
        m_pThemeSetting = new QGSettings(THEME);
        connect(m_pThemeSetting, &QGSettings::changed, this, &UkmediaOsdDisplayWidget::lingmoThemeChangedSlot);
    }
}

void UkmediaOsdDisplayWidget::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
            m_panelGSettings = new QGSettings(id);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }
        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
            if (this->isVisible()) {
                geometryChangedHandle();
            }
        });
    }
}

QRect UkmediaOsdDisplayWidget::caculatePosition()
{
    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;
    int margin = 8;

    switch (m_panelPosition)
    {
    case PanelPosition::Top:
    {
        x = availableGeo.x() + availableGeo.width() - this->width() - margin;
        y = availableGeo.y() + m_panelSize + margin;
    }
        break;
    case PanelPosition::Bottom:
    {
        x = availableGeo.x() + availableGeo.width() - this->width() - margin;
        y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - margin;
    }
        break;
    case PanelPosition::Left:
    {
        x = availableGeo.x() + m_panelSize + margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    case PanelPosition::Right:
    {
        x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    }

    return QRect(x, y, this->width(), this->height());
}

/**
 * @brief UkmediaOsdDisplayWidget::dialogShow
 * 弹出窗口
 */
void UkmediaOsdDisplayWidget::dialogShow()
{
    geometryChangedHandle();
    repaintWidget();
    show();

    mTimer->start(2000);
}

/**
 * @brief UkmediaOsdDisplayWidget::timeoutHandle
 * 超时隐藏窗口
 */
void UkmediaOsdDisplayWidget::timeoutHandle()
{
    hide();
    mTimer->stop();
}

/**
 * @brief UkmediaOsdDisplayWidget::setIcon
 * 设置弹窗图标
 */
void UkmediaOsdDisplayWidget::setIcon(QString iconStr)
{
    mIconName.clear();
    mIconName = iconStr;
}

/**
 * @brief UkmediaOsdDisplayWidget::repaintWidget
 * 重绘窗口
 */
void UkmediaOsdDisplayWidget::repaintWidget()
{
    if(m_pThemeSetting->get("style-name").toString() == "lingmo-light"){
        setPalette(QPalette(QColor("#F5F5F5")));//设置窗口背景
    } else{
        setPalette(QPalette(QColor("#232426")));//设置窗口背景色
    }
    QPixmap m_pixmap = QIcon::fromTheme(mIconName).pixmap(QSize(48,48));
    iconLabel->setPixmap(drawLightColoredPixmap(m_pixmap,m_pThemeSetting->get("style-name").toString()));
}

/**
 * @brief UkmediaOsdDisplayWidget::lingmoThemeChangedSlot
 * 更换系统主题自动刷新界面
 */
void UkmediaOsdDisplayWidget::lingmoThemeChangedSlot(const QString &themeStr)
{
    if (!this->isHidden()) {
        hide();
        repaintWidget();
        geometryChangedHandle();
        show();
    }
}


/**
 * @brief UkmediaOsdDisplayWidget::getGlobalOpacity
 * 获取系统透明度
 */
double UkmediaOsdDisplayWidget::getGlobalOpacity()
{
    double transparency=0.0;
    if(QGSettings::isSchemaInstalled(TRANSPARENCY))
    {
        QGSettings gsetting(TRANSPARENCY);
        if(gsetting.keys().contains(QString("transparency")))
            transparency=gsetting.get("transparency").toDouble();
    }
    return transparency;
}

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
QT_END_NAMESPACE


QPixmap UkmediaOsdDisplayWidget::drawLightColoredPixmap(const QPixmap &source, const QString &style)
{
    int value = 255;
    if(style == "lingmo-light"){
        value = 0;
    }

    QColor gray(255,255,255);
    QColor standard (0,0,0);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(value);
                    color.setGreen(value);
                    color.setBlue(value);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(value);
                    color.setGreen(value);
                    color.setBlue(value);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}


void UkmediaOsdDisplayWidget::geometryChangedHandle()
{
    QRect rect = caculatePosition();
    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        this->setGeometry(QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
    }
    else {
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect((rect.x() - rect.width()*2), rect.y(), rect.width(), rect.height()));
    }
}


void UkmediaOsdDisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 12, 12);
    //画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.20);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();
    //模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 8, false, false);

    //挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);
    //绘制阴影
    painter.drawPixmap(this->rect(), pixmap, pixmap.rect());

    //绘制描边
    QPainterPath linePath;
    linePath.addRoundedRect(QRect(9, 9, m_frame->width()+1, m_frame->height()+1), 12, 12);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::transparent);
    painter.setOpacity(0.20);
    painter.drawPath(linePath);

    //毛玻璃
    qreal opacity = getGlobalOpacity();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(this->palette().base());
    painter.setOpacity(opacity);
    painter.drawPath(linePath);

    KWindowEffects::enableBlurBehind(this->winId(), true,
                                     QRegion(linePath.toFillPolygon().toPolygon()));

    QWidget::paintEvent(event);
}


void UkmediaOsdDisplayWidget::resizeEvent(QResizeEvent* event)
{
//    iconLabel->move((width() - iconLabel->width())/2,(height() - iconLabel->height())/2);
    QWidget::resizeEvent(event);
}

UkmediaOsdDisplayWidget::~UkmediaOsdDisplayWidget()
{
    delete mTimer;
    mTimer = nullptr;
}
