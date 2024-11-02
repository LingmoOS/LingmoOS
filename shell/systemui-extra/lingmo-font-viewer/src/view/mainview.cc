#include <QScreen>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QStackedWidget>
#include <QPainterPath>

/* 适配组内SDK */
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "windowmanage.hpp"


#include "mainview.h"

MainView::MainView(QWidget *parent) : QWidget(parent)
{
    kabase::WindowManage::getWindowId(&m_windowId);

    setWidgetUi();
    initLayout();
    setSlotConnet();
}

MainView::~MainView()
{
}

MainView *MainView::getInstance()
{
    static MainView instance;
    return &instance;
}

void MainView::setWidgetUi()
{
    initGsetting();

    m_functionWid = new FunctionWid(this);
    m_fontWid = new FontWidget(this);

    this->installEventFilter(this);
    m_functionWid->installEventFilter(this);
    m_fontWid->installEventFilter(this);
    return;
}

void MainView::initLayout()
{
    //毛玻璃
    this->setProperty("useSystemStyleBlur",true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    this->setMinimumSize(880 , 570);
    this->setWindowTitle(tr("Font Viewer"));

    m_hLayout = new QHBoxLayout();
    m_hLayout->addWidget(m_functionWid);
    m_hLayout->addSpacing(10);
    m_hLayout->addWidget(m_fontWid);
    m_hLayout->setSpacing(0);
    m_hLayout->setMargin(0);

    this->setLayout(m_hLayout);

    // 应用居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());

    this->changeStyle();
    return;
}

void MainView::setSlotConnet()
{
    connect(m_functionWid, &FunctionWid::sigChangeFontType, m_fontWid, &FontWidget::slotChangeFilterType);
    connect(this, &MainView::sigWindowState, m_fontWid, &FontWidget::slotWindowState);
    return;
}

void MainView::pullUpWindow()
{
    this->show();
    ::kabase::WindowManage::setMiddleOfScreen(this);
    return;
}

void MainView::initGsetting()
{
    GlobalSizeData::getInstance();
    /* 主题 */
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalSizeData::getInstance()->getSystemTheme();
        // m_functionWid->changeTheme();
        this->changeStyle();
        this->update();
    });

    /* 透明度 */ 
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this, [=]() {
        GlobalSizeData::getInstance()->getSystemTransparency();
        this->update();
    });

}

void MainView::changeStyle()
{
    // 设置背景色
    m_fontWid->setAutoFillBackground(true);

    QPalette pal;
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::LINGMOLight) {
        pal.setColor(QPalette::Background, QColor(QString("#FFFFFF")));
        m_fontWid->setPalette(pal);
    } else {
        pal.setColor(QPalette::Background, QColor(QString("#000000")));
        m_fontWid->setPalette(pal);
    }
    return;
}

void MainView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    /* 反锯齿 */
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath rectPath;
    rectPath.addRoundedRect(rect(), 0, 0);

    QStyleOption opt;
    opt.init(this);

    p.setPen(Qt::NoPen);
    QColor color = qApp->palette().color(QPalette::Window);
    color.setAlpha(GlobalSizeData::getInstance()->g_transparency);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window,QColor(color));
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.fillPath(rectPath, brush);

    return;
}

bool MainView::eventFilter(QObject *watch, QEvent *event)
{
    if (m_fontWid->eventFilter(watch, event)) {
        return QObject::eventFilter(watch, event);
    }
    return QObject::eventFilter(watch, event);
}

void MainView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        // 帮助点击事件处理
        kdk::UserManual userManual;
        if (!userManual.callUserManual("lingmo-font-viewer")) {
            qCritical() << "user manual call fail!";
        }
    }
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
       m_fontWid->m_titleBar->m_searchArea->setFocus();
    }

    return QWidget::keyPressEvent(event);
}

void MainView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (this->windowState() == Qt::WindowMaximized) {
            QString state = "maxWindow";
            emit sigWindowState(state);
        } else {
            QString state = "normalWindow";
            emit sigWindowState(state);
        }
    }
    return;
}

void MainView::getFontFileFromClick(QString fontFile)
{
    m_fontWid->useFontFileOpen(fontFile);
    
    return;
}

void MainView::getFontFileFromClickAferOpen(QString fontFile)
{
    m_fontWid->cilckFontFile(fontFile);
    
    return;
}
