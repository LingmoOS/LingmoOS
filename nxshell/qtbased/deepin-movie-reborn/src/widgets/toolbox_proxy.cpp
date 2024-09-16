// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include "toolbox_proxy.h"
#include "mainwindow.h"
#include "compositing_manager.h"
#include "player_engine.h"
#include "toolbutton.h"
#include "dmr_settings.h"
#include "actions.h"
#include "slider.h"
#include "thumbnail_worker.h"
#include "tip.h"
#include "utils.h"
#include "filefilter.h"

//#include <QtWidgets>
#include <DImageButton>
#include <DThemeManager>
#include <DArrowRectangle>
#include <DApplication>
#include <QThread>
#include <DSlider>
#include <DUtil>
#include <QDBusInterface>
#include <DToolButton>
#include <dthememanager.h>
#include <iostream>
#include "../accessibility/ac-deepin-movie-define.h"
static const int LEFT_MARGIN = 9;
static const int RIGHT_MARGIN = 10;
//static const int PROGBAR_SPEC = 10 + 120 + 17 + 54 + 10 + 54 + 10 + 170 + 10 + 20;

static const QString SLIDER_ARROW = ":resources/icons/slider.svg";

#define POPUP_DURATION 350

DWIDGET_USE_NAMESPACE

//thx  wayland chuang kou bai kuai
#define WAYLAND_BLACK_WINDOW \
    do {\
        auto systemEnv = QProcessEnvironment::systemEnvironment();\
        QString XDG_SESSION_TYPE = systemEnv.value(QStringLiteral("XDG_SESSION_TYPE"));\
        QString WAYLAND_DISPLAY = systemEnv.value(QStringLiteral("WAYLAND_DISPLAY"));\
        if (XDG_SESSION_TYPE == QLatin1String("wayland") ||\
                WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {\
            auto colortype = DGuiApplicationHelper::instance()->themeType();\
            if(colortype == DGuiApplicationHelper::LightType)\
            {\
                QPalette palette(qApp->palette());\
                this->setAutoFillBackground(true);\
                this->setPalette(palette);\
                if(m_pPlaylist)\
                {\
                    QPalette pal(qApp->palette());\
                    m_pPlaylist->setAutoFillBackground(true);\
                    m_pPlaylist->setPalette(pal);\
                }\
                if(m_pEngine )\
                {\
                    QPalette pal(qApp->palette());\
                    m_pEngine->setAutoFillBackground(true);\
                    m_pEngine->setPalette(pal);\
                }\
            }\
            else\
            {\
                QPalette palette(qApp->palette());\
                palette.setColor(QPalette::Background,Qt::black);\
                this->setAutoFillBackground(true);\
                this->setPalette(palette);\
                if(m_pPlaylist)\
                {\
                    QPalette pal(qApp->palette());\
                    pal.setColor(QPalette::Background,Qt::black);\
                    m_pPlaylist->setAutoFillBackground(true);\
                    m_pPlaylist->setPalette(pal);\
                }\
                if(m_pEngine)\
                {\
                    QPalette pal(qApp->palette());\
                    pal.setColor(QPalette::Background,Qt::black);\
                    m_pEngine->setAutoFillBackground(true);\
                    m_pEngine->setPalette(pal);\
                }\
            }\
        }\
    }while(0)

#define THEME_TYPE(colortype) do { \
        if (colortype == DGuiApplicationHelper::LightType){\
            QColor backMaskColor(247, 247, 247);\
            backMaskColor.setAlphaF(0.6);\
            this->blurBackground()->setMaskColor(backMaskColor);\
            bot_widget->setMaskColor(backMaskColor);\
        } else if (colortype == DGuiApplicationHelper::DarkType){\
            QColor backMaskColor(32, 32, 32);\
            backMaskColor.setAlphaF(0.5);\
            blurBackground()->setMaskColor(backMaskColor);\
            bot_widget->setMaskColor(backMaskColor);\
        }\
    } while(0);

namespace dmr {
/**
 * @brief The TooltipHandler class
 * 鼠标悬停事件过滤器
 */
class TooltipHandler: public QObject
{
public:
    /**
     * @brief TooltipHandler 构造函数
     * @param parent 父窗口
     */
    explicit TooltipHandler(QObject *parent): QObject(parent) {}

protected:
    /**
     * @brief eventFilter 事件过滤器
     * @param obj 事件对象
     * @param event 事件
     * @return 返回是否继续执行
     */
    bool eventFilter(QObject *obj, QEvent *event)
    {
        switch (event->type()) {
        case QEvent::ToolTip:
        case QEvent::Enter: {
            //QHelpEvent *he = static_cast<QHelpEvent *>(event);
            auto tip = obj->property("HintWidget").value<Tip *>();
            auto btn = tip->property("for").value<QWidget *>();
            tip->setText(btn->toolTip());
            tip->show();
            tip->raise();
            tip->adjustSize();

            QPoint pos = btn->parentWidget()->mapToGlobal(btn->pos());
            pos.rx() = pos.x() + (btn->width() - tip->width()) / 2;
            pos.ry() = pos.y() - 40;
            tip->move(pos);
            return true;
        }

        case QEvent::Leave: {
            auto parent = obj->property("HintWidget").value<Tip *>();
            parent->hide();
            event->ignore();
            break;
        }
        case QEvent::MouseMove: {
            QHelpEvent *he = static_cast<QHelpEvent *>(event);
            auto tip = obj->property("HintWidget").value<Tip *>();
            tip->hide();
        }
        default:
            break;
        }
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
};
/**
 * @brief The SliderTime class 进度条事件显示类
 */
class SliderTime: public DArrowRectangle
{
    Q_OBJECT
public:
    /**
     * @brief SliderTime 构造函数
     */
    SliderTime(): DArrowRectangle(DArrowRectangle::ArrowBottom)
    {
        initMember();

        //setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlag(Qt::WindowStaysOnTopHint);
        resize(m_miniSize);
        setRadius(8);
        setArrowWidth(10);
        setArrowHeight(5);
        const QPalette pal = QGuiApplication::palette();
        QColor bgColor = pal.color(QPalette::Highlight);
        setBorderWidth(1);
        setBorderColor(bgColor);
        setBackgroundColor(bgColor);

        QHBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 5);
        m_pTime = new DLabel(this);
        m_pTime->setAlignment(Qt::AlignCenter);
//        _time->setFixedSize(_size);
        m_pTime->setForegroundRole(DPalette::Text);
        DPalette pa = DApplicationHelper::instance()->palette(m_pTime);
        QColor color = pa.textLively().color();
        qInfo() << color.name();
        pa.setColor(DPalette::Text, color);
        m_pTime->setPalette(pa);
        m_pTime->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T8));
        layout->addWidget(m_pTime, Qt::AlignCenter);
        setLayout(layout);
        connect(qApp, &QGuiApplication::fontChanged, this, &SliderTime::slotFontChanged);

    }

    /**
     * @brief setTime 设置时间
     * @param time 时间
     */
    void setTime(const QString &time)
    {
        m_pTime->setText(time);

        if (!m_bFontChanged) {
            QFontMetrics fontMetrics(DFontSizeManager::instance()->get(DFontSizeManager::T8));
            m_pTime->setFixedSize(fontMetrics.width(m_pTime->text()) + 5, fontMetrics.height());
        } else {
            QFontMetrics fontMetrics(m_font);
            m_pTime->setFont(m_font);
            m_pTime->setFixedSize(fontMetrics.width(m_pTime->text()) + 10, fontMetrics.height());
        }
        this->setWidth(m_pTime->width());
        this->setHeight(m_pTime->height() + 5);
        this->setMinimumSize(m_miniSize);
    }
public slots:
    /**
     * @brief slotFontChanged 字体变化槽函数
     * @param font 字体
     */
    void slotFontChanged(const QFont &font)
    {
        m_font = font;
        m_bFontChanged = true;
    }
private:
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember()
    {
        m_pTime = nullptr;
        m_miniSize = QSize(58, 25);
        m_font = {QFont()};
        m_bFontChanged = false;
    }

    DLabel *m_pTime;
    QSize m_miniSize;
    QFont m_font;
    bool m_bFontChanged;
};

/**
 * @brief The ViewProgBar class 胶片模式窗口
 */
class ViewProgBar: public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief ViewProgBar 构造函数
     * @param m_pProgBar 进度条
     * @param parent 父窗口
     */
    ViewProgBar(DMRSlider *m_pProgBar, QWidget *parent = nullptr)
    {
        initMemeber();
        //传入进度条，以便重新获取胶片进度条长度 by ZhuYuliang
        this->m_pProgBar = m_pProgBar;
        _parent = parent;
        setFixedHeight(TOOLBOX_HEIGHT);
        setContentsMargins(0, 0, 0, 0);

        m_bIsBlockSignals = false;
        setMouseTracking(true);

        m_pBack = new QWidget(this);
        m_pBack->setFixedHeight(50);
        m_pBack->setFixedWidth(this->width());
        m_pBack->setContentsMargins(0, 0, 0, 0);

        m_pFront = new QWidget(this);
        m_pFront->setFixedHeight(50);
        m_pFront->setFixedWidth(0);
        m_pFront->setContentsMargins(0, 0, 0, 0);

        m_pIndicator = new IndicatorItem(this);
        m_pIndicator->resize(5, 52);
        m_pIndicator->setObjectName("indicator");

        m_pSliderTime = new SliderTime;
        m_pSliderTime->hide();

        QMatrix matrix;
        matrix.rotate(180);
        QPixmap pixmap = utils::LoadHiDPIPixmap(SLIDER_ARROW);
        m_pSliderArrowUp = new DArrowRectangle(DArrowRectangle::ArrowTop);
        //m_pSliderArrowUp->setFocusPolicy(Qt::NoFocus);
        m_pSliderArrowUp->setAttribute(Qt::WA_DeleteOnClose);
        m_pSliderArrowUp->setWindowFlag(Qt::WindowStaysOnTopHint);
        m_pSliderArrowUp->setArrowWidth(10);
        m_pSliderArrowUp->setArrowHeight(7);
        const QPalette pa = QGuiApplication::palette();
        QColor bgColor = pa.color(QPalette::Highlight);
        m_pSliderArrowUp->setBackgroundColor(bgColor);
        m_pSliderArrowUp->setFixedSize(10, 7);
        m_pSliderArrowUp->hide();
        m_pSliderArrowDown = new DLabel(this);
        m_pSliderArrowDown->setFixedSize(20, 18);
        m_pSliderArrowDown->setPixmap(pixmap.transformed(matrix, Qt::SmoothTransformation));
        m_pSliderArrowDown->hide();

        m_pBack->setMouseTracking(true);
        m_pFront->setMouseTracking(true);
        m_pIndicator->setMouseTracking(true);

        m_pViewProgBarLayout = new QHBoxLayout(m_pBack);
        m_pViewProgBarLayout->setContentsMargins(0, 0, 0, 0);
        m_pBack->setLayout(m_pViewProgBarLayout);

        m_pViewProgBarLayout_black = new QHBoxLayout(m_pFront);
        m_pViewProgBarLayout_black->setContentsMargins(0, 0, 0, 0);
        m_pFront->setLayout(m_pViewProgBarLayout_black);
    }
