// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "volumeslider.h"
#include "toolbox_proxy.h"
#include "dbusutils.h"

DWIDGET_USE_NAMESPACE

namespace dmr {

VolumeSlider::VolumeSlider(MainWindow *mw, QWidget *parent)
    : QWidget(parent), _mw(mw)
{
    if (CompositingManager::get().platform() != Platform::X86)
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    m_iStep = 0;
    m_bIsWheel = false;
    m_nVolume = 100;
    m_bHideWhenFinished = false;

    hide();
    setFocusPolicy(Qt::TabFocus);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(2, 16, 2, 14);  //内边距，与UI沟通确定
    vLayout->setSpacing(0);
    setLayout(vLayout);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setWeight(QFont::Medium);

    vLayout->addStretch();
    m_pLabShowVolume = new DLabel(_mw);
    DFontSizeManager::instance()->bind(m_pLabShowVolume, DFontSizeManager::T6);
    m_pLabShowVolume->setForegroundRole(QPalette::BrightText);
    m_pLabShowVolume->setFont(font);
    m_pLabShowVolume->setAlignment(Qt::AlignCenter);
    m_pLabShowVolume->setText("0%");
    vLayout->addWidget(m_pLabShowVolume, 0, Qt::AlignCenter);

    m_slider = new DSlider(Qt::Vertical, this);
    m_slider->setFixedWidth(24);
    m_slider->setIconSize(QSize(15, 15));
    m_slider->installEventFilter(this);
    m_slider->show();
    m_slider->slider()->setRange(0, 100);
    m_slider->setValue(m_nVolume);
    m_slider->setObjectName(VOLUME_SLIDER);
    m_slider->slider()->setObjectName(SLIDER);
    m_slider->slider()->setAccessibleName(SLIDER);
    m_slider->slider()->setMinimumHeight(120);
    m_slider->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    m_slider->slider()->setFocusPolicy(Qt::NoFocus);
    vLayout->addWidget(m_slider, 1, Qt::AlignCenter);

    m_pBtnChangeMute = new DToolButton(this);
    m_pBtnChangeMute->setObjectName(MUTE_BTN);
    m_pBtnChangeMute->setAccessibleName(MUTE_BTN);
    m_pBtnChangeMute->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //同时设置按钮与图标的尺寸，改变其默认比例
    m_pBtnChangeMute->setFixedSize(30, 30);
    m_pBtnChangeMute->setIconSize(QSize(30, 30));
    m_pBtnChangeMute->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    m_pBtnChangeMute->setFocusPolicy(Qt::NoFocus);
    vLayout->addStretch(10);
    vLayout->addWidget(m_pBtnChangeMute, 0, Qt::AlignHCenter);
    vLayout->addStretch();

    connect(m_slider, &DSlider::valueChanged, this, &VolumeSlider::volumeChanged);
    connect(m_pBtnChangeMute, SIGNAL(clicked()), this, SLOT(muteButtnClicked()));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &VolumeSlider::setThemeType);
    m_autoHideTimer.setSingleShot(true);
    connect(&m_autoHideTimer, &QTimer::timeout, this, &VolumeSlider::popup);
}

void VolumeSlider::initVolume()
{
    QTimer::singleShot(500, this, [ = ] { //延迟加载等待信号槽连接
        int nVolume = Settings::get().internalOption("global_volume").toInt();
        bool bMute = Settings::get().internalOption("mute").toBool();

        changeVolume(nVolume);
        changeMuteState(bMute);

        setMute(bMute);
    });
}

void VolumeSlider::stopTimer()
{
    m_autoHideTimer.stop();
}

QString VolumeSlider::readSinkInputPath()
{
    QString strPath = "";

    QVariant v = ApplicationAdaptor::redDBusProperty("org.deepin.daemon.Audio1", "/org/deepin/daemon/Audio1",
                                                     "org.deepin.daemon.Audio1", "SinkInputs");

    if (!v.isValid())
        return strPath;

    QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();

    for (auto curPath : allSinkInputsList) {
        QVariant nameV = ApplicationAdaptor::redDBusProperty("org.deepin.daemon.Audio1", curPath.path(),
                                                             "org.deepin.daemon.Audio1.SinkInput", "Name");
        QString strMovie = QObject::tr("Movie");
        if (!nameV.isValid() || (!nameV.toString().contains(strMovie, Qt::CaseInsensitive) && !nameV.toString().contains("deepin movie", Qt::CaseInsensitive)))
            continue;

        strPath = curPath.path();
        break;
    }
    return strPath;
}

void VolumeSlider::setMute(bool muted)
{
    if (m_bIsMute != muted || m_nVolume == 0) {
        return;
    }

    QString sinkInputPath = readSinkInputPath();

    if (!sinkInputPath.isEmpty()) {
        QDBusInterface ainterface("org.deepin.daemon.Audio1", sinkInputPath,
                                  "org.deepin.daemon.Audio1.SinkInput",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return;
        }

        //调用设置音量
        ainterface.call(QLatin1String("SetMute"), muted);
    }

    return;
}

