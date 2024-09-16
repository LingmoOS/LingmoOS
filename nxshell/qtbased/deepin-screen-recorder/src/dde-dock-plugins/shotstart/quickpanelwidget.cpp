#include "quickpanelwidget.h"
#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DToolTip>
DGUI_USE_NAMESPACE
QuickPanelWidget::QuickPanelWidget(QWidget *parent)
    :QWidget(parent)
    , m_icon(new CommonIconButton(this))
    , m_description(new DLabel(this))
{
    initUI();
    m_timer = new QTimer(this);
    m_showTimeStr = tr("Screenshot");
    m_type = SHOT;
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::themeTypeChanged,this,&QuickPanelWidget::refreshIcon);
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

void QuickPanelWidget::setIcon(const QIcon &icon)
{
    m_icon->setIcon(icon);
}

void QuickPanelWidget::setDescription(const QString &description)
{
    m_description->setText(description);
    //m_description->setToolTip(description);
}

void QuickPanelWidget::setWidgetState(WidgetState state)
{
    if (m_icon)
        m_icon->setActiveState(WS_ACTIVE == state);
}

void QuickPanelWidget::changeType(int type)
{
    setDescription(m_showTimeStr);
    m_type = type;
    if(type == SHOT){
        QString shotIcon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType? "screenshot-dark" : "screenshot";
        setIcon(QIcon::fromTheme(shotIcon, QIcon(QString(":/res/%1.svg").arg(shotIcon))));
    }else if(type == RECORD){
        QString recordIcon("screen-recording");
        setIcon(QIcon::fromTheme(recordIcon, QIcon(QString(":/res/%1.svg").arg(recordIcon))));
    }else {
        qWarning() << "Type Unkonw! Please select SHOT or RECORD";
    }
}
void QuickPanelWidget::start()
{
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

void QuickPanelWidget::stop()
{
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    if(m_timer->isActive()){
        m_timer->stop();
    }
    m_showTimeStr = tr("Screenshot");
}

void QuickPanelWidget::pause()
{
    m_timer->stop();
    setDescription(m_showTimeStr);
}

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

void QuickPanelWidget::refreshIcon()
{
    changeType(m_type);
}
void QuickPanelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (underMouse()) {
        Q_EMIT clicked();
    }
}