//    virtual ~ViewProgBar();
    void setIsBlockSignals(bool isBlockSignals)
    {
        m_bIsBlockSignals = isBlockSignals;
    }
    bool getIsBlockSignals()
    {
        return  m_bPress ? true: m_bIsBlockSignals;
    }
    void setValue(int v)
    {
        if (v < m_nStartPoint) {
            v = m_nStartPoint;
        } else if (v > (m_nStartPoint + m_nViewLength)) {
            v = (m_nStartPoint + m_nViewLength);
        }
        m_IndicatorPos = {v, rect().y()};
        update();
    }
    int getValue()
    {
        return m_pIndicator->x();
    }
    int getTimePos()
    {
        return position2progress(QPoint(m_pIndicator->x(), 0));
    }
    void setTime(qint64 pos)
    {
        QTime time(0, 0, 0);
        QString strTime = time.addSecs(static_cast<int>(pos)).toString("hh:mm:ss");
        m_pSliderTime->setTime(strTime);
    }
    void setTimeVisible(bool visible)
    {
        if (visible) {
            auto pos = this->mapToGlobal(QPoint(0, 0));
            m_pSliderTime->show(pos.x() + m_IndicatorPos.x() + 1, pos.y() + m_IndicatorPos.y() + 4);
        } else {
            m_pSliderTime->hide();
        }
    }
    /**
     * @brief setViewProgBar 设置胶片模式位置
     * @param pEngine 播放引擎对象指针
     * @param pmList 彩色胶片图像列表
     * @param pmBlackList 灰色胶片图像列表
     */
    void setViewProgBar(PlayerEngine *pEngine, QList<QPixmap> pmList, QList<QPixmap> pmBlackList)
    {
        m_pEngine = pEngine;

        m_pViewProgBarLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_pViewProgBarLayout->setSpacing(1);

        /*这段代码将胶片添加到两个label中，一个label置灰，一个彩色，通过光标调整两个label的位置
         *以实现通过光标来显示播放过的位置
         */
        const int nPixWidget = 42/*m_pProgBar->width() / 100*/;
        m_nViewLength = (nPixWidget + 1) * pmList.count() - 1;
        m_nStartPoint = (m_pProgBar->width() - m_nViewLength) / 2; //开始位置
        for (int i = 0; i < pmList.count(); i++) {
            ImageItem *label = new ImageItem(pmList.at(i), false, m_pBack);
            label->setMouseTracking(true);
            label->move(i * (nPixWidget + 1) + m_nStartPoint, 5);
            label->setFixedSize(nPixWidget, 42);

            ImageItem *label_black = new ImageItem(pmBlackList.at(i), true, m_pFront);
            label_black->setMouseTracking(true);
            label_black->move(i * (nPixWidget + 1) + m_nStartPoint, 5);
            label_black->setFixedSize(nPixWidget, 42);
        }
        update();
    }
    void clear()
    {
        foreach (QLabel *label, m_pFront->findChildren<QLabel *>()) {
            if (label) {
                label->deleteLater();
                label = nullptr;
            }
        }

        foreach (QLabel *label, m_pBack->findChildren<QLabel *>()) {
            if (label) {
                label->deleteLater();
                label = nullptr;
            }
        }

        m_pSliderTime->setVisible(false);
        m_pSliderArrowDown->setVisible(false);
        m_pSliderArrowUp->setVisible(false);
        // 清除状态时还原初始显示状态
        m_bPress = false;
        m_pIndicator->setPressed(m_bPress);
        m_pIndicator->resize(5, 52);
    }

    int getViewLength()
    {
        return m_nViewLength;
    }

    int getStartPoint()
    {
        return m_nStartPoint;
    }

private:
    void changeStyle(bool press)
    {
        if (!isVisible()) return;

        if (press) {
            m_pIndicator->setPressed(press);
            m_pIndicator->resize(2, 52);

        } else {
            m_pIndicator->setPressed(press);
            m_pIndicator->resize(5, 52);
        }
    }

signals:
    void leave();
    void hoverChanged(int);
    void sliderMoved(int);
    void mousePressed(bool pressed);

protected:
    void leaveEvent(QEvent *e) override
    {
        emit leave();

        DWidget::leaveEvent(e);
    }
    void showEvent(QShowEvent *se) override
    {
        DWidget::showEvent(se);
    }
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!isEnabled()) return;

        if (e->pos().x() >= 0 && e->pos().x() <= contentsRect().width()) {
            int v = position2progress(e->pos());
            if (e->buttons() & Qt::LeftButton) {
                int distance = (e->pos() -  m_startPos).manhattanLength();
                if (distance >= QApplication::startDragDistance()) {
                    emit sliderMoved(v);
                    emit hoverChanged(v);
                    emit mousePressed(true);
                    setValue(e->pos().x());
                    setTime(v);
                    repaint();
                }
            } else {
                if (m_nVlastHoverValue != v) {
                    emit hoverChanged(v);
                }
                m_nVlastHoverValue = v;
            }
        }
        e->accept();
    }
    void mousePressEvent(QMouseEvent *e) override
    {
        if (!m_bPress && e->buttons() == Qt::LeftButton && isEnabled()) {
            m_startPos = e->pos();

            int v = position2progress(e->pos());
            emit sliderMoved(v);
            emit hoverChanged(v);
            emit mousePressed(true);
            setValue(e->pos().x());
            changeStyle(!m_bPress);
            m_bPress = !m_bPress;
        }
    }
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        emit mousePressed(false);
        if (m_bPress && isEnabled()) {
            changeStyle(!m_bPress);
            m_bPress = !m_bPress;
            //鼠标释放时seek视频位置。
            int v = position2progress(e->pos());
            m_pEngine->seekAbsolute(v);
        }

        m_pSliderArrowUp->setVisible(m_bPress);
        setTimeVisible(m_bPress);

        DWidget::mouseReleaseEvent(e);
    }
    void paintEvent(QPaintEvent *e) override
    {
        m_pIndicator->move(m_IndicatorPos.x(), m_IndicatorPos.y());
        QPoint pos = this->mapToGlobal(QPoint(0, 0));
        m_pSliderArrowUp->move(pos.x() + m_IndicatorPos.x() + 1, pos.y() + m_pIndicator->height() - 5);
        m_pFront->setFixedWidth(m_IndicatorPos.x());

        m_pSliderArrowUp->setVisible(m_bPress);
        setTimeVisible(m_bPress);

        DWidget::paintEvent(e);
    }
    void resizeEvent(QResizeEvent *event) override
    {
        m_pBack->setFixedWidth(this->width());

        DWidget::resizeEvent(event);
    }
private:
    int  position2progress(const QPoint &p)
    {
        int nPosition = 0;

        if (!m_pEngine) {
            return 0;
        }

        if (p.x() < m_nStartPoint) {
            nPosition = m_nStartPoint;
        } else if (p.x() > (m_nViewLength + m_nStartPoint)) {
            nPosition = (m_nViewLength + m_nStartPoint);
        } else {
            nPosition = p.x();
        }

        auto total = m_pEngine->duration();
        int span = static_cast<int>(total * (nPosition - m_nStartPoint) / m_nViewLength);
        return span/* * (p.x())*/;
    }

    void initMemeber()
    {
        m_pEngine = nullptr;
        _parent = nullptr;
//        m_pViewProgBarLoad = nullptr;
        m_pBack = nullptr;
        m_pFront = nullptr;
        m_pIndicator  = nullptr;
        m_pSliderTime = nullptr;
        m_pSliderArrowDown = nullptr;
        m_pSliderArrowUp = nullptr;
        m_pIndicatorLayout = nullptr;
        m_pViewProgBarLayout = nullptr;
        m_pViewProgBarLayout_black = nullptr;
        m_pProgBar = nullptr;
        m_nViewLength = 0;
        m_nStartPoint = 0;
        m_nVlastHoverValue = 0;
        m_startPos = QPoint(0, 0);
        m_IndicatorPos  = QPoint(0, 0);
        m_bPress = false;
        m_bIsBlockSignals = false;
    }

    PlayerEngine *m_pEngine;
    QWidget *_parent;
    int  m_nVlastHoverValue;
    QPoint  m_startPos;
    bool  m_bIsBlockSignals;
    QPoint m_IndicatorPos;
    QColor _indicatorColor;
//    viewProgBarLoad *m_pViewProgBarLoad;
    QWidget *m_pBack;
    QWidget *m_pFront;
    IndicatorItem *m_pIndicator;
    SliderTime *m_pSliderTime;
    DLabel *m_pSliderArrowDown;
    DArrowRectangle *m_pSliderArrowUp;
    bool m_bPress;
    QHBoxLayout *m_pIndicatorLayout;
    QHBoxLayout *m_pViewProgBarLayout;
    QHBoxLayout *m_pViewProgBarLayout_black;
    DMRSlider *m_pProgBar;
    int m_nViewLength;
    int m_nStartPoint;
};

class ThumbnailPreview: public QWidget
{
    Q_OBJECT
public:
    ThumbnailPreview()
    {
        setAttribute(Qt::WA_DeleteOnClose);
        // FIXME(hualet): Qt::Tooltip will cause Dock to show up even
        // the player is in fullscreen mode.
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setObjectName("ThumbnailPreview");

        m_pWMDBus = new QDBusInterface("com.deepin.WMSwitcher", "/com/deepin/WMSwitcher", "com.deepin.WMSwitcher", QDBusConnection::sessionBus());
        QDBusReply<QString> reply = m_pWMDBus->call("CurrentWM");
        m_bIsWM = reply.value().contains("deepin wm");
        connect(m_pWMDBus, SIGNAL(WMChanged(QString)), this, SLOT(slotWMChanged(QString)));

        auto *l = new QVBoxLayout;
        l->setContentsMargins(0, 0, 0, 0);

        _thumb = new DFrame(this);
        if (m_bIsWM) {
            DStyle::setFrameRadius(_thumb, 8);
        } else {
            DStyle::setFrameRadius(_thumb, 0);
        }

        //_thumb->setFixedSize(ThumbnailWorker::thumbSize());
        l->addWidget(_thumb/*,Qt::AlignTop*/);
        setLayout(l);
//        winId(); // force backed window to be created
        m_shadow_effect = new QGraphicsDropShadowEffect(this);
    }

    void updateWithPreview(const QPixmap &pm, qint64 secs, int rotation)
    {
        QPixmap rounded;
        if (m_bIsWM) {
            rounded = utils::MakeRoundedPixmap(pm, 4, 4, secs, rotation);
        } else {
            rounded = pm;
        }

        if (rounded.width() == 0)
            return;

        if (rounded.width() > rounded.height()) {
            static int roundedH = static_cast<int>(
                                      (static_cast<double>(m_thumbnailFixed)
                                       / static_cast<double>(rounded.width()))
                                      * rounded.height());
            QSize size(m_thumbnailFixed, roundedH);
            resizeThumbnail(rounded, size);
        } else {
            static int roundedW = static_cast<int>(
                                      (static_cast<double>(m_thumbnailFixed)
                                       / static_cast<double>(rounded.height()))
                                      * rounded.width());
            QSize size(roundedW, m_thumbnailFixed);
            resizeThumbnail(rounded, size);
        }

        QImage image;
        QPalette palette;
        image = rounded.toImage();

        auto dpr = qApp->devicePixelRatio();
        palette.setBrush(_thumb->backgroundRole(),
                         QBrush(image.scaled(// 缩放背景图.
                                    _thumb->size() * dpr,
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation)));
        _thumb->setPalette(palette);
    }

    void updateWithPreview(const QPoint &pos)
    {
        move(pos.x() - this->width() / 2, pos.y() - this->height() + 10);

        show();
    }
public slots:
    void slotWMChanged(QString msg)
    {
        if (msg.contains("deepin metacity")) {
            m_bIsWM = false;
            DStyle::setFrameRadius(_thumb, 0);
        } else {
            m_bIsWM = true;
            DStyle::setFrameRadius(_thumb, 8);
        }
    }

signals:
    void leavePreview();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE{
        m_shadow_effect->setOffset(0, 0);
        m_shadow_effect->setColor(Qt::gray);
        m_shadow_effect->setBlurRadius(8);
        setGraphicsEffect(m_shadow_effect);
        if (!m_bIsWM)
        {
            QPainter painter(this);
            QPainterPath path;
            path.addRect(rect());
            painter.fillPath(path, QColor(230, 230, 230));
        }

        QWidget::paintEvent(e);
    }
    void leaveEvent(QEvent *e) override
    {
        emit leavePreview();
    }