void VolumeSlider::updatePoint(QPoint point)
{
    QRect main_rect = _mw->rect();
    QRect view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));
    m_point = point + QPoint(TOOLBOX_BUTTON_WIDTH * 3 + (VOLSLIDER_WIDTH - TOOLBOX_BUTTON_WIDTH) / 2 + 20,
                             view_rect.height() - TOOLBOX_HEIGHT - VOLSLIDER_HEIGHT);
}
void VolumeSlider::popup()
{
    QRect main_rect = _mw->rect();
    QRect view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));

    int x = TOOLBOX_BUTTON_WIDTH * 3 + (TOOLBOX_BUTTON_WIDTH - VOLSLIDER_WIDTH) / 2 + 20;
    int y = view_rect.height() - TOOLBOX_HEIGHT - VOLSLIDER_HEIGHT - TOOLBOX_TOP_EXTENT;
    QRect end(x, y, VOLSLIDER_WIDTH, VOLSLIDER_HEIGHT);
    QRect start = end;

    start.setWidth(start.width() + 12);
    start.setHeight(start.height() + 10);
    if(CompositingManager::get().platform() == Platform::X86) {
        start.moveTo(start.topLeft() - QPoint(6, 20));
    } else {
        end.moveTo(m_point + QPoint(6, 0));
        start.moveTo(m_point - QPoint(0, 14));
    }

    //动画未完成，等待动画结束后再隐藏控件
    if (pVolAnimation) {
        m_bHideWhenFinished = true;
        return;
    }

    if (m_state == State::Close && isVisible()) {
        pVolAnimation = new QPropertyAnimation(this, "geometry");
        pVolAnimation->setEasingCurve(QEasingCurve::Linear);
        pVolAnimation->setKeyValueAt(0, end);
        pVolAnimation->setKeyValueAt(0.3, start);
        pVolAnimation->setKeyValueAt(1, end);
        pVolAnimation->setDuration(230);
        m_bFinished = true;
        raise();
        pVolAnimation->start();
        connect(pVolAnimation, &QPropertyAnimation::finished, [ = ] {
            pVolAnimation->deleteLater();
            pVolAnimation = nullptr;
            m_state = Open;
            m_bFinished = false;
            if (m_bHideWhenFinished) {
                popup();
                m_bHideWhenFinished = false;
            }
        });
    } else {
        m_state = Close;
        hide();
    }
}
void VolumeSlider::delayedHide()
{
    m_mouseIn = false;
    DUtil::TimerSingleShot(100, [this]() {
        if (!m_mouseIn)
            popup();
    });
}
void VolumeSlider::changeVolume(int nVolume)
{
    if (nVolume <= 0) {
        nVolume = 0;
    } else if (nVolume > 200) {
        nVolume = 200;
    }

    m_nVolume = nVolume;

    m_slider->setValue(nVolume > 100 ? 100 : nVolume);  //音量实际能改变200,但是音量条最大支持到100

    //1.保证初始化音量(100)和配置文件中音量一致时，也能得到刷新
    //2.m_slider的最大值为100,如果大于100必须主动调用
    if (nVolume >= 100) {
        volumeChanged(nVolume);
    }
}

void VolumeSlider::calculationStep(int iAngleDelta){
    m_bIsWheel = true;

    if ((m_iStep > 0 && iAngleDelta > 0) || (m_iStep < 0 && iAngleDelta < 0)) {
        m_iStep += iAngleDelta;
    } else {
        m_iStep = iAngleDelta;
    }
}

void VolumeSlider::volumeUp()
{
    if (m_bIsWheel) {
        m_bIsWheel = false;
        if(qAbs(m_iStep) >= 120) {
            m_nVolume += qAbs(m_iStep) / 120 * 10;
            changeVolume(qMin(m_nVolume, 200));
            m_iStep = 0;
        }
    } else {
        changeVolume(qMin(m_nVolume + 10, 200));
    }
}

void VolumeSlider::volumeDown()
{
    if(m_bIsWheel){
        m_bIsWheel = false;
        if(qAbs(m_iStep) >= 120){
            m_nVolume -= qAbs(m_iStep) / 120 * 10 ;
            changeVolume(qMax(m_nVolume, 0));
            m_iStep = 0;
        }
    }else{
        changeVolume(qMax(m_nVolume - 10, 0));
    }

}

void VolumeSlider::changeMuteState(bool bMute)
{
    if (m_bIsMute == bMute || m_nVolume == 0) {
        return;
    }

    m_bIsMute = bMute;
    refreshIcon();
    Settings::get().setInternalOption("mute", m_bIsMute);
    emit sigMuteStateChanged(bMute);
}

