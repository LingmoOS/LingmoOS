// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform_volumeslider.h"
#include "platform_toolbox_proxy.h"
#include "dbusutils.h"

DWIDGET_USE_NAMESPACE

namespace dmr {

Platform_VolumeSlider::Platform_VolumeSlider(Platform_MainWindow *mw, QWidget *parent)
    : QWidget(parent), _mw(mw)
{
    if (CompositingManager::get().platform() != Platform::X86) {
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    } else {
        setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    }
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

    connect(m_slider, &DSlider::valueChanged, this, &Platform_VolumeSlider::volumeChanged);
    connect(m_pBtnChangeMute, SIGNAL(clicked()), this, SLOT(muteButtnClicked()));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &Platform_VolumeSlider::setThemeType);
    m_autoHideTimer.setSingleShot(true);
    connect(&m_autoHideTimer, &QTimer::timeout, this, &Platform_VolumeSlider::popup);
}

void Platform_VolumeSlider::initVolume()
{
    QTimer::singleShot(500, this, [ = ] { //延迟加载等待信号槽连接
        int nVolume = Settings::get().internalOption("global_volume").toInt();
        bool bMute = Settings::get().internalOption("mute").toBool();

        changeVolume(nVolume);
        changeMuteState(bMute);

        setMute(bMute);
    });
}

void Platform_VolumeSlider::stopTimer()
{
    m_autoHideTimer.stop();
}

QString Platform_VolumeSlider::readSinkInputPath()
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

void Platform_VolumeSlider::setMute(bool muted)
{
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

void Platform_VolumeSlider::updatePoint(QPoint point)
{
    QRect main_rect = _mw->rect();
    QRect view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));
    m_point = point + QPoint((TOOLBOX_BUTTON_WIDTH + TOOLBOX_LAYOUT_SPACING) * 3 + (TOOLBOX_BUTTON_WIDTH - VOLSLIDER_WIDTH) / 2,
                             view_rect.height() - TOOLBOX_HEIGHT - VOLSLIDER_HEIGHT);
}
void Platform_VolumeSlider::popup()
{
    QRect main_rect = _mw->rect();
    QRect view_rect = main_rect.marginsRemoved(QMargins(1, 1, 1, 1));

    int x = (TOOLBOX_BUTTON_WIDTH + TOOLBOX_LAYOUT_SPACING) * 3 + (TOOLBOX_BUTTON_WIDTH - VOLSLIDER_WIDTH) / 2;
    int y = view_rect.height() - TOOLBOX_HEIGHT - VOLSLIDER_HEIGHT;
    y += 10;
    QRect end(x, y, VOLSLIDER_WIDTH, VOLSLIDER_HEIGHT);
    QRect start = end;

    start.setWidth(start.width() + 12);
    start.setHeight(start.height() + 10);
    end.moveTo(m_point - QPoint(10, 0));
    start.moveTo(m_point - QPoint(10, 14));

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
void Platform_VolumeSlider::delayedHide()
{
    const int nGap = 18;   // 音量条和音量按钮之间的间距
    QRect adRect = QRect(m_point + QPoint(6, VOLSLIDER_HEIGHT), m_point + QPoint(6 + VOLSLIDER_WIDTH, VOLSLIDER_HEIGHT + nGap));

    m_mouseIn = false;
    if(adRect.contains(QCursor::pos())){
        DUtil::TimerSingleShot(2000, [=]() {
            if (!m_mouseIn)
                popup();
        });
    } else {
        DUtil::TimerSingleShot(100, [=]() {
            if (!m_mouseIn)
                popup();
        });
    }
}
void Platform_VolumeSlider::changeVolume(int nVolume)
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

void Platform_VolumeSlider::calculationStep(int iAngleDelta){
    m_bIsWheel = true;

    if ((m_iStep > 0 && iAngleDelta > 0) || (m_iStep < 0 && iAngleDelta < 0)) {
        m_iStep += iAngleDelta;
    } else {
        m_iStep = iAngleDelta;
    }
}

void Platform_VolumeSlider::volumeUp()
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

void Platform_VolumeSlider::volumeDown()
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

void Platform_VolumeSlider::changeMuteState(bool bMute)
{
    if (m_bIsMute == bMute || m_nVolume == 0) {
        return;
    }

    m_bIsMute = bMute;
    refreshIcon();
    Settings::get().setInternalOption("mute", m_bIsMute);
    emit sigMuteStateChanged(bMute);
}

void Platform_VolumeSlider::volumeChanged(int nVolume)
{
    if (m_nVolume != nVolume) {
        m_nVolume = nVolume;
    }

    if (m_nVolume > 0 && m_bIsMute) {      //音量改变时改变静音状态
        changeMuteState(false);
    }

    refreshIcon();
    Settings::get().setInternalOption("global_volume", m_nVolume > 100 ? 100 : m_nVolume);

    emit sigVolumeChanged(nVolume);
}

void Platform_VolumeSlider::refreshIcon()
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

void Platform_VolumeSlider::muteButtnClicked()
{
    bool bMute = m_bIsMute;

    changeMuteState(!bMute);
    setMute(!bMute);
}

bool Platform_VolumeSlider::getsliderstate()
{
    return m_bFinished;
}

int Platform_VolumeSlider::getVolume()
{
    return m_nVolume;
}

void Platform_VolumeSlider::setThemeType(int type)
{
    Q_UNUSED(type)
}

void Platform_VolumeSlider::enterEvent(QEvent *e)
{
    m_mouseIn = true;
    QWidget::leaveEvent(e);
}
void Platform_VolumeSlider::showEvent(QShowEvent *se)
{
    QWidget::showEvent(se);
}
void Platform_VolumeSlider::leaveEvent(QEvent *e)
{
    m_mouseIn = false;
    delayedHide();
    QWidget::leaveEvent(e);
}
void Platform_VolumeSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor bgColor = this->palette().background().color();
    painter.fillRect(rect(), bgColor);
}

void Platform_VolumeSlider::keyPressEvent(QKeyEvent *pEvent)
{
    int nCurVolume = getVolume();
    if (pEvent->key() == Qt::Key_Up) {
        changeVolume(qMin(nCurVolume + 5, 200));
    } else if (pEvent->key() == Qt::Key_Down) {
        changeVolume(qMax(nCurVolume - 5, 0));
    }
    QWidget::keyPressEvent(pEvent);
}

bool Platform_VolumeSlider::eventFilter(QObject *obj, QEvent *e)
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

Platform_VolumeSlider::~Platform_VolumeSlider()
{
}

}
