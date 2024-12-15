// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickpanelwidget.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DToolTip>

#include <QVBoxLayout>

DGUI_USE_NAMESPACE

QuickPanelWidget::QuickPanelWidget(QWidget *parent)
    : QWidget(parent)
    , m_icon(new CommonIconButton(this))
    , m_description(new DLabel(this))
{
    initUI();
    m_timer = new QTimer(this);
    m_showTimeStr = tr("Record");
    m_type = RECORD;
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &QuickPanelWidget::refreshIcon);
}

QuickPanelWidget::~QuickPanelWidget()
{
    if (nullptr != m_timer)
        m_timer->deleteLater();
}

void QuickPanelWidget::initUI()
{
    m_icon->setFixedSize(QSize(24, 24));

    m_description->setElideMode(Qt::ElideRight);
    DToolTip::setToolTipShowMode(m_description, DToolTip::ShowWhenElided);
    DFontSizeManager::instance()->bind(m_description, DFontSizeManager::T10);

    auto layout = new QVBoxLayout;
    layout->setMargin(10);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_icon, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(m_description, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    setLayout(layout);
}

/**
   @brief 设置当前显示的图标为 \a icon ，默认状态和录制模式显示不同图标
 */
void QuickPanelWidget::setIcon(const QIcon &icon)
{
    m_icon->setIcon(icon);
}

/**
   @brief 设置显示的文案为 \a description ，录制时将显示"录制中"文案
 */
void QuickPanelWidget::setDescription(const QString &description)
{
    m_description->setText(description);
    // 同样更新提示文案
    m_description->setToolTip(description);
}

/**
   @brief 切换截图或录屏模式为 \a type
   @sa FuctionType
 */
void QuickPanelWidget::changeType(FuctionType type)
{
    setDescription(m_showTimeStr);
    m_type = type;
    if (type == RECORD) {
        QString shotIcon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ?
                               "status-screen-record-dark" :
                               "status-screen-record";
        setIcon(QIcon::fromTheme(shotIcon, QIcon(QString(":/res/%1.svg").arg(shotIcon))));
    } else if (type == RECORDING) {
        QString recordIcon("screen-recording");
        setIcon(QIcon::fromTheme(recordIcon, QIcon(QString(":/res/%1.svg").arg(recordIcon))));
    } else {
        qWarning() << "Type Unkonw! Please select RECORD or RECORDING";
    }
}

/**
   @brief start:开始计时
 */
void QuickPanelWidget::start()
{
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

/**
   @brief stop:停止计时
 */
void QuickPanelWidget::stop()
{
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    m_showTimeStr = tr("Record");
}

/**
   @brief pause:暂停
 */
void QuickPanelWidget::pause()
{
    m_timer->stop();
    setDescription(m_showTimeStr);
}

/**
   @brief 超时时更新显示的时间范围
 */
void QuickPanelWidget::onTimeout()
{
    QTime showTime(0, 0, 0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_showTimeStr = showTime.toString("hh:mm:ss");
    qInfo() << "Current show time: " << m_showTimeStr;
    setDescription(m_showTimeStr);
    update();
}

/**
   @brief 刷新图标，重设当前的显示图标
 */
void QuickPanelWidget::refreshIcon()
{
    changeType(m_type);
}

/**
   @brief 鼠标点击抛出点击信号
 */
void QuickPanelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (underMouse()) {
        Q_EMIT clicked();
    }
}
