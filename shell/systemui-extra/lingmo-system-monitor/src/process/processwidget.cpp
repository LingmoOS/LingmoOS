#include "processwidget.h"

#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDebug>

#include "../style/usmproxystyle.h"
#include "../macro.h"
#include "../util.h"

ProcessWidget::ProcessWidget(QSettings *proSettings, QWidget *parent)
    : QWidget(parent)
    , m_proSettings(proSettings)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initConnections();
    onSwitchProcType(m_nWhichNum);
}

ProcessWidget::~ProcessWidget()
{
    if (m_ifSettings) {
        delete m_ifSettings;
        m_ifSettings = nullptr;
    }
}

void ProcessWidget::onWndClose()
{
    if (m_procTabView) {
        m_procTabView->onWndClose();
    }
}

void ProcessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ProcessWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12,0,0,0);
    m_mainLayout->setSpacing(0);
    m_topBtnsLayout = new QHBoxLayout();
    m_topBtnsLayout->setContentsMargins(0,0,0,16);
    m_topBtnsLayout->setSpacing(0);
    m_topBtnsLayout->setAlignment(Qt::AlignHCenter);
    m_procTableLayout = new QVBoxLayout();
    m_procTableLayout->setContentsMargins(0,0,0,0);
    m_procTableLayout->setSpacing(0);

    // 初始化控件
    processTabBar = new kdk::KTabBar(kdk::SegmentDark);
    processTabBar->setFixedWidth(NORMALWIDTH*3);
    processTabBar->addTab(tr("Applications"));
    processTabBar->addTab(tr("My Processes"));
    processTabBar->addTab(tr("All Process"));
    m_topBtnsLayout->addWidget(processTabBar);
    
    //初始化进程列表
    m_procTabView = new ProcessTableView(m_proSettings);
    m_procTabView->installEventFilter(this);
    m_procTableLayout->addWidget(m_procTabView);

    m_mainLayout->addLayout(m_topBtnsLayout);
    m_mainLayout->addLayout(m_procTableLayout);
    this->setLayout(m_mainLayout);
    initThemeMode();
}

void ProcessWidget::initConnections()
{
    connect(this,SIGNAL(changeProcessItemDialog(int)), m_procTabView, SLOT(onChangeProcessFilter(int)));
    connect(processTabBar, SIGNAL(currentChanged(int)), this, SLOT(onSwitchProcType(int)));
    connect(this, SIGNAL(searchSignal(QString)), m_procTabView, SLOT(onSearch(QString)), Qt::QueuedConnection);
}

void ProcessWidget::initThemeMode()
{
    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid))
    {
        m_ifSettings = new QGSettings(ifid);
    }
    if (m_ifSettings && m_ifSettings->keys().contains(WHICH_MENU_KEY)) {
        auto whichNum = m_ifSettings->get(WHICH_MENU);
        if (whichNum.isValid()) {
            m_nWhichNum =  whichNum.toInt();
        }
    }
}


void ProcessWidget::onSwitchProcType(int nType)
{
#if 1
    if (nType == PROCESSTYPE_ACTIVE) {
        nType = PROCESSTYPE_APPLICATION;
    }
    if (nType > PROCESSTYPE_APPLICATION || nType < PROCESSTYPE_ACTIVE) {
        nType = PROCESSTYPE_APPLICATION;
    }
#endif
    processTabBar->blockSignals(true);
    if (nType == PROCESSTYPE_APPLICATION) {
        processTabBar->setCurrentIndex(0);
    } else {
        processTabBar->setCurrentIndex(nType);
    }
    processTabBar->blockSignals(false);
    Q_EMIT changeProcessItemDialog(nType);
    if (m_ifSettings) {
        m_ifSettings->set(WHICH_MENU, nType);
    }
}