    void showEvent(QShowEvent *se) override
    {
        QWidget::showEvent(se);
    }

private:
    void resizeThumbnail(QPixmap &pixmap, const QSize &size)
    {
        auto dpr = qApp->devicePixelRatio();
        pixmap.setDevicePixelRatio(dpr);
        pixmap = pixmap.scaled(size * dpr, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(dpr);
        _thumb->setFixedSize(size);
        int offect = 2;
        if (!m_bIsWM) {
            offect = 0;
        }
        this->setFixedWidth(_thumb->width() + offect);
        this->setFixedHeight(_thumb->height() + offect);
    }

private:
    DFrame *_thumb {nullptr};
    int m_thumbnailFixed = 106;
    QGraphicsDropShadowEffect *m_shadow_effect{nullptr};
    QDBusInterface *m_pWMDBus{nullptr};
    bool m_bIsWM{false};
};

static QString libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(strlib)) {
        return strlib;
    } else {
        list.sort();
    }

    Q_ASSERT(list.size() > 0);
    return list.last();
}

void viewProgBarLoad::initThumb()
{
    QLibrary library(CompositingManager::libPath("libffmpegthumbnailer.so"));
    m_mvideo_thumbnailer = (mvideo_thumbnailer) library.resolve("video_thumbnailer_create");
    m_mvideo_thumbnailer_destroy = (mvideo_thumbnailer_destroy) library.resolve("video_thumbnailer_destroy");
    m_mvideo_thumbnailer_create_image_data = (mvideo_thumbnailer_create_image_data) library.resolve("video_thumbnailer_create_image_data");
    m_mvideo_thumbnailer_destroy_image_data = (mvideo_thumbnailer_destroy_image_data) library.resolve("video_thumbnailer_destroy_image_data");
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer = (mvideo_thumbnailer_generate_thumbnail_to_buffer) library.resolve("video_thumbnailer_generate_thumbnail_to_buffer");

    if (m_mvideo_thumbnailer == nullptr || m_mvideo_thumbnailer_destroy == nullptr
            || m_mvideo_thumbnailer_create_image_data == nullptr || m_mvideo_thumbnailer_destroy_image_data == nullptr
            || m_mvideo_thumbnailer_generate_thumbnail_to_buffer == nullptr)
    {
        return;
    }

    m_video_thumbnailer = m_mvideo_thumbnailer();
}

void viewProgBarLoad::initMember()
{
    m_pEngine = nullptr;
    m_pParent = nullptr;
    m_pProgBar = nullptr;
    m_pListPixmapMutex = nullptr;
}

void viewProgBarLoad::loadViewProgBar(QSize size)
{
    int pixWidget =  40;
    int num = int(m_pProgBar->width() / (40 + 1)); //number of thumbnails
    int tmp = (num == 0) ? 0: (m_pEngine->duration() * 1000) / num;

    QList<QPixmap> pmList;
    QList<QPixmap> pmBlackList;

    QTime time(0, 0, 0, 0);
    if (m_pEngine->videoSize().width() > 0 && m_pEngine->videoSize().height() > 0) {
        m_video_thumbnailer->thumbnail_size = (static_cast<int>(50 * (m_pEngine->videoSize().width() / m_pEngine->videoSize().height() * 50)
                                                                * qApp->devicePixelRatio()));
    }

    if (m_image_data == nullptr) {
        m_image_data = m_mvideo_thumbnailer_create_image_data();
    }

//    m_video_thumbnailer->seek_time = d.toString("hh:mm:ss").toLatin1().data();
    int length = strlen(time.toString("hh:mm:ss").toLatin1().data());
    memcpy(m_seekTime, time.toString("hh:mm:ss").toLatin1().data(), length + 1);
    m_video_thumbnailer->seek_time = m_seekTime;

    auto url = m_pEngine->playlist().currentInfo().url;
    auto file = QFileInfo(url.toLocalFile()).absoluteFilePath();

    for (auto i = 0; i < num ; i++) {
        if (isInterruptionRequested()) {
            qInfo() << "isInterruptionRequested";
            return;
        }

        time = time.addMSecs(tmp);

//        m_video_thumbnailer->seek_time = d.toString("hh:mm:ss").toLatin1().data();
        memcpy(m_seekTime, time.toString("hh:mm:ss").toLatin1().data(), length + 1);
        m_video_thumbnailer->seek_time = m_seekTime;
        try {

            m_mvideo_thumbnailer_generate_thumbnail_to_buffer(m_video_thumbnailer, file.toUtf8().data(),  m_image_data);
            auto img = QImage::fromData(m_image_data->image_data_ptr, static_cast<int>(m_image_data->image_data_size), "png");
            if (img.format() == QImage::Format_Invalid) {
                return;
            }
            auto img_tmp = img.scaledToHeight(50);


            pmList.append(QPixmap::fromImage(img_tmp.copy(img_tmp.size().width() / 2 - 4, 0, pixWidget, 50))); //-2 为了1px的内边框
            QImage img_black = img_tmp.convertToFormat(QImage::Format_Grayscale8);
            pmBlackList.append(QPixmap::fromImage(img_black.copy(img_black.size().width() / 2 - 4, 0, pixWidget, 50)));

        } catch (const std::logic_error &) {

        }
    }

    m_mvideo_thumbnailer_destroy_image_data(m_image_data);
    m_image_data = nullptr;

    m_pListPixmapMutex->lock();
    m_pParent->addpmList(pmList);
    m_pParent->addpmBlackList(pmBlackList);
    m_pListPixmapMutex->unlock();
    emit sigFinishiLoad(size);
//    emit finished();
}
/**
 * @brief ToolboxProxy 构造函数
 * @param mainWindow 主窗口
 * @param pPlayerEngine 播放引擎对象指针
 */
ToolboxProxy::ToolboxProxy(QWidget *mainWindow, PlayerEngine *proxy)
    : DFloatingWidget(mainWindow),
      m_pMainWindow(static_cast<MainWindow *>(mainWindow)),
      m_pEngine(proxy)
{
    initMember();
    if (utils::check_wayland_env()) {
        m_bThumbnailmode = false;
    }

    m_pPreviewer = new ThumbnailPreview;
    m_pPreviewTime = new SliderTime;
    m_pPreviewTime->hide();
    m_mircastWidget = new MircastWidget(mainWindow, proxy);
    m_mircastWidget->hide();

    setup();
    slotThemeTypeChanged();

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged,
            this, &ToolboxProxy::updatePlayState);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged,
            this, &ToolboxProxy::updateplaylisticon);
    connect(m_mircastWidget, &MircastWidget::updatePlayStatus, this, &ToolboxProxy::updatePlayState);
    connect(m_mircastWidget, &MircastWidget::updateTime, this, &ToolboxProxy::updateMircastTime, Qt::QueuedConnection);
}
void ToolboxProxy::finishLoadSlot(QSize size)
{
    qInfo() << "thumbnail has finished";

    if (m_pmList.isEmpty()) return;
    if (!m_bThumbnailmode) {
        return;
    }
    m_pViewProgBar->setViewProgBar(m_pEngine, m_pmList, m_pmBlackList);

    if(CompositingManager::get().platform() == Platform::X86) {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            PlayItemInfo info = m_pEngine->playlist().currentInfo();
            if (!info.url.isLocalFile()) {
                return;
            }
            m_pProgBar_Widget->setCurrentIndex(2);
        }
    }
}

void ToolboxProxy::setthumbnailmode()
{
    if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
        return;
    }

    if(CompositingManager::get().platform() == Platform::X86  && CompositingManager::isMpvExists()) {
        if (Settings::get().isSet(Settings::ShowThumbnailMode)) {
            m_bThumbnailmode = true;
            updateThumbnail();
        } else {
            m_bThumbnailmode = false;
            updateMovieProgress();
            m_pProgBar_Widget->setCurrentIndex(1);   //恢复进度条模式 by zhuyuliang
        }
    } else {
        //no thunbnail progress bar is loaded except amd plantform
        m_bThumbnailmode = false;
        updateMovieProgress();
    }

}

void ToolboxProxy::updateplaylisticon()
{
    if (m_pListBtn->isChecked() && DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        m_pListBtn->setIcon(QIcon(":/icons/deepin/builtin/light/checked/episodes_checked.svg"));
    } else {
        m_pListBtn->setIcon(QIcon::fromTheme("dcc_episodes"));
    }

    if (m_pMircastBtn->isChecked() && DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        m_pMircastBtn->setIcon(QIcon(":/icons/deepin/builtin/light/checked/mircast_chenked.svg"));
    } else {
        m_pMircastBtn->setIcon(QIcon::fromTheme("dcc_mircast"));
    }
}

void ToolboxProxy::setup()
{
    QStackedLayout *stacked = new QStackedLayout(this);
    stacked->setContentsMargins(0, 0, 0, 0);
    stacked->setStackingMode(QStackedLayout::StackAll);
    setLayout(stacked);

    this->setBlurBackgroundEnabled(true);
    this->blurBackground()->setRadius(30);
    this->blurBackground()->setBlurEnabled(true);
    this->blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);

    bot_widget = new DBlurEffectWidget(this);
    bot_widget->setObjectName(BOTTOM_WIDGET);
    bot_widget->setBlurRectXRadius(18);
    bot_widget->setBlurRectYRadius(18);
    bot_widget->setRadius(30);
    bot_widget->setBlurEnabled(true);
    bot_widget->setMode(DBlurEffectWidget::GaussianBlur);
    auto type = DGuiApplicationHelper::instance()->themeType();
    THEME_TYPE(type);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &ToolboxProxy::slotThemeTypeChanged);

