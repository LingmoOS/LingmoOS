#include "menumodule.h"
#include "kyview.h"
#include "sizedate.h"
menuModule::menuModule(QWidget *parent = nullptr) : QWidget(parent)
{
    init();
}
menuModule::~menuModule()
{
    if (m_menu != nullptr) {
        delete m_menu;
    }
}

void menuModule::init()
{
    initAction();
}

void menuModule::initAction()
{

    m_bodyAppName = new QLabel();
    m_ft.setPixelSize(18); //先不用，之后要用
    m_bodyAppName->setFont(m_ft);
    m_bodyAppVersion = new QLabel();
    m_bodySupport = new QLabel();
    m_titleText = new QLabel();
    m_bodyAppDescribe = new QLabel();
    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(0,0,0,1)\">"
                             "support@kylinos.cn</a>");
    m_bodyAppVersion->setText(tr("Version: ") + m_appVersion);
    m_bodyAppDescribe->setText(tr("A system picture tool that can quickly open common formats. It provides zoom,flip "
                                  "and other processing simplely."));
    m_bodyAppDescribe->setWordWrap(true); //设置自动换行

    menuButton = new QToolButton(this);
    menuButton->setToolTip(tr("Option"));
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setIconSize(MICON_SIZES);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    if (platForm.contains(Variable::platForm)) {
        menuButton->setFixedSize(MBTN_SIZE_INTEL);
    } else {
        menuButton->setFixedSize(MBTN_SIZE);
    }

    menuButton->setAutoRaise(true);
    menuButton->setFocusPolicy(Qt::NoFocus);
    m_menu = new QMenu();

    QList<QAction *> actions;
    QAction *actionOpen = new QAction(m_menu);
    actionOpen->setText(tr("Open"));
    //    actionOpen->setFont(m_ft);
    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    //    actionTheme->setFont(m_ft);
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    //    actionHelp->setFont(m_ft);
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    //    actionAbout->setFont(m_ft);
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));
    //    actionQuit->setFont(m_ft);
    actions << actionOpen /*<<actionTheme*/ << actionHelp << actionAbout << actionQuit; //暂时禁掉主题切换按钮
    m_menu->addActions(actions);
    //    互斥按钮组
    QMenu *m_themeMenu = new QMenu;
    QActionGroup *m_themeMenuGroup = new QActionGroup(this);
    QAction *autoTheme = new QAction("Auto", this);
    m_themeMenuGroup->addAction(autoTheme);
    m_themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction("Light", this);
    m_themeMenuGroup->addAction(lightTheme);
    m_themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction("Dark", this);
    m_themeMenuGroup->addAction(darkTheme);
    m_themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction *> themeActions;
    themeActions << autoTheme << lightTheme << darkTheme;
    actionTheme->setMenu(m_themeMenu);
    menuButton->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
    connect(this, &menuModule::menuModuleClose, Interaction::getInstance(), &Interaction::close);
    connect(this, &menuModule::menuModuleClose, this, &menuModule::exitAppFromMenu);
    //    connect(Interaction::getInstance(),&Interaction::progremExit, KyView::mutual, &KyView::close);
    initGsetting();
}


void menuModule::triggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("Quit") == str) {
        Q_EMIT menuModuleClose();
    } else if (tr("About") == str) {
        Q_EMIT aboutShow();
    } else if (tr("Help") == str) {
        helpAction();
    } else if (tr("Open") == str) {
        Q_EMIT openSignal();
    }
}

void menuModule::aboutAction()
{
    //    关于点击事件处理

    initAbout();
}

void menuModule::helpAction()
{
    kdk::kabase::UserManualManagement userManualTest;
    if (!userManualTest.callUserManual("pictures")) {
        qCritical() << "user manual call fail!";
    }
    return;
}

void menuModule::initAbout()
{

    Q_EMIT aboutShow();
    m_aboutWindow->deleteLater();
    m_aboutWindow = new QDialog();
    m_aboutWindow->installEventFilter(this);
    m_aboutWindow->setWindowModality(Qt::ApplicationModal);
    m_aboutWindow->setWindowFlag(Qt::Tool);
    m_aboutWindow->setAutoFillBackground(true);
    m_aboutWindow->setBackgroundRole(QPalette::Base);

    m_aboutWindow->setFixedSize(M_ABOUT);
    //    m_aboutWindow->setMinimumHeight(M_ABOUT.height());
    QVBoxLayout *mainlyt = new QVBoxLayout();
    QHBoxLayout *titleLyt = initTitleBar();
    QVBoxLayout *bodylyt = initBody();
    mainlyt->setMargin(0);
    mainlyt->addLayout(titleLyt);
    mainlyt->addLayout(bodylyt);
    mainlyt->addStretch();
    m_aboutWindow->setLayout(mainlyt);
    // TODO:在中央显示
    QRect availableGeometry = this->parentWidget()->parentWidget()->geometry();
    m_aboutWindow->move(availableGeometry.center() - m_aboutWindow->rect().center());
    m_aboutWindow->show();
}

