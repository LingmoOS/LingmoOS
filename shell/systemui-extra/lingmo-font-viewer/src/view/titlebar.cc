#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

#include <usermanual.h>

#include "titlebar.h"
#include "globalsizedata.h"
#include <QDebug>
const QSize BTN_SIZE = QSize(30, 30);
const QSize BTN_SIZE_MAX = QSize(48, 48);
const QSize SEARCH_SIZE =  QSize(240, 37);
const QSize SEARCH_SIZE_MAX = QSize(300, 40);

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    this->init();
    this->initMenu();
    this->initLayout();
    this->conn();
}

TitleBar::~TitleBar() {}

void TitleBar::init()
{
    // this->setMinimumHeight(40);

    m_addFontBtn = new QPushButton(this);
    // m_searchArea = new BaseSearchEdit();
    m_searchArea = new KSearchLineEdit(this);
    m_menuBtn = new QToolButton(this);
    m_menu = new QMenu();
    m_minBtn = new QPushButton(this);
    m_maxBtn = new QPushButton(this);
    m_closeBtn = new QPushButton(this);

    // 添加字体按钮
    m_addFontBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_addFontBtn->setIconSize(QSize(16, 16));
    m_addFontBtn->setProperty("isWindowButton", 0x1);
    m_addFontBtn->setProperty("useIconHighlightEffect", 0x2);
    m_addFontBtn->setContentsMargins(4, 0, 4, 0);
    m_addFontBtn->setToolTip(tr("Add Font"));

    // 最小化按钮
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_minBtn->setIconSize(QSize(16, 16));
    m_minBtn->setProperty("isWindowButton", 0x1);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setFlat(true);
    m_minBtn->setContentsMargins(4, 0, 4, 0);
    m_minBtn->setToolTip(tr("Minimize"));

    // 最大化按钮
    m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_maxBtn->setIconSize(QSize(16, 16));
    m_maxBtn->setProperty("isWindowButton", 0x1);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setFlat(true);
    m_maxBtn->setContentsMargins(4, 0, 4, 0);
    m_maxBtn->setToolTip(tr("Maximize"));

    // 关闭按钮
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setIconSize(QSize(16, 16));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    m_closeBtn->setContentsMargins(4, 0, 4, 0);
    m_closeBtn->setToolTip(tr("Close"));

    m_searchArea->setClearButtonEnabled(false);
    this->setFocusPolicy(Qt::ClickFocus);

    this->installEventFilter(this);
    m_addFontBtn->installEventFilter(this);
    m_minBtn->installEventFilter(this);
    m_maxBtn->installEventFilter(this);
    m_closeBtn->installEventFilter(this);
    m_menuBtn->installEventFilter(this);
    m_searchArea->installEventFilter(this);

    initWidgetStyle();
    return;
}

void TitleBar::initWidgetStyle()
{
    switch (GlobalSizeData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCSize();        
            break;
        case CurrentMode::HMode:
        slotChangeFaltSize();
            break;
        case CurrentMode::VMode:
        slotChangeFaltSize();
            break;
    }
    return;
}

void TitleBar::initMenu()
{
    // 菜单按钮
    m_menuBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuBtn->setIconSize(QSize(16, 16));
    m_menuBtn->setProperty("isWindowButton", 0x1);
    m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuBtn->setPopupMode(QToolButton::InstantPopup);
    m_menuBtn->setAutoRaise(true);
    m_menuBtn->setContentsMargins(4, 0, 4, 0);
    m_menuBtn->setToolTip(tr("Options"));

    // 创建Action
    QAction *actionHelp = new QAction(m_menu);
    QAction *actionAbout = new QAction(m_menu);
    QAction *actionQuit = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    actionAbout->setText(tr("About"));
    actionQuit->setText(tr("Quit"));

    // 将Action添加到菜单中
    m_actions << actionHelp << actionAbout<< actionQuit;
    m_menu->addActions(m_actions);

    m_menuBtn->setMenu(m_menu);
    return;
}

void TitleBar::initLayout()
{
    // 整体布局
    m_hlayout = new QHBoxLayout();
    m_hlayout->setSpacing(0);
    m_hlayout->addWidget(m_addFontBtn);
    m_hlayout->addStretch();
    m_hlayout->addWidget(m_searchArea);
    m_hlayout->addStretch();
    m_hlayout->addWidget(m_menuBtn);
    m_hlayout->addWidget(m_minBtn);
    m_hlayout->addWidget(m_maxBtn);
    m_hlayout->addWidget(m_closeBtn);
    m_hlayout->setContentsMargins(10, 4, 5, 4);

    this->setLayout(m_hlayout);
    return;
}