//    bot_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    bot_widget->setFixedHeight(TOOLBOX_HEIGHT);
    QVBoxLayout *botv = new QVBoxLayout(bot_widget);
    botv->setContentsMargins(0, 0, 0, 0);
    botv->setSpacing(0);

    m_pBotSpec = new QWidget(bot_widget);
    m_pBotSpec->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_pBotSpec->setFixedWidth(width());
    m_pBotSpec->setVisible(false);
    botv->addWidget(m_pBotSpec);
    botv->addStretch();

    m_pBotToolWgt = new QWidget(bot_widget);
    m_pBotToolWgt->setObjectName(BOTTOM_TOOL_BUTTON_WIDGET);
    m_pBotToolWgt->setFixedHeight(TOOLBOX_HEIGHT);
    m_pBotToolWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QHBoxLayout *bot_layout = new QHBoxLayout(m_pBotToolWgt);
    bot_layout->setContentsMargins(LEFT_MARGIN, 0, RIGHT_MARGIN, 0);
    bot_layout->setSpacing(0);
    m_pBotToolWgt->setLayout(bot_layout);
    botv->addWidget(m_pBotToolWgt);

    bot_widget->setLayout(botv);
    stacked->addWidget(bot_widget);

    m_pTimeLabel = new QLabel(m_pBotToolWgt);
    m_pTimeLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_pTimeLabel, DFontSizeManager::T6);
    m_pTimeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_pTimeLabelend = new QLabel(m_pBotToolWgt);
    m_pTimeLabelend->setAlignment(Qt::AlignCenter);
    m_pTimeLabelend->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    DFontSizeManager::instance()->bind(m_pTimeLabelend, DFontSizeManager::T6);

    m_pProgBar = new DMRSlider(m_pBotToolWgt);
    m_pProgBar->setObjectName(MOVIE_PROGRESS_WIDGET);
    m_pProgBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_pProgBar->slider()->setObjectName(PROGBAR_SLIDER);
    m_pProgBar->slider()->setAccessibleName(PROGBAR_SLIDER);
    m_pProgBar->slider()->setOrientation(Qt::Horizontal);
    m_pProgBar->slider()->setFocusPolicy(Qt::TabFocus);
    m_pProgBar->slider()->setRange(0, 100);
    m_pProgBar->setValue(0);
    m_pProgBar->setEnableIndication(m_pEngine->state() != PlayerEngine::Idle);

    connect(m_pProgBar, &DMRSlider::sigUnsupported, this, &ToolboxProxy::sigUnsupported);
    connect(m_pPreviewer, &ThumbnailPreview::leavePreview, this, &ToolboxProxy::slotLeavePreview);
    connect(&Settings::get(), &Settings::baseChanged, this, &ToolboxProxy::setthumbnailmode);
    connect(m_pEngine, &PlayerEngine::siginitthumbnailseting, this, &ToolboxProxy::setthumbnailmode);

    //刷新显示预览当前时间的label
    connect(m_pProgBar, &DMRSlider::hoverChanged, this, &ToolboxProxy::progressHoverChanged);
    connect(m_pProgBar, &DMRSlider::sliderMoved, this, &ToolboxProxy::progressHoverChanged);
    connect(m_pProgBar, &DMRSlider::leave, this, &ToolboxProxy::slotHidePreviewTime);

    connect(m_pProgBar, &DMRSlider::sliderPressed, this, &ToolboxProxy::slotSliderPressed);
    connect(m_pProgBar, &DMRSlider::sliderReleased, this, &ToolboxProxy::slotSliderReleased);
    connect(&Settings::get(), &Settings::baseMuteChanged, this, &ToolboxProxy::slotBaseMuteChanged);

    m_pViewProgBar = new ViewProgBar(m_pProgBar, m_pBotToolWgt);


    //刷新显示预览当前时间的label
    connect(m_pViewProgBar, &ViewProgBar::hoverChanged, this, &ToolboxProxy::progressHoverChanged);
    connect(m_pViewProgBar, &ViewProgBar::leave, this, &ToolboxProxy::slotHidePreviewTime);
    connect(m_pViewProgBar, &ViewProgBar::mousePressed, this, &ToolboxProxy::updateTimeVisible);

    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, static_cast<void(QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped),
            this, &ToolboxProxy::buttonClicked);

    _mid = new QHBoxLayout(m_pBotToolWgt);
    _mid->setContentsMargins(0, 0, 0, 0);
    _mid->setSpacing(0);
    _mid->setAlignment(Qt::AlignLeft);
    bot_layout->addLayout(_mid);

    QHBoxLayout *time = new QHBoxLayout(m_pBotToolWgt);
    time->setContentsMargins(10, 9, 11, 9);
    time->setSpacing(0);
    time->setAlignment(Qt::AlignLeft);
    bot_layout->addLayout(time);
    time->addWidget(m_pTimeLabel);
    QHBoxLayout *progBarspec = new QHBoxLayout(m_pBotToolWgt);
    progBarspec->setContentsMargins(0, 2, 0, 0);
    progBarspec->setSpacing(0);
    progBarspec->setAlignment(Qt::AlignHCenter);

    if (utils::check_wayland_env()) {
        //lmh0706,延时
        connect(m_pNextBtn, &DButtonBoxButton::clicked, this, &ToolboxProxy::waitPlay);
        connect(m_pPlayBtn, &DButtonBoxButton::clicked, this, &ToolboxProxy::waitPlay);
        connect(m_pPrevBtn, &DButtonBoxButton::clicked, this, &ToolboxProxy::waitPlay);
    }

    m_pProgBar_Widget = new QStackedWidget(m_pBotToolWgt);
    m_pProgBar_Widget->setObjectName(PROGBAR_WIDGET);
    m_pProgBar_Widget->setAccessibleName(PROGBAR_WIDGET);
    m_pProgBar_Widget->setContentsMargins(0, 0, 0, 0);
    m_pProgBar_Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_pProgBarspec = new DWidget(m_pProgBar_Widget);
    m_pProgBarspec->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_pProgBar_Widget->addWidget(m_pProgBarspec);
    m_pProgBar_Widget->addWidget(m_pProgBar);
    m_pProgBar_Widget->addWidget(m_pViewProgBar);
    m_pProgBar_Widget->setCurrentIndex(0);
    progBarspec->addWidget(m_pProgBar_Widget);
    bot_layout->addLayout(progBarspec);

    QHBoxLayout *timeend = new QHBoxLayout(m_pBotToolWgt);
    timeend->setContentsMargins(10, 10, 10, 10);
    timeend->setSpacing(0);
    timeend->setAlignment(Qt::AlignRight);
    bot_layout->addLayout(timeend);
    timeend->addWidget(m_pTimeLabelend);

    QList<DButtonBoxButton *> list;
    m_pPrevBtn = new ButtonBoxButton("", this);
    m_pPlayBtn = new ButtonBoxButton("", this);
    m_pNextBtn = new ButtonBoxButton("", this);
    m_pVolBtn = new VolumeButton();

    _mid->addWidget(m_pPrevBtn);
    _mid->addWidget(m_pPlayBtn);
    _mid->addWidget(m_pNextBtn);
    _mid->addWidget(m_pVolBtn);

    m_pPrevBtn->setIcon(QIcon::fromTheme("dcc_last", QIcon(":/icons/deepin/builtin/light/normal/last_normal.svg")));
    m_pPrevBtn->setIconSize(QSize(36, 36));
    m_pPrevBtn->setFixedSize(TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_WIDTH);
    m_pPrevBtn->setObjectName(PREV_BUTTON);
    m_pPrevBtn->setAccessibleName(PREV_BUTTON);
    m_pPrevBtn->setFocusPolicy(Qt::TabFocus);
    connect(m_pPrevBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pPrevBtn, "prev");
    list.append(m_pPrevBtn);

    m_pPlayBtn->setIcon(QIcon::fromTheme("dcc_play", QIcon(":/icons/deepin/builtin/light/normal/play_normal.svg")));
    m_pPlayBtn->setIconSize(QSize(36, 36));
    m_pPlayBtn->setFixedSize(TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_WIDTH);
    m_pPlayBtn->setFocusPolicy(Qt::TabFocus);
    m_pPlayBtn->setObjectName(PLAY_BUTTON);
    m_pPlayBtn->setAccessibleName(PLAY_BUTTON);
    connect(m_pPlayBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pPlayBtn, "play");
    list.append(m_pPlayBtn);

    m_pNextBtn->setIcon(QIcon::fromTheme("dcc_next", QIcon(":/icons/deepin/builtin/light/normal/next_normal.svg")));
    m_pNextBtn->setIconSize(QSize(36, 36));
    m_pNextBtn->setFixedSize(TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_WIDTH);
    m_pNextBtn->setFocusPolicy(Qt::TabFocus);
    m_pNextBtn->setObjectName(NEXT_BUTTON);
    m_pNextBtn->setAccessibleName(NEXT_BUTTON);
    connect(m_pNextBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pNextBtn, "next");
    list.append(m_pNextBtn);

    m_pVolBtn->installEventFilter(this);
    m_pVolBtn->setFixedSize(36, 36);
    m_pVolBtn->setFocusPolicy(Qt::TabFocus);
    m_pVolBtn->setObjectName(VOLUME_BUTTON);
    m_pVolBtn->setAccessibleName(VOLUME_BUTTON);

    m_pVolSlider = new VolumeSlider(m_pMainWindow, m_pMainWindow);
    m_pVolSlider->setObjectName(VOLUME_SLIDER_WIDGET);

    connect(m_pPrevBtn, &ButtonBoxButton ::sigUnsupported, this, &ToolboxProxy::sigUnsupported);
    connect(m_pNextBtn, &ButtonBoxButton ::sigUnsupported, this, &ToolboxProxy::sigUnsupported);
    connect(m_pVolBtn, &VolumeButton ::sigUnsupported, this, &ToolboxProxy::sigUnsupported);
    connect(m_pVolBtn, &VolumeButton::clicked, this, &ToolboxProxy::slotVolumeButtonClicked);
    connect(m_pVolBtn, &VolumeButton::leaved, m_pVolSlider, &VolumeSlider::delayedHide);
    connect(m_pVolSlider, &VolumeSlider::sigVolumeChanged, this, &ToolboxProxy::slotVolumeChanged);
    connect(m_pVolSlider, &VolumeSlider::sigMuteStateChanged, this, &ToolboxProxy::slotMuteStateChanged);
    connect(m_pVolBtn, &VolumeButton::requestVolumeUp, m_pVolSlider, &VolumeSlider::volumeUp);
    connect(m_pVolBtn, &VolumeButton::requestVolumeDown, m_pVolSlider, &VolumeSlider::volumeDown);

    _right = new QHBoxLayout(m_pBotToolWgt);
    _right->setContentsMargins(0, 0, 0, 0);
    _right->setSizeConstraint(QLayout::SetFixedSize);
    _right->setSpacing(0);
    bot_layout->addLayout(_right);

    m_pMircastBtn = new ToolButton(m_pBotToolWgt);
    m_pMircastBtn->setIcon(QIcon::fromTheme("dcc_mircast", QIcon(":/resources/icons/mircast/mircast.svg")));
    m_pMircastBtn->setIconSize(QSize(24, 24));
    m_pMircastBtn->installEventFilter(this);
    m_pMircastBtn->setCheckable(true);
    m_pMircastBtn->setFixedSize(50, 50);
    m_pMircastBtn->setFocusPolicy(Qt::TabFocus);
    m_pMircastBtn->initToolTip();
    m_pMircastBtn->setObjectName(MIRVAST_BUTTON);
    m_pMircastBtn->setAccessibleName(MIRVAST_BUTTON);
    connect(m_pMircastBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pMircastBtn, "mircast");
    connect(m_mircastWidget, &MircastWidget::mircastState, this, &ToolboxProxy::slotUpdateMircast);
    _right->addWidget(m_pMircastBtn);
    _right->addSpacing(10);

    m_pListBtn = new ToolButton(m_pBotToolWgt);
    m_pListBtn->setIcon(QIcon::fromTheme("dcc_episodes"));
    m_pListBtn->setIconSize(QSize(36, 36));
    m_pListBtn->setFocusPolicy(Qt::TabFocus);
    m_pListBtn->setFixedSize(TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_WIDTH);
    m_pListBtn->initToolTip();
    m_pListBtn->setCheckable(true);
    m_pListBtn->setObjectName(PLAYLIST_BUTTON);
    m_pListBtn->setAccessibleName(PLAYLIST_BUTTON);
    m_pListBtn->installEventFilter(this);

    connect(m_pListBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pListBtn, "list");
    _right->addWidget(m_pListBtn);
    _right->addSpacing(10);

    m_pFullScreenBtn = new ToolButton(m_pBotToolWgt);
    m_pFullScreenBtn->setObjectName(FS_BUTTON);
    m_pFullScreenBtn->setAccessibleName(FS_BUTTON);
    m_pFullScreenBtn->setFocusPolicy(Qt::TabFocus);
    m_pFullScreenBtn->setIcon(QIcon::fromTheme("dcc_zoomin"));
    m_pFullScreenBtn->setIconSize(QSize(36, 36));
    m_pFullScreenBtn->setFixedSize(TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_WIDTH);
    m_pFullScreenBtn->initToolTip();
    connect(m_pFullScreenBtn, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(m_pFullScreenBtn, "fs");

    m_pVolSlider->initVolume();

    _right->addWidget(m_pFullScreenBtn);

    //将进度条的Tab键次序移动到nextBtn之后
    setTabOrder(m_pNextBtn, m_pProgBar->slider());

    // these tooltips is not used due to deepin ui design
    //lmh0910wayland下用这一套tooltip
    if (utils::check_wayland_env()) {
        initToolTip();
    } else {
        TooltipHandler *th = new TooltipHandler(this);
        QWidget *btns[] = {
            m_pPlayBtn, m_pPrevBtn, m_pNextBtn, m_pFullScreenBtn, m_pMircastBtn, m_pListBtn
        };
        QString hints[] = {
            tr("Play/Pause"), tr("15s backward"), tr("15s forward"),
            tr("Fullscreen"), tr("Miracast"), tr("Playlist")
        };
        QString attrs[] = {
            "play", "prev", "next",
            "fs", "mir", "list"
        };

        for (unsigned int i = 0; i < sizeof(btns) / sizeof(btns[0]); i++) {
            if (i < 3) { //first three buttons prev/play/next
                btns[i]->setToolTip(hints[i]);
                Tip *t = new Tip(QPixmap(), hints[i], parentWidget());
                t->setProperty("for", QVariant::fromValue<QWidget *>(btns[i]));
                btns[i]->setProperty("HintWidget", QVariant::fromValue<QWidget *>(t));
                btns[i]->installEventFilter(th);
            } else {
                ToolButton *btn = dynamic_cast<ToolButton *>(btns[i]);
                btn->setTooTipText(hints[i]);
                btn->setProperty("TipId", attrs[i]);
                connect(btn, &ToolButton::entered, this, &ToolboxProxy::buttonEnter);
                connect(btn, &ToolButton::leaved, this, &ToolboxProxy::buttonLeave);
            }
        }
    }

    connect(m_pEngine, &PlayerEngine::stateChanged, this, &ToolboxProxy::updatePlayState);
    connect(m_pEngine, &PlayerEngine::stateChanged, this, &ToolboxProxy::updateButtonStates);   // 控件状态变化由updateButtonStates统一处理
    connect(m_pEngine, &PlayerEngine::fileLoaded, this, &ToolboxProxy::slotFileLoaded);
    connect(m_pEngine, &PlayerEngine::elapsedChanged, this, &ToolboxProxy::slotElapsedChanged);
    connect(m_pEngine, &PlayerEngine::updateDuration, this, &ToolboxProxy::slotElapsedChanged);

    connect(window()->windowHandle(), &QWindow::windowStateChanged, this, &ToolboxProxy::updateFullState);

    connect(m_pEngine, &PlayerEngine::tracksChanged, this, &ToolboxProxy::updateButtonStates);
    connect(m_pEngine, &PlayerEngine::fileLoaded, this, &ToolboxProxy::updateButtonStates);
    connect(&m_pEngine->playlist(), &PlaylistModel::countChanged, this, &ToolboxProxy::updateButtonStates);
    connect(m_pMainWindow, &MainWindow::initChanged, this, &ToolboxProxy::updateButtonStates);

    updatePlayState();
    updateFullState();
    updateButtonStates();

    connect(qApp, &QGuiApplication::applicationStateChanged, this, &ToolboxProxy::slotApplicationStateChanged);
}