void VolumeSlider::volumeChanged(int nVolume)
{
    if (m_nVolume != nVolume) {
        m_nVolume = nVolume;
    }

    if (m_nVolume > 0 && m_bIsMute) {      //音量改变时改变静音状态
        changeMuteState(false);
        setMute(false);
    }

    refreshIcon();

    emit sigVolumeChanged(nVolume);
}

void VolumeSlider::refreshIcon()
{
    int nValue = m_slider->value();

    if (nValue >= 66)
        m_pBtnChangeMute->setIcon(QIcon::fromTheme("dcc_volume"));
    else if (nValue >= 33)
        m_pBtnChangeMute->setIcon(QIcon::fromTheme("dcc_volumemid"));
    else
        m_pBtnChangeMute->setIcon(QIcon::fromTheme("dcc_volumelow"));

    if (m_bIsMute || nValue == 0)
        m_pBtnChangeMute->setIcon(QIcon::fromTheme("dcc_mute"));

    m_pLabShowVolume->setText(QString("%1%").arg(nValue * 1.0 / m_slider->maximum() * 100));
}

void VolumeSlider::muteButtnClicked()
{
    bool bMute = m_bIsMute;

    changeMuteState(!bMute);
    setMute(!bMute);
}

bool VolumeSlider::getsliderstate()
{
    return m_bFinished;
}

int VolumeSlider::getVolume()
{
    return m_nVolume;
}

void VolumeSlider::setThemeType(int type)
{
    Q_UNUSED(type)
}

void VolumeSlider::enterEvent(QEvent *e)
{
    m_mouseIn = true;
    QWidget::leaveEvent(e);
}
void VolumeSlider::showEvent(QShowEvent *se)
{
    QWidget::showEvent(se);
}
void VolumeSlider::leaveEvent(QEvent *e)
{
    m_mouseIn = false;
    delayedHide();
    QWidget::leaveEvent(e);
}
void VolumeSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor bgColor = this->palette().background().color();
    if(CompositingManager::get().platform() != Platform::X86) {
        QRect rect = this->rect();
                rect.setTopLeft(QPoint(1, 1));
                rect.setSize(QSize(VOLSLIDER_WIDTH - 2, VOLSLIDER_HEIGHT - 2));
                painter.fillRect(rect, bgColor);
    } else {
        double dRation = this->height() * 1.0 / VOLSLIDER_HEIGHT;
        const qreal radius = 20 * dRation;
        const qreal triHeight = 30 * dRation;
        const qreal height = this->height() - triHeight;
        const qreal width = this->width();

        painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

        // 背景上矩形
        //这里要一次绘制不然中间会出现虚线
        QPainterPath pathRect;
        pathRect.moveTo(radius, 0);
        pathRect.lineTo(width - radius, 0);
        pathRect.arcTo(QRectF(QPointF(width - 2 * radius, 0), QPointF(width, 2 * radius)), 90.0, -90.0);
        pathRect.lineTo(width, height);
        pathRect.lineTo(0, height);
        pathRect.lineTo(0, radius);
        pathRect.arcTo(QRectF(QPointF(0, 0), QPointF(2 * radius, 2 * radius)), 180.0, -90.0);

        // 背景下三角，半边
        qreal radius1 = radius / 2;
        QPainterPath pathTriangle;
        pathTriangle.moveTo(0, height - radius1);
        pathTriangle.arcTo(QRectF(QPointF(0, height - radius1), QSizeF(2 * radius1, 2 * radius1)), 180, 60);
        pathTriangle.lineTo(width / 2, this->height());
        qreal radius2 = radius / 4;
        pathTriangle.arcTo(QRectF(QPointF(width / 2 - radius2, this->height() - radius2 * 2 - 2), QSizeF(2 * radius2, 2 * radius2)), 220, 130);
        pathTriangle.lineTo(width / 2, height);

        // 正向绘制
        painter.fillPath(pathRect, bgColor);
        painter.fillPath(pathTriangle, bgColor);

        // 平移坐标系
        painter.translate(width, 0);
        // 坐标系X反转
        painter.scale(-1, 1);

        // 反向绘制
        painter.fillPath(pathTriangle, bgColor);
    }
}

void VolumeSlider::keyPressEvent(QKeyEvent *pEvent)
{
    QWidget::keyPressEvent(pEvent);
}

bool VolumeSlider::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent *>(e);
        qInfo() << we->angleDelta() << we->modifiers() << we->buttons();
        if (we->buttons() == Qt::NoButton && we->modifiers() == Qt::NoModifier) {
            calculationStep(we->angleDelta().y());
            if (we->angleDelta().y() > 0 ) {
                volumeUp();
            } else {
                volumeDown();
            }
        }
        return false;
    } else {
        return QObject::eventFilter(obj, e);
    }
}

VolumeSlider::~VolumeSlider()
{
}

}