void TitleBar::initAbout()
{
    QProcess p;
    QString version;
    QStringList args;
    args << "-c" << "dpkg -l lingmo-font-viewer | grep lingmo-font-viewer";
    p.start("bash", args);
    p.waitForFinished();
    p.waitForReadyRead();
    version = p.readAll();
   QStringList fields = version.split(QRegularExpression("[ \t]+"));
   if (fields.size() >= 3)
       version = fields.at(2);
   else
       version = "none";

    kdk::KAboutDialog aboutWindow(this,QIcon::fromTheme("lingmo-font-viewer"),tr(appShowingName.toLocal8Bit()),tr("Version: ") + version);
    
    aboutWindow.setBodyText(tr("Font Viewer is a tool to help users install and organize management; "
                      "After installation, the font can be applied to self-developed applications, "
                      "third-party pre installed applications and user self installed applications."));
    aboutWindow.setBodyTextVisiable(false);

    aboutWindow.exec();
    return;
}

void TitleBar::initHelp()
{
    // 帮助点击事件处理
    kdk::UserManual userManualTest;
    if (!userManualTest.callUserManual("lingmo-font-viewer")) {
        qCritical() << "user manual call fail!";
    }
    return;
}

void TitleBar::conn()
{
    connect(m_addFontBtn, &QPushButton::clicked, this, &TitleBar::slotAddFont);
    connect(m_minBtn, &QPushButton::clicked, this, &TitleBar::slotMinWid);
    connect(m_maxBtn, &QPushButton::clicked, this, &TitleBar::slotMaxWid);
    connect(m_closeBtn, &QPushButton::clicked, this, &TitleBar::slotCloseWid);

    connect(m_menu, &QMenu::triggered, this, &TitleBar::slotTriggerMenu);
    // connect(m_searchArea, &BaseSearchEdit::sigSearchFont, this, &TitleBar::slotTitleSearch);
    connect(m_searchArea, &KSearchLineEdit::textChanged, this, &TitleBar::slotTitleSearch);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontNameChange, this, &TitleBar::slotChangeFontName);

    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigPCMode, this, &TitleBar::slotChangePCSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigHFlatMode, this, &TitleBar::slotChangeFaltSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigVFlatMode, this, &TitleBar::slotChangeFaltSize);
    return;
}

bool TitleBar::eventFilter(QObject *watch, QEvent *e)
{
    // if (m_searchArea->eventFilter(watch, e)) {
    //     return QObject::eventFilter(watch, e);
    // }
    return false;
}

void TitleBar::SearchFocusOut()
{
    // m_searchArea->searchFocusOut();
    m_searchArea->clearFocus();
    return;
}

void TitleBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        initHelp();
    }
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_searchArea->setFocus();
    }
    return QWidget::keyPressEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        slotMaxWid();
    }
    return;
}

void TitleBar::slotAddFont()
{
    emit sigAddFont();
    return;
}

void TitleBar::slotMinWid()
{
    this->window()->showMinimized();
    return;
}

void TitleBar::slotMaxWid()
{
    if (this->window()->windowState() == Qt::WindowMaximized) {
        this->window()->showNormal();
        m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maxBtn->setToolTip(tr("Maximize"));
    } else {
        this->window()->showMaximized();
        m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maxBtn->setToolTip(tr("Normal"));
    }
    return;
}

void TitleBar::slotCloseWid()
{
    this->window()->close();
    return;
}

void TitleBar::slotTriggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("About") == str) {
        initAbout();
    } else if (tr("Help") == str) {
        initHelp();
    } else if (tr("Quit") == str) {
        slotCloseWid();
    }
    return;
}

void TitleBar::slotTitleSearch(QString search)
{
    /* 添加 搜索字体 SDK的打点 */
    GlobalSizeData::getInstance()->searchFontSDKPoint();
    emit sigTitleSearch(search);
    return;
}

void TitleBar::slotWindowState(QString state)
{
    if (state == "maxWindow") {
        m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maxBtn->setToolTip(tr("Normal"));
    } else {
        m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maxBtn->setToolTip(tr("Maximize"));
    }
    return;
}

void TitleBar::slotChangeFontName()
{
    m_searchArea->update();
    return;
}

void TitleBar::slotChangeFaltSize()
{
    this->setMinimumHeight(64);

    m_addFontBtn->setFixedSize(BTN_SIZE_MAX);
    m_minBtn->setFixedSize(BTN_SIZE_MAX);
    m_maxBtn->setFixedSize(BTN_SIZE_MAX);
    m_closeBtn->setFixedSize(BTN_SIZE_MAX);
    m_menuBtn->setFixedSize(BTN_SIZE_MAX);
    m_searchArea->setFixedSize(SEARCH_SIZE_MAX);
    m_maxBtn->hide();
    return;
}

void TitleBar::slotChangePCSize()
{
    this->setMinimumHeight(46);

    m_addFontBtn->setFixedSize(BTN_SIZE);
    m_minBtn->setFixedSize(BTN_SIZE);
    m_maxBtn->setFixedSize(BTN_SIZE);
    m_closeBtn->setFixedSize(BTN_SIZE);
    m_menuBtn->setFixedSize(BTN_SIZE);
    m_searchArea->setFixedSize(SEARCH_SIZE);
    m_maxBtn->show();
    return;
}