void ToolboxProxy::updateThumbnail()
{
    disconnect(m_pWorker, SIGNAL(sigFinishiLoad(QSize)), this, SLOT(finishLoadSlot(QSize)));

    //如果打开的是音乐
    QString suffix = m_pEngine->playlist().currentInfo().info.suffix();

    if (m_pEngine->currFileIsAudio()) {
        return;
    }

    qInfo() << "worker" << m_pWorker;
    QTimer::singleShot(1000, this, &ToolboxProxy::slotUpdateThumbnailTimeOut);

}

void ToolboxProxy::updatePreviewTime(qint64 secs, const QPoint &pos)
{
    QTime time(0, 0, 0);
    QString strTime = time.addSecs(static_cast<int>(secs)).toString("hh:mm:ss");
    m_pPreviewTime->setTime(strTime);
    m_pPreviewTime->show(pos.x(), pos.y() + 14);
}

void ToolboxProxy::initMember()
{
    m_pmList.clear();
    m_pmBlackList.clear();

    m_pPlaylist = nullptr;

    m_pProgBarspec = nullptr;
    m_pBotSpec = nullptr;
    m_pBotToolWgt = nullptr;
    m_pProgBar_Widget = nullptr;
    bot_widget = nullptr;

    _mid = nullptr;
    _right = nullptr;

    m_pTimeLabel = nullptr;
    m_pTimeLabelend = nullptr;
    m_pViewProgBar = nullptr;
    m_pViewProgBar = nullptr;
    m_pProgBar = nullptr;
    m_pPreviewer = nullptr;
    m_pPreviewTime = nullptr;
    m_mircastWidget = nullptr;

    m_pPlayBtn = nullptr;
    m_pPrevBtn = nullptr;
    m_pNextBtn = nullptr;
    m_pVolBtn = nullptr;
    m_pListBtn = nullptr;
    m_pFullScreenBtn = nullptr;

    m_pPlayBtnTip = nullptr;
    m_pPrevBtnTip = nullptr;
    m_pNextBtnTip = nullptr;
    m_pFullScreenBtnTip = nullptr;
    m_pListBtnTip = nullptr;
    m_pmiracastBtnTip = nullptr;

    m_pWorker = nullptr;
    m_pPaOpen = nullptr;
    m_pPaClose = nullptr;

    m_nClickTime = 0;
    m_processAdd = 0.0;

    m_bMouseFlag = false;
    m_bMousePree = false;
    m_bThumbnailmode = false;
    m_bCanPlay = false;
    m_bSetListBtnFocus = false;
}

/**
 * @brief closeAnyPopup 关闭所有弹窗效果
 */
void ToolboxProxy::closeAnyPopup()
{
    if (m_pPreviewer->isVisible()) {
        m_pPreviewer->hide();
        qInfo() << "hide previewer";
    }

    if (m_pPreviewTime->isVisible()) {
        m_pPreviewTime->hide();
    }

    if (m_pVolSlider->isVisible()) {
        m_pVolSlider->stopTimer();
        m_pVolSlider->popup();
    }
}
/**
 * @brief anyPopupShown 是否存在一些弹出显示窗口
 * @return true时为有，false为无
 */
bool ToolboxProxy::anyPopupShown() const
{
    //返回鼠标悬停缩略图、鼠标悬停时间弹窗、音量弹窗是否有弹出
    return m_pPreviewer->isVisible() || m_pPreviewTime->isVisible() || m_pVolSlider->isVisible();
}

void ToolboxProxy::updateHoverPreview(const QUrl &url, int secs)
{
    if (m_pEngine->state() == PlayerEngine::CoreState::Idle)
        return;

    if (m_pEngine->playlist().currentInfo().url != url)
        return;

    if (!Settings::get().isSet(Settings::PreviewOnMouseover))
        return;

    if (m_pVolSlider->isVisible())
        return;

    const PlayItemInfo &pif = m_pEngine->playlist().currentInfo();
    if (!pif.url.isLocalFile())
        return;

    const QString &absPath = pif.info.canonicalFilePath();
    if (!QFile::exists(absPath)) {
        m_pPreviewer->hide();
        m_pPreviewTime->hide();
        return;
    }

    if (!m_bMouseFlag) {
        return;
    }

    int nPosition = 0;
    qint64 nDuration = m_pEngine->duration();
    QPoint showPoint;

    if(nDuration<=0)
    {
        return;
    }

    if (m_pProgBar->isVisible()) {
        nPosition = (secs * m_pProgBar->slider()->width()) / nDuration;
        showPoint = m_pProgBar->mapToGlobal(QPoint(nPosition, -TOOLBOX_TOP_EXTENT));
    } else {
        nPosition = secs * m_pViewProgBar->getViewLength() / nDuration + m_pViewProgBar->getStartPoint();
        showPoint = m_pViewProgBar->mapToGlobal(QPoint(nPosition, -TOOLBOX_TOP_EXTENT));
    }

    QPixmap pm = ThumbnailWorker::get().getThumb(url, secs);


    if (!pm.isNull()) {
        QPoint point { showPoint.x(), showPoint.y() };
        point.setY(mapToGlobal(QPoint(0, -TOOLBOX_TOP_EXTENT)).y());
        m_pPreviewer->updateWithPreview(pm, secs, m_pEngine->videoRotation());
        m_pPreviewer->updateWithPreview(point);
    }
}

void ToolboxProxy::waitPlay()
{
    if (m_pPlayBtn) {
        m_pPlayBtn->setEnabled(false);
    }
    if (m_pPrevBtn) {
        m_pPrevBtn->setEnabled(false);
    }
    if (m_pNextBtn) {
        m_pNextBtn->setEnabled(false);
    }
    QTimer::singleShot(500, [ = ] {
        if (m_pPlayBtn)
        {
            m_pPlayBtn->setEnabled(true);
        }
        if (m_pPrevBtn)
        {
            m_pPrevBtn->setEnabled(true);
        }
        if (m_pNextBtn)
        {
            m_pNextBtn->setEnabled(true);
        }
    });
}

void ToolboxProxy::slotThemeTypeChanged()
{
    QPalette textPalette;
    bool bRawFormat = false;
    auto type = DGuiApplicationHelper::instance()->themeType();
    WAYLAND_BLACK_WINDOW;
    THEME_TYPE(type);

    // 组合按钮无边框
    QColor framecolor("#FFFFFF");
    framecolor.setAlphaF(0.00);
    QString rStr;
    if (type == DGuiApplicationHelper::LightType) {
        textPalette.setColor(QPalette::WindowText, QColor(0, 0, 0, 40));   // 浅色背景下时长显示置灰
        textPalette.setColor(QPalette::Text, QColor(0, 0, 0, 40));

        QColor maskColor(247, 247, 247);
        maskColor.setAlphaF(0.60);
        rStr = "light";

        DPalette pa;
        pa = m_pFullScreenBtn->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        pa.setColor(DPalette::ButtonText, QColor(Qt::black));
        // 单个按钮边框
        QColor btnframecolor("#000000");
        btnframecolor.setAlphaF(0.00);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);
        DApplicationHelper::instance()->setPalette(m_pFullScreenBtn, pa);
        DApplicationHelper::instance()->setPalette(m_pVolBtn, pa);
        DApplicationHelper::instance()->setPalette(m_pListBtn, pa);
    } else {
        textPalette.setColor(QPalette::WindowText, QColor(255, 255, 255, 40));   // 深色背景下时长显示置灰
        textPalette.setColor(QPalette::Text, QColor(255, 255, 255, 40));

        QColor maskColor(32, 32, 32);
        maskColor.setAlphaF(0.80);
        rStr = "dark";

        DPalette pa;
        pa = m_pFullScreenBtn->palette();
        QColor btnMaskColor("#000000");
        btnMaskColor.setAlphaF(0.30);
        pa.setColor(DPalette::Light, btnMaskColor);
        pa.setColor(DPalette::Dark, btnMaskColor);
        pa.setColor(DPalette::ButtonText, QColor("#c5cfe0"));
        pa.setColor(DPalette::FrameBorder, framecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, framecolor);
        DApplicationHelper::instance()->setPalette(m_pFullScreenBtn, pa);
        DApplicationHelper::instance()->setPalette(m_pVolBtn, pa);
        DApplicationHelper::instance()->setPalette(m_pListBtn, pa);
    }

    if(m_pEngine->state() != PlayerEngine::CoreState::Idle) {
        bRawFormat = m_pEngine->getplaylist()->currentInfo().mi.isRawFormat();
        if(bRawFormat && !m_pEngine->currFileIsAudio()) {
            m_pTimeLabel->setPalette(textPalette);
            m_pTimeLabelend->setPalette(textPalette);

            m_pVolBtn->setButtonEnable(false);
        }
        else if (bRawFormat) {
            m_pTimeLabel->setPalette(textPalette);
            m_pTimeLabelend->setPalette(textPalette);
        }
        else {
            textPalette.setColor(QPalette::WindowText, DApplication::palette().windowText().color());
            textPalette.setColor(QPalette::Text, DApplication::palette().text().color());

            m_pTimeLabel->setPalette(textPalette);
            m_pTimeLabelend->setPalette(textPalette);

            m_pVolBtn->setButtonEnable(true);
        }
    } else {
        textPalette.setColor(QPalette::WindowText, DApplication::palette().windowText().color());
        textPalette.setColor(QPalette::Text, DApplication::palette().text().color());

        m_pTimeLabel->setPalette(textPalette);
        m_pTimeLabelend->setPalette(textPalette);

        m_pVolBtn->setButtonEnable(true);
    }
}

void ToolboxProxy::slotLeavePreview()
{
    auto pos = m_pProgBar->mapFromGlobal(QCursor::pos());
    if (!m_pProgBar->geometry().contains(pos)) {
        m_pPreviewer->hide();
        m_pPreviewTime->hide();
        m_pProgBar->forceLeave();
    }
}

void ToolboxProxy::slotHidePreviewTime()
{
    m_pPreviewer->hide();
    m_pPreviewTime->hide();
    m_bMouseFlag = false;
}

void ToolboxProxy::slotSliderPressed()
{
    m_bMousePree = true;
}

void ToolboxProxy::slotSliderReleased()
{
    m_bMousePree = false;
    if (m_mircastWidget->getMircastState() == MircastWidget::Screening)
        m_mircastWidget->slotSeekMircast(m_pProgBar->slider()->sliderPosition());
    else
        m_pEngine->seekAbsolute(m_pProgBar->slider()->sliderPosition());
}