QHBoxLayout *menuModule::initTitleBar()
{
    //    QLabel* titleIcon = new QLabel();
    QPushButton *titleIcon = new QPushButton();
    titleIcon->setFixedSize(MICON_SIZEM);
    titleIcon->setIcon(QIcon::fromTheme("lingmo-photo-viewer", QIcon(":/res/res/kyview_logo.png")));
    titleIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
                             "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
                             "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}");
    titleIcon->setIconSize(MICON_SIZEM);
    m_appShowingName = tr("Pictures");
    m_iconPath = ":/res/res/kyview_logo.png";
    //    titleIcon->setPixmap(QPixmap::fromImage(QImage(m_iconPath)));
    //    titleIcon->setScaledContents(true);

    QPushButton *titleBtnClose = new QPushButton;
    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    titleBtnClose->setToolTip(tr("close"));
    titleBtnClose->setFixedSize(MBTN_SIZE);
    titleBtnClose->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    titleBtnClose->setProperty("isWindowButton", 0x2);
    titleBtnClose->setProperty("useIconHighlightEffect", 0x8);
    titleBtnClose->setFlat(true);
    connect(titleBtnClose, &QPushButton::clicked, [=]() {
        m_aboutWindow->close();
    });

    QHBoxLayout *hlyt = new QHBoxLayout;
    m_titleText->setText(tr("Pictures"));
    hlyt->setSpacing(0);
    hlyt->setMargin(4);
    hlyt->addSpacing(4);
    hlyt->addWidget(titleIcon, 0, Qt::AlignBottom); //居下显示
    hlyt->addSpacing(8);
    hlyt->addWidget(m_titleText, 0, Qt::AlignBottom);
    hlyt->addStretch();
    hlyt->addWidget(titleBtnClose, 0, Qt::AlignBottom);
    return hlyt;
}

QVBoxLayout *menuModule::initBody()
{
    m_appVersion = VERSION_NEM;
    //    QLabel* bodyIcon = new QLabel();
    QPushButton *bodyIcon = new QPushButton();
    bodyIcon->setFixedSize(MICON_SIZEB);
    //    bodyIcon->setPixmap(QPixmap::fromImage(QImage(m_iconPath)));
    bodyIcon->setIcon(QIcon::fromTheme("lingmo-photo-viewer", QIcon(":/res/res/kyview_logo.png")));
    //    bodyIcon->setStyleSheet("font-size:14px;background-color:transparent;");
    bodyIcon->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
                            "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
                            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}");
    bodyIcon->setIconSize(MICON_SIZEB);
    //    bodyIcon->setScaledContents(true);
    m_bodyAppName->setFixedHeight(NAME_HEIGHT);
    m_bodyAppDescribe->setFixedWidth(380);
    m_bodyAppDescribe->setMinimumHeight(DESCRIBE_HEI);
    m_bodyAppName->setText(tr(m_appShowingName.toLocal8Bit()));
    m_bodyAppVersion->setFixedHeight(VERSION_HEI);
    m_bodyAppVersion->setText(tr("Version: ") + m_appVersion);
    m_bodyAppVersion->setAlignment(Qt::AlignLeft);
    m_bodyAppDescribe->setAlignment(Qt::AlignLeft);

    connect(m_bodySupport, &QLabel::linkActivated, this, [=](const QString url) {
        QDesktopServices::openUrl(QUrl(url));
    });
    m_bodySupport->setFixedHeight(VERSION_HEI);
    m_bodySupport->setOpenExternalLinks(true);
    m_bodySupport->setContextMenuPolicy(Qt::NoContextMenu);
    QVBoxLayout *vlyt = new QVBoxLayout;

    vlyt->addWidget(bodyIcon, 0, Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(m_bodyAppName, 0, Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(m_bodyAppVersion, 0, Qt::AlignHCenter);
    vlyt->addSpacing(5);
    vlyt->addWidget(m_bodyAppDescribe, 0, Qt::AlignLeft);
    vlyt->addSpacing(5);
    vlyt->addWidget(m_bodySupport, 0, Qt::AlignLeft);
    vlyt->setContentsMargins(20, 10, 20, 10);
    return vlyt;
}

void menuModule::initGsetting()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        refreshThemeBySystemConf();
    });
    refreshThemeBySystemConf();
    return;
}

void menuModule::refreshThemeBySystemConf()
{
    QString themeNow = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("lingmo-dark" == themeNow || "lingmo-black" == themeNow) {
        setThemeDark();
    } else {
        setThemeLight();
    }
}

void menuModule::setThemeDark()
{

    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(255,255,255,1)\">"
                             "support@kylinos.cn</a>");
}

void menuModule::setMenuBtnSize(QSize btnSize)
{
    menuButton->setFixedSize(btnSize);
}

void menuModule::setThemeLight()
{

    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(0,0,0,1)\">"
                             "support@kylinos.cn</a>");
}