void ToolboxProxy::slotBaseMuteChanged(QString sk, const QVariant &/*val*/)
{
    if (sk == "base.play.mousepreview") {
        m_pProgBar->setEnableIndication(m_pEngine->state() != PlayerEngine::Idle);
    }
}

void ToolboxProxy::slotVolumeButtonClicked()
{
    if (m_pVolSlider->getsliderstate())
        return;
    //与其他按键保持一致，工具栏隐藏时不响应
    if (!isVisible()) return;
    /*
     * 设置-2为已经完成第一次打开设置音量
     * -1为初始化数值
     * 大于等于零表示为已完成初始化
     */
    if (!m_pVolSlider->isVisible()) {
        m_pVolSlider->show();
        m_pVolSlider->popup();
    } else {
        m_pVolSlider->popup();
    }
}

void ToolboxProxy::slotFileLoaded()
{
    m_pProgBar->slider()->setRange(0, static_cast<int>(m_pEngine->duration()));
    m_pProgBar_Widget->setCurrentIndex(1);
    update();
    //正在投屏时如果当前播放为音频直接播放下一首。
    if(m_pEngine->currFileIsAudio()&&m_mircastWidget->getMircastState() != MircastWidget::Idel) {
        //如果全是音频文件则退出投屏
        bool isAllAudio = true;
        QString sNextVideoName;
        int nNextIndex = -1;
        QList<PlayItemInfo> lstItemInfo = m_pEngine->getplaylist()->items();
        for(int i = 0; i < lstItemInfo.count(); i++) {
            PlayItemInfo iteminfo = lstItemInfo.at(i);
            if(iteminfo.mi.vCodecID != -1) {
                isAllAudio = false;
                if(sNextVideoName.isNull()) {
                    sNextVideoName = iteminfo.mi.filePath;
                    nNextIndex = i;
                    break;
                }
            }
        }
        if(isAllAudio) {
            m_pMainWindow->slotExitMircast();
            return;
        }
        QString sCurPath = m_pEngine->getplaylist()->currentInfo().mi.filePath;
        int nIndex = -1;
        for(int i = 0; i < lstItemInfo.count(); i++) {
            PlayItemInfo iteminfo = lstItemInfo.at(i);
            if(iteminfo.mi.filePath == sCurPath) {
                nIndex = i;
                break;
            }
        }
        if(nIndex == -1) return;
        if(nIndex < nNextIndex && !sNextVideoName.isNull()) {
            m_pMainWindow->play({sNextVideoName});
        } else{
            bool isNext = true;
            for(int i = nIndex; i < lstItemInfo.count(); i++) {
                PlayItemInfo iteminfo = lstItemInfo.at(i);
                if(iteminfo.mi.vCodecID != -1) {
                    isNext = false;
                    m_pMainWindow->play({iteminfo.mi.filePath});
                    break;
                }
            }
            if(m_pEngine->getplaylist()->playMode() == PlaylistModel::OrderPlay) {
                if(isNext)
                    m_pMainWindow->slotExitMircast();
                return;
            }
            if(isNext && !sNextVideoName.isNull()){
                m_pMainWindow->play({sNextVideoName});
            }
        }
        return;
    }
    m_mircastWidget->playNext();
}

void ToolboxProxy::slotElapsedChanged()
{
    if(m_mircastWidget->getMircastState() != MircastWidget::Idel)
        return;
    quint64 url = static_cast<quint64>(-1);
    if (m_pEngine->playlist().current() != -1) {
        url = static_cast<quint64>(m_pEngine->duration());
    }
    //TODO(xxxpengfei):此处代码同时更新全屏的时长并未判断全屏状态，请维护同事查看是否存在优化空间
    updateTimeInfo(static_cast<qint64>(url), m_pEngine->elapsed(), m_pTimeLabel, m_pTimeLabelend, true);
    updateMovieProgress();
}

void ToolboxProxy::slotApplicationStateChanged(Qt::ApplicationState e)
{
    if (e == Qt::ApplicationInactive && anyPopupShown()) {
        closeAnyPopup();
    }
}

void ToolboxProxy::slotPlayListStateChange(bool isShortcut)
{
    closeAnyPopup();
    if (m_pPlaylist->state() == PlaylistWidget::State::Opened) {
        //非x86平台播放列表切换不展示动画,故按键状态不做限制
        if(CompositingManager::get().platform() == Platform::X86) {
            if (!m_pListBtn->isChecked())
                m_pListBtn->setChecked(true);
        } else {
            Q_UNUSED(isShortcut);
            m_pListBtn->setChecked(true);
        }
    } else {
        if(CompositingManager::get().platform() == Platform::X86) {
            if (m_pListBtn->isChecked())
                m_pListBtn->setChecked(false);
        } else {
            Q_UNUSED(isShortcut);
            m_pListBtn->setChecked(false);
        }
    }
}

void ToolboxProxy::slotUpdateThumbnailTimeOut()
{
    //如果视频长度小于1s应该直接返回不然会UI错误
    if (m_pEngine->duration() < 1) {
        return;
    }

    m_pViewProgBar->clear();  //清除前一次进度条中的缩略图,以便显示新的缩略图
    m_listPixmapMutex.lock();
    m_pmList.clear();
    m_pmBlackList.clear();
    m_listPixmapMutex.unlock();

    if (m_pWorker == nullptr) {
        m_pWorker = new viewProgBarLoad(m_pEngine, m_pProgBar, this);
        m_pWorker->setListPixmapMutex(&m_listPixmapMutex);
    }
    m_pWorker->requestInterruption();
    QTimer::singleShot(500, this, [ = ] {m_pWorker->start();});
    connect(m_pWorker, SIGNAL(sigFinishiLoad(QSize)), this, SLOT(finishLoadSlot(QSize)));
    m_pProgBar_Widget->setCurrentIndex(1);
}

void ToolboxProxy::slotVolumeChanged(int nVolume)
{
    m_pVolBtn->setVolume(nVolume);

    emit sigVolumeChanged(nVolume);
}

void ToolboxProxy::slotMuteStateChanged(bool bMute)
{
    m_pVolBtn->setMute(bMute);

    emit sigMuteStateChanged(bMute);
}

qint64 ToolboxProxy::getMouseTime()
{
    return m_nClickTime;
}

void ToolboxProxy::clearPlayListFocus()
{
    if (m_pPlaylist->isFocusInPlaylist()) {
        m_pPlaylist->clearFocus();
    }
    m_bSetListBtnFocus = false;
}

void ToolboxProxy::setBtnFocusSign(bool sign)
{
    m_bSetListBtnFocus = sign;
}

bool ToolboxProxy::isInMircastWidget(const QPoint &p)
{
    if (!m_mircastWidget->isVisible())
        return false;
    return m_mircastWidget->geometry().contains(p);
}

void ToolboxProxy::updateMircastWidget(QPoint p)
{
    m_mircastWidget->move(p.x() - m_mircastWidget->width(), p.y() - m_mircastWidget->height() - 10);
}

void ToolboxProxy::hideMircastWidget()
{
    m_mircastWidget->hide();
    m_pMircastBtn->setChecked(false);
    updateplaylisticon();
}
/**
 * @brief volumeUp 鼠标滚轮增加音量
 */
void ToolboxProxy::volumeUp()
{
    if(!m_pVolSlider->isEnabled()) {    // 不能调节音量需要给出提示
        emit sigUnsupported();
    } else {
        m_pVolSlider->volumeUp();
    }
}
/**
 * @brief volumeUp 鼠标滚轮减少音量
 */
void ToolboxProxy::volumeDown()
{
    if(!m_pVolSlider->isEnabled()) {
        emit sigUnsupported();
    } else {
        m_pVolSlider->volumeDown();
    }
}
/**
 * @brief calculationStep 计算鼠标滚轮滚动的步进
 * @param iAngleDelta 鼠标滚动的距离
 */
void ToolboxProxy::calculationStep(int iAngleDelta)
{
    m_pVolSlider->calculationStep(iAngleDelta);
}
/**
 * @brief changeMuteState 切换静音模式
 */
void ToolboxProxy::changeMuteState()
{
    m_pVolSlider->muteButtnClicked();
}
/**
 * @brief playlistClosedByEsc Esc关闭播放列表
 */
void ToolboxProxy::playlistClosedByEsc()
{
    if (m_pPlaylist->isFocusInPlaylist() && m_bSetListBtnFocus) {
//        m_bSetListBtnFocus = true;
        m_pMainWindow->requestAction(ActionFactory::TogglePlaylist);
//        m_pListBtn->setFocus();   //焦点回到播放列表按钮
    }
}

void ToolboxProxy::progressHoverChanged(int nValue)
{
    if (m_pEngine->state() == PlayerEngine::CoreState::Idle)
        return;

    if (m_pVolSlider->isVisible())
        return;

    const auto &pif = m_pEngine->playlist().currentInfo();
    if (!pif.url.isLocalFile())
        return;
    const auto &absPath = pif.info.canonicalFilePath();
    if (!QFile::exists(absPath)) {
        m_pPreviewer->hide();
        m_pPreviewTime->hide();
        return;
    }

    m_bMouseFlag = true;

    QPoint pos = m_pProgBar->mapToGlobal(QPoint(0, TOOLBOX_TOP_EXTENT - 10));
    QPoint point { QCursor::pos().x(), pos.y() };
    QPoint startPoint = mapToGlobal(QPoint(m_pProgBar_Widget->x(), 0));
    QPoint endPoint = mapToGlobal(QPoint(m_pProgBar_Widget->x() + m_pProgBar->width(), 0));

    /*********************************
    * 时长显示不能超出进度条
    * ********************************/
    if (point.x() < startPoint.x()) {
        point.setX(startPoint.x());
    }

    if (point.x() > endPoint.x()) {
        point.setX(endPoint.x());
    }

    bool bIsAudio = m_pEngine->currFileIsAudio();
    if (!Settings::get().isSet(Settings::PreviewOnMouseover) || bIsAudio) {
        updatePreviewTime(nValue, point);
        return;
    }
    //鼠标移动时同步缩略图显示位置
    int nPosition = 0;
    qint64 nDuration = m_pEngine->duration();

    if(nDuration<=0)
    {
        return;
    }

    if (m_pProgBar->isVisible()) {
        nPosition = (nValue * m_pProgBar->slider()->width()) / nDuration;
        point = m_pProgBar->mapToGlobal(QPoint(nPosition, -TOOLBOX_TOP_EXTENT));
    } else {
        nPosition = nValue * m_pViewProgBar->getViewLength() / nDuration + m_pViewProgBar->getStartPoint();
        point = m_pViewProgBar->mapToGlobal(QPoint(nPosition, -TOOLBOX_TOP_EXTENT));
    }
    point.setY(mapToGlobal(QPoint(0, -TOOLBOX_TOP_EXTENT)).y());
    m_pPreviewer->updateWithPreview(point);
    if(CompositingManager::isMpvExists()) {
        ThumbnailWorker::get().requestThumb(pif.url, nValue);
    }
}

void ToolboxProxy::updateTimeVisible(bool visible)
{
    if (Settings::get().isSet(Settings::PreviewOnMouseover))
        return;

    if (m_pPreviewTime) {
        m_pPreviewTime->setVisible(!visible);
    }
}

void ToolboxProxy::updateMovieProgress()
{
    if (m_bMousePree == true)
        return ;
    auto d = m_pEngine->duration();
    auto e = m_pEngine->elapsed();
    if (d > m_pProgBar->maximum()) {
        d = m_pProgBar->maximum();
    }
    int v = 0;
    int v2 = 0;
    if (d != 0 && e != 0) {
        v = static_cast<int>(m_pProgBar->maximum() * e / d);
        v2 = static_cast<int>(m_pViewProgBar->getViewLength() * e / d + m_pViewProgBar->getStartPoint());
    }
    if (!m_pProgBar->signalsBlocked()) {
        m_pProgBar->blockSignals(true);
        m_pProgBar->setValue(v);
        m_pProgBar->blockSignals(false);
    }
    if (!m_pViewProgBar->getIsBlockSignals()) {
        m_pViewProgBar->setIsBlockSignals(true);
        m_pViewProgBar->setValue(v2);
        m_pViewProgBar->setTime(e);
        m_pViewProgBar->setIsBlockSignals(false);
    }
}

void ToolboxProxy::updateButtonStates()
{
    QPalette palette;              // 时长显示的颜色，在某些情况下变化字体颜色区别功能
    bool bRawFormat = false;

    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        palette.setColor(QPalette::WindowText, QColor(0, 0, 0, 40));       // 浅色背景下置灰
        palette.setColor(QPalette::Text, QColor(0, 0, 0, 40));
    } else {
        palette.setColor(QPalette::WindowText, QColor(255, 255, 255, 40)); // 深色背景下置灰
        palette.setColor(QPalette::Text, QColor(255, 255, 255, 40));
    }

    if(m_pEngine->state() != PlayerEngine::CoreState::Idle) {
        bRawFormat = m_pEngine->getplaylist()->currentInfo().mi.isRawFormat();
        m_pMircastBtn->setEnabled(!m_pEngine->currFileIsAudio());
        if(m_pEngine->currFileIsAudio())
            m_mircastWidget->setVisible(false);
        if(bRawFormat && !m_pEngine->currFileIsAudio()) {                                             // 如果正在播放的视频是裸流不支持音量调节和进度调节
            m_pProgBar->setEnabled(false);
            m_pProgBar->setEnableIndication(false);
            m_pVolSlider->setEnabled(false);

            m_pTimeLabel->setPalette(palette);             // 如果正在播放的视频是裸流置灰
            m_pTimeLabelend->setPalette(palette);

            m_pVolBtn->setButtonEnable(false);

            m_pPrevBtn->setDisabled(true);
            m_pNextBtn->setDisabled(true);
        } else if (bRawFormat) {
            m_pProgBar->setEnabled(false);
            m_pProgBar->setEnableIndication(false);

            m_pTimeLabel->setPalette(palette);
            m_pTimeLabelend->setPalette(palette);

            m_pPrevBtn->setDisabled(true);
            m_pNextBtn->setDisabled(true);
        } else {
            m_pProgBar->setEnabled(true);
            m_pProgBar->setEnableIndication(true);
            m_pVolSlider->setEnabled(true);

            palette.setColor(QPalette::WindowText, DApplication::palette().windowText().color());
            palette.setColor(QPalette::Text, DApplication::palette().text().color());

            m_pTimeLabel->setPalette(palette);
            m_pTimeLabelend->setPalette(palette);

            m_pVolBtn->setButtonEnable(true);

            m_pPrevBtn->setEnabled(true);
            m_pNextBtn->setEnabled(true);
        }
    } else {
        m_pVolSlider->setEnabled(true);

        palette.setColor(QPalette::WindowText, DApplication::palette().windowText().color());
        palette.setColor(QPalette::Text, DApplication::palette().text().color());

        m_pTimeLabel->setPalette(palette);
        m_pTimeLabelend->setPalette(palette);

         m_pVolBtn->setButtonEnable(true);

         m_pPrevBtn->setDisabled(true);
         m_pNextBtn->setDisabled(true);
    }

    qInfo() << m_pEngine->playingMovieInfo().subs.size();
    bool vis = m_pEngine->playlist().count() > 1 && m_pMainWindow->inited();
    m_bCanPlay = vis;  //防止连续切换上下曲目
}

void ToolboxProxy::updateFullState()
{
    bool isFullscreen = window()->isFullScreen();
    if (isFullscreen) {
        m_pFullScreenBtn->setIcon(QIcon::fromTheme("dcc_zoomout"));
        if (utils::check_wayland_env())
            m_pFullScreenBtnTip->setText(tr("Exit fullscreen"));
        else
            m_pFullScreenBtn->setTooTipText(tr("Exit fullscreen"));
    } else {
        m_pFullScreenBtn->setIcon(QIcon::fromTheme("dcc_zoomin"));
        if (utils::check_wayland_env())
            m_pFullScreenBtnTip->setText(tr("Fullscreen"));
        else
            m_pFullScreenBtn->setTooTipText(tr("Fullscreen"));
    }
}

void ToolboxProxy::slotUpdateMircast(int state, QString msg)
{
    emit sigMircastState(state, msg);
    if (state == 0) {
        m_pVolBtn->setButtonEnable(false);
        m_pFullScreenBtn->setEnabled(false);
    } else {
        if(m_pEngine->getplaylist()->items().size() == 0) {
            m_pVolBtn->setButtonEnable(true);
        } else {
            bool bRawFormat = m_pEngine->getplaylist()->currentInfo().mi.isRawFormat();
            if(bRawFormat && !m_pEngine->currFileIsAudio()) {
                m_pVolBtn->setButtonEnable(false);
            } else {
                m_pVolBtn->setButtonEnable(true);
            }
        }
    }
    m_pFullScreenBtn->setEnabled(true);
}

void ToolboxProxy::updatePlayState()
{
    if (((m_mircastWidget->getMircastState() != MircastWidget::Idel) && (m_mircastWidget->getMircastPlayState() == MircastWidget::Play))
            || m_pEngine->state() == PlayerEngine::CoreState::Playing) {
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            DPalette pa;

            pa = m_pVolBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pVolBtn->setPalette(pa);

            pa = m_pFullScreenBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pFullScreenBtn->setPalette(pa);

            pa = m_pListBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pListBtn->setPalette(pa);

            pa = m_pMircastBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pMircastBtn->setPalette(pa);

        } else {
            DPalette pa;

            pa = m_pVolBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pVolBtn->setPalette(pa);

            pa = m_pFullScreenBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pFullScreenBtn->setPalette(pa);

            pa = m_pListBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pListBtn->setPalette(pa);

            pa = m_pMircastBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pMircastBtn->setPalette(pa);
        }
        m_pPlayBtn->setIcon(QIcon::fromTheme("dcc_suspend", QIcon(":/icons/deepin/builtin/light/normal/suspend_normal.svg")));
        //lmh0910wayland下用这一套tooltip
        if (utils::check_wayland_env()) {
            m_pPlayBtnTip->setText(tr("Pause"));
        } else {
            m_pPlayBtn->setToolTip(tr("Pause"));
        }
    } else {
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            DPalette pa;


            pa = m_pVolBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pVolBtn->setPalette(pa);

            pa = m_pFullScreenBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pFullScreenBtn->setPalette(pa);

            pa = m_pListBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pListBtn->setPalette(pa);

            pa = m_pMircastBtn->palette();
            pa.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            m_pMircastBtn->setPalette(pa);

        } else {
            DPalette pa;

            pa = m_pVolBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pVolBtn->setPalette(pa);

            pa = m_pFullScreenBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pFullScreenBtn->setPalette(pa);

            pa = m_pListBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pListBtn->setPalette(pa);

            pa = m_pMircastBtn->palette();
            pa.setColor(DPalette::Light, QColor(0, 0, 0, 255));
            pa.setColor(DPalette::Dark, QColor(0, 0, 0, 255));
            m_pMircastBtn->setPalette(pa);

        }
        //lmh0910wayland下用这一套tooltip
        if (utils::check_wayland_env()) {
            m_pPlayBtnTip->setText(tr("Play"));
        } else {
            m_pPlayBtn->setToolTip(tr("Play"));
        }
        m_pPlayBtn->setIcon(QIcon::fromTheme("dcc_play", QIcon(":/icons/deepin/builtin/light/normal/play_normal.svg")));
    }

    if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
        m_pTimeLabel->setText("");
        m_pTimeLabelend->setText("");

        if (m_pPreviewer->isVisible()) {
            m_pPreviewer->hide();
        }

        if (m_pPreviewTime->isVisible()) {
            m_pPreviewTime->hide();
        }

        if (m_pProgBar->isVisible()) {
            m_pProgBar->setVisible(false);
        }
        m_pProgBar_Widget->setCurrentIndex(0);
        setProperty("idle", true);
    } else {
        setProperty("idle", false);
    }
}
/**
 * @brief updateTimeInfo 更新工具栏中播放时间显示
 * @param duration 视频总时长
 * @param pos 当前播放的时间点
 * @param pTimeLabel 当前播放时间
 * @param pTimeLabelend 视频总时长
 * @param flag 是否为全屏的控件
 */
void ToolboxProxy::updateTimeInfo(qint64 duration, qint64 pos, QLabel *pTimeLabel, QLabel *pTimeLabelend, bool flag)
{
    if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
        pTimeLabel->setText("");
        pTimeLabelend->setText("");

    } else {
        //mpv returns a slightly different duration from movieinfo.duration
        //m_pTimeLabel->setText(QString("%2/%1").arg(utils::Time2str(duration))
        //.arg(utils::Time2str(pos)));
        if (1 == flag) {
            pTimeLabel->setText(QString("%1")
                                .arg(utils::Time2str(pos)));
            pTimeLabelend->setText(QString("%1")
                                   .arg(utils::Time2str(duration)));
        } else {
            pTimeLabel->setText(QString("%1 %2")
                                .arg(utils::Time2str(pos)).arg("/"));
            pTimeLabelend->setText(QString("%1")
                                   .arg(utils::Time2str(duration)));
        }


    }
}

void ToolboxProxy::buttonClicked(QString id)
{
    //add by heyi
    static bool bFlags = true;
    if (bFlags) {
//        m_pMainWindow->firstPlayInit();
        m_pMainWindow->repaint();
        bFlags = false;
    }

    if (!isVisible()) return;

    qInfo() << __func__ << id;
    if (id == "play") {
        if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
            m_pMainWindow->requestAction(ActionFactory::ActionKind::StartPlay);
        } else {
            m_pMainWindow->requestAction(ActionFactory::ActionKind::TogglePause);
        }
    } else if (id == "fs") {
        m_pMainWindow->requestAction(ActionFactory::ActionKind::ToggleFullscreen);
    } else if (id == "vol") {
        m_pMainWindow->requestAction(ActionFactory::ActionKind::ToggleMute);
    } else if (id == "prev") {  //如果影片未加载完成，则不播放上一曲
//        m_pMainWindow->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
        m_pEngine->seekBackward(15);
    } else if (id == "next") {
//        m_pMainWindow->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
        m_pEngine->seekForward(15);
    } else if (id == "list") {
        m_nClickTime = QDateTime::currentMSecsSinceEpoch();
        m_pMainWindow->requestAction(ActionFactory::ActionKind::TogglePlaylist);
        m_pListBtn->hideToolTip();
    } else if (id == "mircast") {
        m_mircastWidget->togglePopup();
        m_pMircastBtn->hideToolTip();
        m_pMircastBtn->setChecked(m_mircastWidget->isVisible());
        if (m_pMircastBtn->isChecked())
            m_pMircastBtn->setIcon(QIcon(":/icons/deepin/builtin/light/checked/mircast_chenked.svg"));
        else
            m_pMircastBtn->setIcon(QIcon::fromTheme("dcc_mircast"));
    }
}

void ToolboxProxy::buttonEnter()
{
    if (!isVisible()) return;

    ToolButton *btn = qobject_cast<ToolButton *>(sender());
    QString id = btn->property("TipId").toString();

    if (id == "sub" || id == "fs" || id == "list" || id == "mir") {
        updateToolTipTheme(btn);
        btn->showToolTip();
    }
}

void ToolboxProxy::buttonLeave()
{
    if (!isVisible()) return;

    ToolButton *btn = qobject_cast<ToolButton *>(sender());
    QString id = btn->property("TipId").toString();

    if (id == "sub" || id == "fs" || id == "list" || id == "mir") {
        btn->hideToolTip();
    }
}

void ToolboxProxy::showEvent(QShowEvent *event)
{
    DFloatingWidget::showEvent(event);
}

void ToolboxProxy::paintEvent(QPaintEvent *event)
{
    if(CompositingManager::get().platform() != X86) {
        QPainter painter(this);
            setFixedWidth(m_pMainWindow->width());
            move(0, m_pMainWindow->height() - this->height());
            if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
                painter.fillRect(rect(), QBrush(QColor(31, 31, 31)));
            } else {
                painter.fillRect(rect(), this->palette().background());
            }
    } else {
        DFloatingWidget::paintEvent(event);
    }
}

void ToolboxProxy::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize().width() != event->size().width()) {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            if (m_bThumbnailmode) {  //如果进度条为胶片模式，重新加载缩略图并显示
                if(CompositingManager::get().platform() == Platform::X86 && CompositingManager::isMpvExists()) {
                    updateThumbnail();
                }
                updateMovieProgress();
            }
            m_pProgBar_Widget->setCurrentIndex(1);
        }
    }

    DFloatingWidget::resizeEvent(event);
}

void ToolboxProxy::mouseMoveEvent(QMouseEvent *ev)
{
    setButtonTooltipHide();
    QWidget::mouseMoveEvent(ev);
}

bool ToolboxProxy::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_pVolBtn) {
        if (ev->type() == QEvent::KeyPress && (m_pVolSlider->state() == VolumeSlider::Open)) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
            int nCurVolume = m_pVolSlider->getVolume();
            //如果音量条升起且上下键按下，以步长为5调整音量
            if (keyEvent->key() == Qt::Key_Up) {
                m_pVolSlider->changeVolume(qMin(nCurVolume + 5, 200));
                return true;
            } else if (keyEvent->key() == Qt::Key_Down) {
                m_pVolSlider->changeVolume(qMax(nCurVolume - 5, 0));
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, ev);
}

void ToolboxProxy::updateMircastTime(int time)
{
    if (m_pProgBar_Widget->currentIndex() == 1) {              //进度条模式

        if (!m_pProgBar->signalsBlocked()) {
            m_pProgBar->blockSignals(true);
        }

        m_pProgBar->slider()->setSliderPosition(time);
        m_pProgBar->slider()->setValue(time);
        m_pProgBar->blockSignals(false);
    } else {
        m_pViewProgBar->setIsBlockSignals(true);
        m_pViewProgBar->setValue(time);
        m_pViewProgBar->setIsBlockSignals(false);
    }
    quint64 url = static_cast<quint64>(-1);
    if (m_pEngine->playlist().current() != -1) {
        url = static_cast<quint64>(m_pEngine->duration());
    }
    updateTimeInfo(url, time, m_pTimeLabel, m_pTimeLabelend, true);
}

void ToolboxProxy::updateToolTipTheme(ToolButton *btn)
{
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        btn->changeTheme(lightTheme);
    } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        btn->changeTheme(darkTheme);
    } else {
        btn->changeTheme(lightTheme);
    }
}

void ToolboxProxy::setPlaylist(PlaylistWidget *pPlaylist)
{
    m_pPlaylist = pPlaylist;
    WAYLAND_BLACK_WINDOW;
    connect(m_pPlaylist, &PlaylistWidget::stateChange, this, &ToolboxProxy::slotPlayListStateChange);

}

void ToolboxProxy::setVolSliderHide()
{
    if (m_pVolSlider->isVisible()) {
        m_pVolSlider->popup();
    }
}

void ToolboxProxy::setButtonTooltipHide()
{
    if (utils::check_wayland_env()) {
        m_pPlayBtnTip->hide();
        m_pPrevBtnTip->hide();
        m_pNextBtnTip->hide();
        m_pFullScreenBtnTip->hide();
        m_pListBtnTip->hide();
        m_pmiracastBtnTip->hide();
    } else {
        m_pListBtn->hideToolTip();
        m_pFullScreenBtn->hideToolTip();
    }
}

void ToolboxProxy::initToolTip()
{
    if (utils::check_wayland_env()) {
        //lmh0910播放
        m_pPlayBtnTip = new ButtonToolTip(m_pMainWindow);
        m_pPlayBtnTip->setText(tr("Play"));
        connect(m_pPlayBtn, &ButtonBoxButton::entered, [ = ]() {
            m_pPlayBtnTip->move(80, m_pMainWindow->height() - TOOLBOX_HEIGHT - 5);
            m_pPlayBtnTip->show();
            m_pPlayBtnTip->update();
            m_pPlayBtnTip->releaseMouse();

        });
        connect(m_pPlayBtn, &ButtonBoxButton::leaved, [ = ]() {
            QTimer::singleShot(0, [ = ] {
                m_pPlayBtnTip->hide();
            });
        });
        //lmh0910上一个
        m_pPrevBtnTip = new ButtonToolTip(m_pMainWindow);
        m_pPrevBtnTip->setText(tr("15s backward"));
        connect(m_pPrevBtn, &ButtonBoxButton::entered, [ = ]() {
            m_pPrevBtnTip->move(40,
                                m_pMainWindow->height() - TOOLBOX_HEIGHT - 5);
            m_pPrevBtnTip->show();
            m_pPrevBtnTip->update();
            m_pPrevBtnTip->releaseMouse();

        });
        connect(m_pPrevBtn, &ButtonBoxButton::leaved, [ = ]() {
            QTimer::singleShot(0, [ = ] {
                m_pPrevBtnTip->hide();
            });
        });

        //lmh0910下一个
        m_pNextBtnTip = new ButtonToolTip(m_pMainWindow);
        m_pNextBtnTip->setText(tr("15s forward"));
        connect(static_cast<ButtonBoxButton *>(m_pNextBtn), &ButtonBoxButton::entered, [ = ]() {
            m_pNextBtnTip->move(120,
                                m_pMainWindow->height() - TOOLBOX_HEIGHT - 5);
            m_pNextBtnTip->show();
            m_pNextBtnTip->update();
            m_pNextBtnTip->releaseMouse();

        });
        connect(static_cast<ButtonBoxButton *>(m_pNextBtn), &ButtonBoxButton::leaved, [ = ]() {
            QTimer::singleShot(0, [ = ] {
                m_pNextBtnTip->hide();
            });
        });
    }
    //lmh0910全屏按键
    m_pFullScreenBtnTip = new ButtonToolTip(m_pMainWindow);
    m_pFullScreenBtnTip->setText(tr("Fullscreen"));
    connect(m_pFullScreenBtn, &ToolButton::entered, [ = ]() {
        m_pFullScreenBtnTip->move(m_pFullScreenBtn->pos().x() + 30,
                                  m_pMainWindow->height() - TOOLBOX_HEIGHT - 10);
        m_pFullScreenBtnTip->show();
        m_pFullScreenBtnTip->update();
        m_pFullScreenBtnTip->releaseMouse();

    });
    connect(m_pFullScreenBtn, &ToolButton::leaved, [ = ]() {
        QTimer::singleShot(0, [ = ] {
            m_pFullScreenBtnTip->hide();
        });
    });
    //lmh0910list按键
    m_pListBtnTip = new ButtonToolTip(m_pMainWindow);
    m_pListBtnTip->setText(tr("Playlist"));
    connect(m_pListBtn, &ToolButton::entered, [ = ]() {
        m_pListBtnTip->move(m_pListBtn->pos().x() + 30,
                            m_pMainWindow->height() - TOOLBOX_HEIGHT - 10);
        m_pListBtnTip->show();
        m_pListBtnTip->update();
        m_pListBtnTip->releaseMouse();

    });
    connect(m_pListBtn, &ToolButton::leaved, [ = ]() {
        QTimer::singleShot(0, [ = ] {
            m_pListBtnTip->hide();
        });
    });
    //lmh0910mir按键
    m_pmiracastBtnTip = new ButtonToolTip(m_pMainWindow);
    m_pmiracastBtnTip->setText(tr("Miracast"));
    connect(m_pMircastBtn, &ToolButton::entered, [ = ]() {
        m_pmiracastBtnTip->move(m_pMircastBtn->pos().x() + 35,
                            m_pMainWindow->height() - TOOLBOX_HEIGHT - 10);
        m_pmiracastBtnTip->show();
        m_pmiracastBtnTip->update();
        m_pmiracastBtnTip->releaseMouse();

    });
    connect(m_pMircastBtn, &ToolButton::leaved, [ = ]() {
        QTimer::singleShot(0, [ = ] {
            m_pmiracastBtnTip->hide();
        });
    });
}

bool ToolboxProxy::getListBtnFocus()
{
    return m_pListBtn->hasFocus();
}

bool ToolboxProxy::getVolSliderIsHided()
{
    return m_pVolSlider->isHidden();
}
/**
 * @brief updateProgress 更新播放进度条显示
 * @param nValue 进度条的值
 */
void ToolboxProxy::updateProgress(int nValue)
{
    int nDuration = static_cast<int>(m_pEngine->duration());

    if (m_pProgBar_Widget->currentIndex() == 1) {              //进度条模式

        float value = nValue * nDuration / m_pProgBar->width();
        int nCurrPos;
        if (value > 1 || value < -1) {
            nCurrPos = m_pProgBar->value() + value;
        } else {
            if (m_processAdd < 1.0 && m_processAdd > -1.0) {
                m_processAdd += (float)(nValue * nDuration) / m_pProgBar->width();
                qInfo() << m_processAdd;
                return;
            }
            else {
                nCurrPos = m_pProgBar->value() + m_processAdd;
                m_processAdd = .0;
            }
        }
        if (!m_pProgBar->signalsBlocked()) {
            m_pProgBar->blockSignals(true);
        }

        m_pProgBar->slider()->setSliderPosition(nCurrPos);
        m_pProgBar->slider()->setValue(nCurrPos);
    } else {
        m_pViewProgBar->setIsBlockSignals(true);
        m_pViewProgBar->setValue(m_pViewProgBar->getValue() + nValue);
    }
}
/**
 * @brief updateSlider 根据进度条显示更新影片实际进度
 */
void ToolboxProxy::updateSlider()
{
    if (m_pProgBar_Widget->currentIndex() == 1) {
        m_pEngine->seekAbsolute(m_pProgBar->value());

        m_pProgBar->blockSignals(false);
    } else {
        m_pEngine->seekAbsolute(m_pViewProgBar->getTimePos());
        m_pViewProgBar->setIsBlockSignals(false);
    }
}
/**
 * @brief initThumb 初始化加载胶片线程
 */
void ToolboxProxy::initThumbThread()
{
    ThumbnailWorker::get().setPlayerEngine(m_pEngine);
    connect(&ThumbnailWorker::get(), &ThumbnailWorker::thumbGenerated,
            this, &ToolboxProxy::updateHoverPreview);
}
/**
 * @brief updateSliderPoint 非x86平台下更新音量条控件位置
 * @param point 传入主窗口左上角顶点在屏幕的位置
 */
void ToolboxProxy::updateSliderPoint(QPoint &point)
{
    m_pVolSlider->updatePoint(point);
}

/**
 * @brief ~ToolboxProxy 析构函数
 */
ToolboxProxy::~ToolboxProxy()
{
    if(CompositingManager::isMpvExists()) {
        ThumbnailWorker::get().stop();
    }
    delete m_pPreviewer;
    delete m_pPreviewTime;

    if (m_pWorker) {
        m_pWorker->quit();
        m_pWorker->deleteLater();
    }
}

viewProgBarLoad::viewProgBarLoad(PlayerEngine *engine, DMRSlider *progBar, ToolboxProxy *parent)
{
    initMember();
    m_pParent = parent;
    m_pEngine = engine;
    m_pProgBar = progBar;
    m_seekTime = new char[12];
    initThumb();
}

void viewProgBarLoad::setListPixmapMutex(QMutex *pMutex)
{
    m_pListPixmapMutex = pMutex;
}

void viewProgBarLoad::run()
{
    loadViewProgBar(m_pParent->size());
}

viewProgBarLoad::~viewProgBarLoad()
{
    delete [] m_seekTime;
    m_seekTime = nullptr;

    if (m_video_thumbnailer != nullptr) {
        m_mvideo_thumbnailer_destroy(m_video_thumbnailer);
        m_video_thumbnailer = nullptr;
    }
}

}
#undef THEME_TYPE
#include "toolbox_proxy.moc"
