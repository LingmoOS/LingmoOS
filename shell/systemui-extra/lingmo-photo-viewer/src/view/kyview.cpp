#include "kyview.h"
#include "sizedate.h"
#include "X11/Xlib.h"
#include <QX11Info>
#include <QWindow>
#include <lingmostylehelper/lingmostylehelper.h>
#include "windowmanager/windowmanager.h"
#include "global/interactiveqml.h"
KyView *KyView::mutual = nullptr;
KyView::KyView(const QStringList &args)
{

    Interaction::getInstance()->creatCore(args);
    //获取系统语言
    m_locale = QLocale::system().name();
    // 初始化按键键值
    m_keyState = "keyNone";

    m_gsettingThemeData = new QGSettings("org.lingmo.style");

    mutual = this;
    /* 初始化横竖屏的初值start */
    hOrVMode = new HorizontalOrVerticalMode;
    //标题栏
    m_titlebar = new TitleBar(this);
    m_titlebar->installEventFilter(this);

    hLayoutFlag = hOrVMode->defaultModeCapture();
    if (hLayoutFlag != deviceMode::PCMode) {
        m_isPcModeStarted = false;
    }

    this->setWindowSize();
    connect(hOrVMode, &HorizontalOrVerticalMode::RotationSig, this, &KyView::slotIntelHVModeChanged);
    /* 初始化横竖屏的初值end */
    initInteraction();

    this->setWindowTitle(tr("Pictures"));
    this->installEventFilter(this);
    //毛玻璃
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    //响应mouseMovEevent的无按压事件
    this->setMouseTracking(true);
    //响应拖拽事件
    this->setAcceptDrops(true);
    //焦点不在窗口上时鼠标悬浮可显示tooltips
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    m_titlebar->move(0, 0);

    //默认打开--打开图片按钮widget
    m_openImage = new OpenImage(this);
    m_openImage->setFixedSize(OPEN_IMAGESIZE);
    m_openImage->move(int((this->width() - m_openImage->width()) / 2),
                      int((this->height() - m_openImage->height()) / 2));
    m_openImage->hide();

    //工具栏
    m_toolbar = new ToolBar(this);
    m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
    this->toolBarHideOrShow(false);
    // 0910
    //标记工具栏
    m_markWid = new Marking(this);
    m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
    m_markWid->hide();
    //画笔设置弹窗
    m_brushSettings = new BrushSettings(this);
    m_brushSettings->hide();

    //设置置顶
    m_titlebar->setAttribute(Qt::WA_AlwaysStackOnTop);
    //图片展示界面
    m_showImageWidget = new ShowImageWidget(this);
    m_showImageWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_showImageWidget->setMouseTracking(true);
    m_showImageWidget->move(int((this->width() - m_showImageWidget->width()) / 2),
                            int((this->height() - m_showImageWidget->height()) / 2));
    m_showImageWidget->hide();
    //导航器
    m_navigator = new Navigator(this);
    //    m_navigator->resize(NAVI_SIZE);
    if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
        m_navigator->move(7, this->height() - 70 - m_navigator->height());
    } else {
        m_navigator->move(this->width() - 7 - m_navigator->width(), this->height() - 70 - m_navigator->height());
    }
    m_navigator->hide();

    //信息栏
    m_information = new Information(this);
    if (m_local.system().name() == "en_US") {
        m_information->resize((INFOR_SIZE.width() + 7) * m_detio, INFOR_SIZE.height() * m_detio);
    } else if (m_local.system().name() == "zh_CN") {
        m_information->resize(INFOR_SIZE * m_detio);
    } else {
        m_information->resize((INFOR_SIZE.width() + 7) * m_detio, INFOR_SIZE.height() * m_detio);
    }
    m_inforSize = m_information->size();
    if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
        m_information->move(0, titlebarHeightValue);
    } else {
        m_information->move(this->width() - m_information->width(), titlebarHeightValue);
    }
    m_information->setMouseTracking(true);
    //设置鼠标穿透-解决鼠标移动到顶栏位置，顶栏不弹出的问题
    //m_information->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_information->hide();
    m_information->installEventFilter(this);
    //相册
    m_sideBar = new SideBar(this);
    m_sideBar->hide();
    m_sideBar->installEventFilter(this);
    //定时器
    m_timer = new QTimer(this);
    m_timernavi = new QTimer(this);
    m_timeNomove = new QTimer(this);
    m_timerSidebar = new QTimer(this);

    //应用居中
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
        int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
        leftPoint = QPoint((sw - this->width()) / 2, (sh - this->height()) / 2);
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(leftPoint, QSize(this->width(), this->height())));
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        this->move((screen->geometry().width() - this->width()) / 2,
                   (screen->geometry().height() - this->height()) / 2);
    }

    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
            if (getpid() == kdk::WindowManager::getPid(window_id) && m_isFirst)
            {
                m_windowId = window_id;
                m_isFirst = false;
                kdk::WindowManager::activateWindow(m_windowId);
            }
        });
    }

    this->setMinimumSize(m_miniSize);
    this->initconnect();
    //要判断打开状态，是否隐藏主界面--打开按钮widget
    this->openState();
    this->initGsetting();
    this->initFontSize();
    //初始化手势
    this->initGrabGesture();
    //任务栏尺寸改变处理
    this->connectToPanelSizeChange();
}
KyView::~KyView()
{
    delete hOrVMode;
    hOrVMode = nullptr;
}
// 获取任务栏尺寸改变
void KyView::connectToPanelSizeChange()
{
    if (QGSettings::isSchemaInstalled("org.lingmo.panel.settings")) {
        panelsetting = new QGSettings("org.lingmo.panel.settings");
        QObject::connect(panelsetting, &QGSettings::changed, [=](const QString &key){
            if (key == "panelsize") {
                int panelSize = panelsetting->get(key).toInt();
                handlePanelSizeChanged(panelSize);
            }
        });
    }
}

// 处理任务栏尺寸改变
void KyView::handlePanelSizeChanged(int size)
{
    if (this->isMaximized()||this->isFullScreen() || m_nowFullScreen || m_isMaxAndFull) {
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
            int sh = QGuiApplication::primaryScreen()->availableGeometry().height()- size;
            this->resize(sw, sh);
            kdk::WindowManager::setGeometry(this->windowHandle(), QRect((sw - this->width()) / 2, (sh - this->height()) / 2, this->width(), this->height()));
        }
        this->showMaximized();
     }
}
void KyView::slotIntelHVModeChanged(deviceMode sig)
{
    hLayoutFlag = hOrVMode->defaultModeCapture();
    if (hLayoutFlag == PADHorizontalMode || hLayoutFlag == PADVerticalMode) {
        m_isResponseDbClick = false;
        Variable::platForm = "pc";
    } else {
        m_isResponseDbClick = true;
        Variable::platForm = "normal";
    }

    this->setTitleBtnHide();
}

void KyView::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::fullScreen, this, &KyView::fullScreen);
}


//信号和槽
void KyView::initconnect()
{
    //最大化和还原
    connect(m_titlebar, &TitleBar::recovery, this, &KyView::changOrigSize);
    //右上菜单--打开
    connect(m_titlebar, &TitleBar::openSignal, m_openImage, &OpenImage::openImagePath);
    //打开关于，两栏隐藏
    connect(m_titlebar, &TitleBar::aboutShow, this, &KyView::aboutShow);
    //更新信息栏
    connect(m_titlebar, &TitleBar::updateInformation, m_information, &Information::updateName);
    connect(m_titlebar, &TitleBar::exitApp, this, &KyView::needExitApp);
    //判断为相册处切换图片，刷新动图右键内容
    connect(m_sideBar, &SideBar::changeImage, m_showImageWidget, &ShowImageWidget::albumChangeImage);
    //打开图片
    connect(m_openImage, &OpenImage::openImageSignal, m_showImageWidget, &ShowImageWidget::openImage);
    connect(m_openImage, &OpenImage::openEmptyFile, m_sideBar, &SideBar::openEmptyFile);
    //改变顶栏显示的图片名字
    connect(m_showImageWidget, &ShowImageWidget::perRate, m_toolbar, &ToolBar::changePerRate);
    //打开图片--主界面的界面显示处理
    connect(m_showImageWidget, &ShowImageWidget::toShowImage, this, &KyView::toShowImage);
    //获取图片详细信息
    connect(m_showImageWidget, &ShowImageWidget::changeInfor, m_information, &Information::contentText);
    //顶栏图片名字
    connect(m_showImageWidget, &ShowImageWidget::titleName, m_titlebar, &TitleBar::showImageName);
    //清空相册时，展示的界面
    connect(m_showImageWidget, &ShowImageWidget::clearImage, this, &KyView::clearImage);
    //当图片大于2张及以上，默认展示相册---先保留
    //    connect(m_showImageWidget,&ShowImageWidget::toShowSide,this,&KyView::defaultSidebar);
    //相册尺寸改变，位置改变
    connect(m_sideBar, &SideBar::sizeChange, this, &KyView::albumPosChange);
    //重命名
    connect(m_showImageWidget, &ShowImageWidget::toRename, m_titlebar, &TitleBar::needRename);
    //重命名，显示两栏
    connect(m_showImageWidget, &ShowImageWidget::isRename, this, &KyView::startRename);
    //展示或隐藏图片信息窗口
    connect(m_toolbar, &ToolBar::showInfor, this, &KyView::showInforWid);
    //导航器出现时，位置变化
    connect(m_navigator, &Navigator::naviChange, this, &KyView::naviChange);
    //显示相册
    connect(m_toolbar, &ToolBar::showSidebar, this, &KyView::showSidebar);
    //定时器
    connect(m_timer, &QTimer::timeout, this, &KyView::delayHide);
    connect(m_timernavi, &QTimer::timeout, this, &KyView::delayHide_navi);
    connect(m_timeNomove, &QTimer::timeout, this, &KyView::delayHide_move);
    m_timeNomove->start(2500);
    connect(m_timerSidebar, &QTimer::timeout, this, &KyView::delayHide_sidebar);
    //设置相册尺寸
    connect(m_showImageWidget, &ShowImageWidget::changeSideSize, [=](const int &type) {
        m_sideBar->getSelect(type);
        this->setSidebarState(type);
    });
    connect(this, &KyView::albumState, m_showImageWidget, &ShowImageWidget::albumSlot);
    //删除时界面变化
    connect(m_toolbar, &ToolBar::isDelete, m_showImageWidget, &ShowImageWidget::isDelete);
    connect(m_showImageWidget, &ShowImageWidget::needDelete, m_toolbar, &ToolBar::delImage);
    //    connect(m_showImageWidget,&ShowImageWidget::toSelectHigh,m_sideBar,&SideBar::isDelete);
    connect(m_titlebar, &TitleBar::doubleClick, this, &KyView::dealDoubleClick);
    //判断打开图片成功与否和无权限删除文件，界面应相应的做出改变
    connect(m_showImageWidget, &ShowImageWidget::fileState, m_toolbar, &ToolBar::setButtonState);
    connect(m_showImageWidget, &ShowImageWidget::fileState, m_titlebar, &TitleBar::setRenameEnable);
    //设置信息栏大小
    connect(this, &KyView::changeInforSize, m_information, &Information::changeEverySize);
    connect(m_toolbar, &ToolBar::hideTwoBar, this, &KyView::hideTwoBar); //开始裁剪，隐藏相关控件
    connect(m_toolbar, &ToolBar::startCutting, m_showImageWidget,
            &ShowImageWidget::startCutColor); //裁剪时改变界面背景颜色
    connect(m_showImageWidget, &ShowImageWidget::needExitCut, this, &KyView::exitCut); //退出裁剪
    connect(m_showImageWidget, &ShowImageWidget::sendImageType, m_toolbar, &ToolBar::getImageType);
    connect(m_titlebar, &TitleBar::updateFileInfo, m_toolbar, &ToolBar::updateFileInfo); //更新工具栏文件信息
    connect(m_navigator, &Navigator::navigatorState, m_showImageWidget, &ShowImageWidget::navigatorState);
    connect(m_navigator, &Navigator::notifyNotUpdated, m_showImageWidget, &ShowImageWidget::navigatorMove);
    connect(m_toolbar, &ToolBar::viewOCRMode, this, &KyView::viewOCRMode); //隐藏其他控件进入OCR模式
    connect(m_showImageWidget, &ShowImageWidget::viewExitOCRMode, this, &KyView::exitOCRMode); //退出OCR模式进入普通模式
    connect(m_toolbar, &ToolBar::startGetText, m_showImageWidget,
            &ShowImageWidget::startGetText); //调整为OCR模式对应样式
    connect(Interaction::getInstance(), &Interaction::getTextFinish, [=](QVector<QString> result) {
        m_showImageWidget->setGetTextResult(result);
    });
    connect(m_showImageWidget, &ShowImageWidget::startWayToSetTitleStyle, m_titlebar, &TitleBar::setLeftUpBtnStyle);
    connect(m_titlebar, &TitleBar::updateImageWidName, m_showImageWidget, &ShowImageWidget::updateImagePath);
    connect(m_showImageWidget, &ShowImageWidget::sendSuffixToToolbar, m_toolbar, &ToolBar::getSuffix);
    connect(m_showImageWidget, &ShowImageWidget::decideMainWidDisplay, this, &KyView::setWidDisplay);
    connect(m_toolbar, &ToolBar::startZoomImage, m_showImageWidget, &ShowImageWidget::operatyImage);
    connect(m_showImageWidget, &ShowImageWidget::toCloseNavigator, m_navigator, &Navigator::showNavigation);
    connect(m_navigator, &Navigator::needUpdateImagePosFromMainWid, m_showImageWidget,
            &ShowImageWidget::needUpdateImagePosFromMainWid);
    connect(m_navigator, &Navigator::needUpdateQmlImagePos, m_showImageWidget,
            &ShowImageWidget::noticeQmlAdjustImagePos);
    connect(m_showImageWidget, &ShowImageWidget::doubleEventToMainWid, this, &KyView::dealMouseDouble);
    //展示标注工具栏
    connect(m_toolbar, &ToolBar::showMarkingWid, this, &KyView::toShowMarkWid);
    //展示画笔设置小窗口
    connect(m_markWid, &Marking::showSetBar, this, &KyView::toShowBrushWid);
    //设置画笔设置小窗口的位置和小三角位置--保持与按钮一一对应
    connect(m_markWid, &Marking::showSetPos, this, &KyView::toSetBrushPos);
    connect(m_markWid, &Marking::undoSignal, this, &KyView::undoOperate);
    connect(m_markWid, &Marking::exitSign, this, &KyView::exitSign);
    connect(m_markWid, &Marking::finishSign, this, &KyView::finshSign);
    connect(m_toolbar, &ToolBar::markUpdateImage, m_showImageWidget, &ShowImageWidget::openImage);
    connect(m_showImageWidget, &ShowImageWidget::enterSign, m_toolbar, &ToolBar::enterSign);
    connect(m_showImageWidget, &ShowImageWidget::startSignToToolbar, m_toolbar, &ToolBar::labelbar);
    connect(m_showImageWidget, &ShowImageWidget::adaptiveWidgetSignal, m_toolbar, &ToolBar::adaptiveWidget);
    connect(m_showImageWidget, &ShowImageWidget::sendMousePressOrNot, this, &KyView::getMouseStateFromQml);

    //背景替换
    connect(m_toolbar, &ToolBar::startMatting, m_showImageWidget, &ShowImageWidget::enterReplaceBackground);
    connect(m_showImageWidget, &ShowImageWidget::initMattingConnect, this, &KyView::initMattingConnect);

    //扫描黑白件
    connect(m_toolbar, &ToolBar::startScanner, m_showImageWidget, &ShowImageWidget::enterScanImage);
    connect(m_showImageWidget, &ShowImageWidget::initScannerConnect, this, &KyView::initScannerConnect);
}

void KyView::initMattingConnect()
{
    connect(m_showImageWidget, &ShowImageWidget::toReplaceBackground, this, &KyView::enterReplaceBackgroundMode);
    connect(m_showImageWidget, &ShowImageWidget::quitReplaceBackground, this, &KyView::exitReplaceBackgroundMode);
}

//进入背景替换模式
void KyView::enterReplaceBackgroundMode()
{
    m_showImageWidget->m_replaceBackgroundWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_replaceBackgroundWid->move((this->width() - m_showImageWidget->m_replaceBackgroundWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_replaceBackgroundWid->height());
    m_isReplacing = true;
    displayMode = DisplayMode::MattingMode;
    this->toolBarHideOrShow(false);
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
    if (!m_information->isHidden()) {
        m_information->hide();
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    if (!m_navigator->isHidden()) {
        m_isNavigatorShow = true;
        m_navigator->hide();
    } else {
        m_isNavigatorShow = false;
    }
    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
}
//退出背景替换模式
void KyView::exitReplaceBackgroundMode()
{
    m_isReplacing = false;
    displayMode = DisplayMode::NormalMode;
    if (m_isNavigatorShow) {
        m_navigator->show();
        this->naviChange();
    }
    this->restoreAllBallState();
    this->setFocus();
}

void KyView::initScannerConnect()
{
    connect(m_showImageWidget, &ShowImageWidget::toScanImage, this, &KyView::enterScanImageMode);
    connect(m_showImageWidget, &ShowImageWidget::quitScanImage, this, &KyView::exitScanImageMode);
}

void KyView::enterScanImageMode()
{
    m_showImageWidget->m_scanImageWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_scanImageWid->m_showImageWidSize = m_showImageWidget->m_scanImageWid->size();
    m_showImageWidget->m_scanImageWid->move((this->width() - m_showImageWidget->m_scanImageWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_scanImageWid->height());
    m_isScanning = true;
    displayMode = DisplayMode::ScannerMode;
    this->setMinimumSize(QSize(810,600));
    this->toolBarHideOrShow(false);
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
    if (!m_information->isHidden()) {
        m_information->hide();
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    if (!m_navigator->isHidden()) {
        m_isNavigatorShow = true;
        m_navigator->hide();
    } else {
        m_isNavigatorShow = false;
    }
    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
}

void KyView::exitScanImageMode()
{
    m_isScanning = false;
    displayMode = DisplayMode::NormalMode;
    if (m_isNavigatorShow) {
        m_navigator->show();
        this->naviChange();
    }
    this->setMinimumSize(m_miniSize);
    this->restoreAllBallState();
    this->setFocus();
}

//展示标注工具栏
void KyView::toShowMarkWid()
{
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
    displayMode = DisplayMode::SignMode;
    Interaction::getInstance()->startMark();
    InteractiveQml::getInstance()->setPainterType(-1);
    m_markWid->clearButtonState();
    m_markWid->show();
    this->toolBarHideOrShow(false);
    if (!m_information->isHidden()) {
        m_information->hide();
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
    m_titlebar->hide();
}

void KyView::exitSign()
{
    m_markWid->hide();
    m_brushSettings->hide();
    //给showimage界面发信号，去检查是否需要弹窗提示，进行下一步的操作
    m_showImageWidget->exitSign();
    Interaction::getInstance()->markClear();
    displayMode = DisplayMode::NormalMode;
    this->restoreAllBallState();
}

void KyView::finshSign()
{
    displayMode = DisplayMode::NormalMode;
    m_markWid->hide();
    m_brushSettings->hide();
    //    qDebug() << "完成标注";
    //给showimage界面发出信号，去检查是否需要弹窗提示，进行下一步的操作
    m_showImageWidget->finishSign();
    this->restoreAllBallState();
}

void KyView::undoOperate()
{
    //    qDebug() << "撤销操作";
    //给showimage界面发出信号，撤销上一步的操作
    m_showImageWidget->undoOperate();
    Interaction::getInstance()->markUndo();
}

void KyView::exitSignComplete()
{
    //    qDebug() << "真正的退出了标注";
    // core::发送给前端显示的图片
    // showimagewid::只有在图片其实什么都没有被改变的时候，调用这个函数，其余情况需要由后端来给前端发信号
    //    Variable::g_allowChangeImage = true;
    //    Variable::g_needImageSaveTip = false;
    //    m_markWid->hide();
}
//标注工具栏位置变化
void KyView::markChange()
{
    m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
    m_recordMarkNowPos = m_markWid->pos();
    m_recordMarkMovePos = m_recordMarkNowPos - m_recordMarkOldPos;
}
//展示画笔设置小弹窗
void KyView::toShowBrushWid(bool isShow, bool isText)
{
    if (isShow) {
        m_brushSettings->setSize(isText);
        m_brushSettings->show();
    } else {
        m_brushSettings->hide();
    }
    this->naviChange();
}
//设置画笔设置小弹窗的小三角起始位置和弹出位置--与按钮一一对应
void KyView::toSetBrushPos(int startPos, int showPosX)
{
    m_recordBrushStartPos = startPos;
    m_brushSettings->setStartPos(startPos);
    m_brushSettings->setShowPos(m_markWid->x() - 95 + showPosX, m_markWid->y() - 6 - m_brushSettings->height());
    m_recordMarkOldPos = m_markWid->pos();
}
//画笔设置小窗口位置变化
void KyView::brushSettingsChange()
{
    m_brushSettings->setStartPos(m_recordBrushStartPos);
    m_brushSettings->setShowPos(m_brushSettings->x() + m_recordMarkMovePos.x(),
                                m_brushSettings->y() + m_recordMarkMovePos.y());
}

//打开首先检测是否需要展示工具栏
void KyView::openState()
{
    if (!m_openImage->isHidden()) {
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
        this->toolBarHideOrShow(false);
    }
}
//打开关于，两栏隐藏
void KyView::aboutShow()
{
    if (m_openImage->isHidden()) {
        if (!m_titlebar->isHidden()) {
            m_titlebar->hide();
            this->toolBarHideOrShow(false);
        }
        if (!m_information->isHidden()) {
            if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
                m_information->move(0, 0);
            } else {
                m_information->move(this->width() - m_information->width(), 0);
            }
        }
    }

    m_aboutDialog = new KAboutDialog(this);
    m_aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_aboutDialog->setBodyTextVisiable(true);
    kdk::LingmoUIStyleHelper::self()->removeHeader(m_aboutDialog);
    this->initAboutDialog();

    m_aboutDialog->adjustSize();
    m_aboutDialog->move(mutual->geometry().center().x() - m_aboutDialog->width() / 2,
                        mutual->geometry().center().y() - m_aboutDialog->height() / 2);
    kdk::WindowManager::setGeometry(m_aboutDialog->windowHandle(),m_aboutDialog->geometry());

    m_aboutDialog->exec();
}

void KyView::startRename()
{
    if (!m_markWid->isHidden()) {
        m_titlebar->show();
        this->toolBarHideOrShow(false);
        return;
    }
    m_titlebar->show();
    this->toolBarPostionSet();
    this->toolBarHideOrShow(true);
}
//顶栏大小随主界面大小改变的响应函数
void KyView::titlebarChange()
{
    m_titlebar->move(0, 0);
    m_titlebar->resize(this->width(), titlebarHeightValue);
    m_titlebar->g_titleWid->resize(this->width(), titlebarHeightValue);
}
//中间打开图片按钮大小随主界面大小改变的响应函数
void KyView::openImageChange()
{
    if (m_openImage->isHidden()) {
        return;
    }
    m_openImage->move(int((this->width() - m_openImage->width()) / 2),
                      int((this->height() - m_openImage->height()) / 2));
}
//主界面展示的图片大小随主界面大小改变的响应函数
void KyView::showImageChange()
{
    m_showImageWidget->move(0, 0);
    m_showImageWidget->resize(this->width(), this->height());
    m_showImageWidget->reMove(this->width(), this->height());
}

void KyView::mattingChange()
{
    m_showImageWidget->m_replaceBackgroundWid->move(0, 0);
    m_showImageWidget->m_replaceBackgroundWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_replaceBackgroundWid->move((this->width() - m_showImageWidget->m_replaceBackgroundWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_replaceBackgroundWid->height());
    m_showImageWidget->m_replaceBackgroundWid->reMove(this->width(), this->height() - titlebarHeightValue);
}

void KyView::scannerChange()
{
    m_showImageWidget->m_scanImageWid->move(0, 0);
    m_showImageWidget->m_scanImageWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_scanImageWid->move((this->width() - m_showImageWidget->m_scanImageWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_scanImageWid->height());
}
//工具栏大小随主界面大小改变的响应函数
void KyView::toolbarChange()
{
    if (m_toolbar->isHidden()) {
        return;
    }
    m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
}
//导航栏位置变化
void KyView::naviChange()
{
    if (m_navigator->isHidden()) {
        return;
    }
    if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
        m_navigator->move(7, this->height() - 52 - m_navigator->height());
    } else {
        m_navigator->move(this->width() - 7 - m_navigator->width(), this->height() - 52 - m_navigator->height());
    }
}
//信息栏随顶栏位置变化而变化
void KyView::inforChange()
{
    if (m_information->isHidden()) {
        return;
    }
    if (m_titlebar->isHidden()) {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_information->move(0, 0);
        } else {
            m_information->move(this->width() - m_information->width(), 0);
        }
    } else {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_information->move(0, titlebarHeightValue);
        } else {
            m_information->move(this->width() - m_information->width(), titlebarHeightValue);
        }
    }
}

void KyView::albumChange()
{
    if (m_sideBar->isHidden()) {
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        return;
    }
    if (m_titlebar != nullptr && m_sideBar != nullptr && m_toolbar != nullptr) {
        Variable::g_acturalWidHeight = this->height() - m_titlebar->height() - TOOLBAR_SIZE.height() - 10 - 6 - 50;
        if (m_backWidHeight != Variable::g_acturalWidHeight) {
            m_backWidHeight = Variable::g_acturalWidHeight;
            m_sideBar->setItemNum(Variable::g_acturalWidHeight);
        }
    }
    albumPosChange();
}

void KyView::albumPosChange()
{
    if (m_sideBar->isHidden()) {
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        return;
    }
    if (Variable::g_needChangeAlbumPos) {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_sideBar->move(this->width() - m_sideBar->width(), (this->height() - m_titlebar->height() - m_toolbar->height() - 6 - m_sideBar->height()) / 2
                                   + m_titlebar->height());
        } else {
            m_sideBar->move(0, (this->height() - m_titlebar->height() - m_toolbar->height() - 6 - m_sideBar->height()) / 2
                                   + m_titlebar->height());
        }
    } else {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_sideBar->move(this->width() - m_sideBar->width(), 39 + m_titlebar->height());
        } else {
            m_sideBar->move(0, 39 + m_titlebar->height());
        }
    }
    m_showImageWidget->g_back->move(m_sideBar->x() + m_sideBar->width() + 42, m_showImageWidget->g_next->y());
}
//延时隐藏
void KyView::delayHide()
{
    if (this->mapFromGlobal(QCursor::pos()).y() < BAR_HEIGHT
        || this->mapFromGlobal(QCursor::pos()).y() >= this->height() - toolbarHeightValue
        || displayMode == DisplayMode::OCRMode
        || displayMode == DisplayMode::MattingMode
        || displayMode == DisplayMode::ScannerMode) {
        return;
    }
    if (!m_titlebar->g_myEdit->isHidden() || !m_toolbar->m_editMenu->isHidden()) {
        m_titlebar->show();
        this->toolBarPostionSet();
        this->toolBarHideOrShow(true);
        return;
    }
    if (!m_titlebar->isHidden() && !m_titlebar->g_menu->m_menu->isHidden()
            || displayMode == DisplayMode::OCRMode
            || displayMode == DisplayMode::MattingMode
            || displayMode == DisplayMode::ScannerMode) {
        m_titlebar->show();
        this->toolBarPostionSet();
        this->toolBarHideOrShow(true);
    } else {
        m_titlebar->hide();
        this->toolBarHideOrShow(false);
    }

    inforChange();
}

void KyView::delayHide_sidebar()
{
    if (!m_sideBar->isHidden() && m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    m_sideBar->hide();
    m_albumState = false;
    albumChange();
}

//鼠标离开界面时需要触发，届时会加上对导航器的处理
void KyView::delayHide_navi()
{
    if (displayMode == DisplayMode::OCRMode
            || displayMode == DisplayMode::MattingMode
            || displayMode == DisplayMode::ScannerMode) {
        return;
    } else if (!m_titlebar->g_myEdit->isHidden() || !m_toolbar->m_editMenu->isHidden()) {
        m_titlebar->show();
        this->toolBarPostionSet();
        this->toolBarHideOrShow(true);
        return;
    }
    m_titlebar->hide();
    this->toolBarHideOrShow(false);
    m_sideBar->hide();
    inforChange();
    m_timernavi->stop();
}
//鼠标静止不动，延时两秒两栏隐藏
void KyView::delayHide_move()
{
    if (!m_openImage->isHidden()) {
        return;
    }
    if (!m_titlebar->isHidden() && m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (!m_toolbar->isHidden() && m_toolbar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (!m_sideBar->isHidden() && m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
        return;
    }
    if (!m_titlebar->g_myEdit->isHidden() || !m_toolbar->m_editMenu->isHidden()) {
        m_titlebar->show();
        this->toolBarPostionSet();
        this->toolBarHideOrShow(true);
        return;
    }
    //如果下拉菜单show，则标题栏必须show
    if (!m_titlebar->g_menu->m_menu->isHidden()) {
        return;
    }
    //解决刚一打开图片，不到两秒就隐藏的问题
    if (m_timeNomove->isActive()) {
        m_timeNomove->stop();
    }
    m_titlebar->hide();
    this->toolBarHideOrShow(false);
    m_sideBar->hide();
    m_albumState = false;
    albumChange();
    inforChange();
    m_timeNomove->stop();
}
//展示信息栏
void KyView::showInforWid()
{
    if (m_inforState == true) {
        m_information->show();
        m_inforState = false;
        inforChange();
    } else {
        m_information->hide();
        m_inforState = true;
    }
}
//列表为空-清空图片--回到默认的打开界面
void KyView::clearImage()
{
    //    if (Variable::g_mainlineVersion) {
    //三个time->stop,解当图片全部删除时返回默认界面，两栏偶现消失的问题
    if (m_timeNomove->isActive()) {
        m_timeNomove->stop();
    }
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    if (m_timerSidebar->isActive()) {
        m_timerSidebar->stop();
    }
    m_openImage->show();
    m_showImageWidget->hide();
    this->toolBarHideOrShow(false);
    m_information->hide();
    m_titlebar->show();
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    m_titlebar->g_imageName->clear();
    m_isCutting = false;                   //重置裁剪标志
    displayMode = DisplayMode::NormalMode; //返回普通模式
    openImageChange();
}

void KyView::dealDoubleClick(bool isleaveOrNot)
{
    m_isLeave = isleaveOrNot;
}
//进入裁剪模式
void KyView::hideTwoBar()
{
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
        m_albumState = true;
    } else {
        m_albumState = false;
    }
    if (!m_information->isHidden()) {
        m_information->hide();
        m_inforState = true;
    }
    //进入裁剪，记录导航栏状态
    if (!m_navigator->isHidden()) {
        m_isNavigatorShow = true;
        m_navigator->hide();
    } else {
        m_isNavigatorShow = false;
    }
    m_showImageWidget->g_back->hide();
    m_showImageWidget->g_next->hide();
    m_titlebar->hide();
    this->toolBarHideOrShow(false);
    m_isCutting = true;
    displayMode = DisplayMode::NormalMode; //返回普通模式
    this->setFixedSize(this->size());
}

void KyView::exitCut()
{
    displayMode = DisplayMode::NormalMode;
    //退出裁剪，还原导航栏状态
    if (m_isNavigatorShow) {
        m_navigator->show();
        this->naviChange();
    }
    this->restoreAllBallState();
    m_isCutting = false;
    this->setFixedSize(WIDGETSIZE_MAX, WIDGETSIZE_MAX);
    this->setMinimumSize(m_miniSize);
    this->setFocus();
    if (m_mousePress) {
        m_mousePress = false;
    }
}
void KyView::viewOCRMode()
{
    m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    displayMode = DisplayMode::OCRMode;
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
        m_albumState = true;
    }
    if (!m_information->isHidden()) {
        m_information->hide();
    }
    if (!m_navigator->isHidden()) {
        m_navigator->hide();
    }
    m_showImageWidget->g_back->hide();
    m_showImageWidget->g_next->hide();
    m_showImageWidget->startGetText();
    this->toolBarHideOrShow(false);
    this->resize(this->size());
}

void KyView::exitOCRMode()
{
    displayMode = DisplayMode::NormalMode;
    this->restoreAllBallState();
}

void KyView::needExitApp()
{
    m_showImageWidget->needExitApp();
}

void KyView::setSidebarState(int num)
{
    if (!m_recordFirstOpen) {
        return;
    }
    m_recordFirstOpen = false;
    int sideBarState = Variable::getSettings("sidebarState").toInt();
    if (sideBarState == 0) {
        if (num > 2) {
            m_sideBar->show();
        } else {
            m_sideBar->hide();
        }
    } else if (sideBarState == 1) {
        m_sideBar->show();
    } else if (sideBarState == 2) {
        m_sideBar->hide();
    }
    this->albumChange();
    this->toolBarPostionSet();
    this->toolBarHideOrShow(false);
    m_titlebar->hide();
}

void KyView::getMouseStateFromQml()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    if (m_timeNomove->isActive()) {
        m_timeNomove->stop();
    }
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    if (m_toolbar->isHidden()) {
        if (displayMode == DisplayMode::NormalMode
                || displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
            if (m_titlebar->isHidden()) {
                m_titlebar->raise();
                m_titlebar->show();
            }
        }
        if (displayMode == DisplayMode::NormalMode) {
            if (m_toolbar->isHidden()) {
                this->toolBarPostionSet();
                this->toolBarHideOrShow(true);
            }
        }
        m_clickController = true;
    } else {
        if (!m_toolbar->isHidden()) {
            this->toolBarHideOrShow(false);
        }
        if (displayMode != DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
            if (!m_titlebar->isHidden()) {
                m_titlebar->hide();
            }
        }
        m_clickController = false;
    }
    this->inforChange();
}
void KyView::setWidDisplay(bool isDefaultDisplay)
{
    m_openImage->setVisible(isDefaultDisplay);
    m_showImageWidget->setVisible(!isDefaultDisplay);
    this->toolBarHideOrShow(!isDefaultDisplay);
}
//处理鼠标悬浮两栏的界面展示
void KyView::hoverChange(int y)
{
    static int i = 0;

    if (i == 0) {
        if (!m_clickController) {
            m_timer->start(2500);
            m_timerSidebar->start(2500);
        }
    }
    int hoverToolbarDisTop = this->height() - toolbarHeightValue - 10;
    if (y <= titlebarHeightValue || y >= hoverToolbarDisTop) {
        //需要获取从两栏到中间的变化，且为防止频繁操作
        if (m_timestart == false) {
            if (m_timer->remainingTime() < 1000 && m_timer->isActive()) {
                m_timer->stop();
            }
            m_timestart = true;
        }
        if (m_markWid->isHidden()) {
            this->toolBarPostionSet();
            m_toolbar->show();
        }
        m_titlebar->show();

        //顶栏和标题栏位置的变化
        toolbarChange();
        titlebarChange();

        this->m_showImageWidget->lower();
        this->m_showImageWidget->g_next->hide();
        this->m_showImageWidget->g_back->hide();

        //判断具体在顶栏或工具栏的区域，将其raise
        if (y <= titlebarHeightValue) {
            m_titlebar->raise();
        } else if (y >= this->height() - toolbarHeightValue - 10) {
            m_toolbar->raise();
        }
        //信息栏位置的变化
        inforChange();
        return;
    }

    if (m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        if (m_sideBar->isHidden()) {
            m_sideBar->show();
            m_sideBar->raise();
            m_albumState = true;
        }
        this->m_showImageWidget->lower();
        this->m_showImageWidget->g_next->show();
        this->m_showImageWidget->g_back->show();
        albumChange();
    }

    if (m_timestart == true) {

        if ((m_timer->remainingTime() < 2000 && m_timer->isActive()) || !(m_timer->isActive())) {
            m_timer->stop();
            if (!m_clickController) {
                m_timer->start(2500);
            }
        }
        m_timestart = false;
    }
    //判断定时器是否正在计时。如是，则停止
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }

    //判断列表中是否只有一张图，一张图片左右按钮不显示
    if (m_showImageWidget->g_buttonState == false) {
        this->m_showImageWidget->g_next->hide();
        this->m_showImageWidget->g_back->hide();
    } else {
        if (m_markWid->isHidden()) {
            this->m_showImageWidget->g_next->show();
            this->m_showImageWidget->g_back->show();
        }
    }
}
//读取主题配置文件
void KyView::initGsetting()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        themeChange();
    });
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemTransparencyChange, this, [=]() {
        transChange();
    });
    themeChange();
    transChange();
    return;
}
//主题变化
void KyView::themeChange()
{
    QString themeIcon = m_gsettingThemeData->get("icon-theme-name").toString();
    QString icon = "";
    if (themeIcon == "lingmo-icon-theme-classical") {
        icon = "qrc:/res/res/kyview_classical.png";
    } else if (themeIcon == "lingmo-icon-theme-fashion") {
        icon = "qrc:/res/res/kyview_fashion.png";
    } else {
        icon = "qrc:/res/res/kyview_logo.png";
    }
    InteractiveQml::getInstance()->setThemeIcon(icon);

    QString themeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("lingmo-dark" == themeStyle || "lingmo-black" == themeStyle) {
        m_information->setStyleSheet(
            "QWidget{background-color:rgba(0,0,0,0.80);border-top-left-radius:0px;border-top-right-radius：0px;border-"
            "bottom-left-radius:4px;border-bottom-right-radius:0px;}");
        m_titlebar->g_menu->setThemeDark();
        m_showImageWidget->g_next->setIcon(QIcon(":/res/res/1right.svg"));
        m_showImageWidget->g_back->setIcon(QIcon(":/res/res/1left.svg"));
        m_sideBar->setStyleSheet(
            "QListView{border:1px "
            ";border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-left-radius:"
            "0px;border-bottom-right-radius:4px;outline:none;background:rgba(63, 69, 77, 1)}"
            "QListView::item{margin:2 0px 0 0;background:rgba(0, 0, 0, 0.4);border-radius:4px;}"
            "QListView::item:selected{margin:0 0px 0 0;border:2px solid rgba(13, 135, 255, 1);background:rgba(0, "
            "0, 0, 0.4);border-radius:4px;}"
            "QListView::item:hover{background:rgba(0, 0, 0, 0.4);border-radius:4px;}");
    } else {
        m_information->setStyleSheet("QWidget{background-color:rgba(225, 240, 250, "
                                     "0.85);border-top-left-radius:0px;border-top-right-radius：0px;border-bottom-left-"
                                     "radius:4px;border-bottom-right-radius:0px;}");
        m_sideBar->setStyleSheet(
            "QListView{border:1px "
            ";border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-left-radius:0px;border-bottom-right-"
            "radius:4px;outline:none;background:rgba(227, 235, 239, 1)}"
            "QListView::item{margin:2 0px 0 0;background:rgba(255, 255, 255, 0.5);border-radius:4px;}"
            "QListView::item:selected{margin:0 0px 0 0;border:2px solid rgba(13, 135, 255, "
            "0.86);background:rgba(255, "
            "255, 255, "
            "0.9);border-radius:4px;}"
            "QListView::item:hover{background:rgba(255, 255, 255, 0.9);border-radius:4px;}");
        m_titlebar->g_menu->setThemeLight();
        m_showImageWidget->g_next->setIcon(QIcon(":/res/res/right.svg"));
        m_showImageWidget->g_back->setIcon(QIcon(":/res/res/left.svg"));
    }
    //    m_sideBar->setSideBarQss(themeStyle);
    Interaction::getInstance()->changeOpenIcon(themeStyle);
    m_toolbar->changeStyle();
    m_markWid->changeStyle();
    m_showImageWidget->cutBtnWidChange(themeStyle);
    m_brushSettings->setTextSettings();
}
void KyView::lable2SetFontSizeSlot(double size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = DEFAULT_FONT_SIZE;
    double nowFontSize = lable2BaseFontSize * size / DEFAULT_FONT_SIZE; // 11为系统默认大小
    m_detio = nowFontSize / lable2BaseFontSize;
    Q_EMIT changeInforSize(nowFontSize, m_detio, m_inforSize);
    inforChange();
}
void KyView::initFontSize()
{
    //只有非标准字号的控件才需要绑定
    connect(this, &KyView::setFontSizeSignal, this, &KyView::lable2SetFontSizeSlot);
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemFontSizeChange, this, [=]() {
        //获取字号的值
        double fontSizeKey = kdk::kabase::Gsettings::getSystemFontSize().toDouble();
        //发送改变信号
        if (fontSizeKey > 0) {
            if (fontSizeKey > 15) {
                fontSizeKey = 15;
            }
            Q_EMIT setFontSizeSignal(fontSizeKey);
        }
    });
    //启动时设置字号
    double fontSizeKey = DEFAULT_FONT; //系统默认字号
    double fontSizeKeyTmp = kdk::kabase::Gsettings::getSystemFontSize().toDouble();
    if (fontSizeKeyTmp > 0) {
        fontSizeKey = fontSizeKeyTmp;
    }
    if (fontSizeKey > 15) {
        fontSizeKey = 15;
    }
    Q_EMIT setFontSizeSignal(fontSizeKey);

    //系统字体类型改变
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        systemGSettings = new QGSettings("org.lingmo.style");
        QObject::connect(systemGSettings, &QGSettings::changed, [=](const QString &key){
            if (key == "systemFont") {
                double systemFontSizeKey = systemGSettings->get("systemFontSize").toDouble();
                Q_EMIT setFontSizeSignal(systemFontSizeKey);
            }
        });
    }
}
//全屏
void KyView::fullScreen()
{
    if (this->isFullScreen()) {
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            this->showNormal();
        } else {
            KWindowSystem::clearState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = false;

    } else {
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            if(this->isMaximized() ){
                m_isMaxAndFull = true;
            }
            this->showFullScreen();
        } else {
            KWindowSystem::setState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = true;
    }
    m_titlebar->dealEditState();
}

void KyView::transChange()
{
    m_tran = kdk::kabase::Gsettings::getSystemTransparency().toDouble();
    this->update();
}

void KyView::avoidChange()
{
    //暂时用此方法解决鼠标在子上释放事件不被检测的bug，以后如有更好的办法再更改。
    //解决点击导航器，触发图片切换的问题。
    if (m_navigator != nullptr) {
        if (!m_navigator->isHidden() && m_navigator->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        } /*else { //此部分会导致打开相册后就将m_panTriggered设为true，导致->左右滑动事件点击即触发
            if (m_navigator->isHidden())
            m_panTriggered = true;
        }*/
    }
    //解决点击侧栏，触发图片切换的问题。
    if (m_sideBar != nullptr) {
        if (!m_sideBar->isHidden() && m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
    //解决点击左右按钮，触发图片切换的问题。
    if (m_showImageWidget != nullptr) {
        if (!m_showImageWidget->g_back->isHidden()
            && (m_showImageWidget->g_back->geometry().contains(this->mapFromGlobal(QCursor::pos()))
                || m_showImageWidget->g_next->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            m_panTriggered = false;
        }
    }
    //解决点击顶栏，触发图片切换的问题。
    if (m_titlebar != nullptr) {
        if (!m_titlebar->isHidden() && m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
    //解决点击工具栏，触发图片切换的问题。
    if (m_toolbar != nullptr) {
        if (!m_toolbar->isHidden() && m_toolbar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
}

void KyView::markToolChange()
{
    if (m_markWid != nullptr) {
        if (!m_markWid->isHidden()) {
            m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
        }
    }
}

//最大化和还原
void KyView::changOrigSize()
{
    if (this->isFullScreen() || m_nowFullScreen) {  //全屏恢复
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            this->showNormal();
        } else {
            KWindowSystem::clearState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = false;

    } else if (this->isMaximized() || m_isMaxAndFull) { //最大化恢复
        if(m_isMaxAndFull && QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
            m_isMaxAndFull = false;
            kdk::WindowManager::setGeometry(this->windowHandle(),m_origRect);
        }else{
            this->showNormal();
        }

    } else {    //最大化
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            m_origRect = kdk::WindowManager::windowGeometry(m_windowId);
            kdk::WindowManager::maximizeWindow(m_windowId);

        }else{
            this->showMaximized();
        }
    }
    m_titlebar->dealEditState();
}

//显示图片--主界面的界面显示
void KyView::toShowImage()
{
    this->m_openImage->hide();
    this->m_showImageWidget->show();
    if (!m_titlebar->isHidden()) {
        this->m_showImageWidget->lower();
    }
    inforChange();
    if (m_timeNomove->isActive()) {
        m_timeNomove->stop();
        return;
    }
    if (!m_clickController) {
        m_timeNomove->start(2500);
    }
}
//显示相册
void KyView::showSidebar()
{
    if (m_sideBar == nullptr) {
        return;
    }
    if (m_sideBar->isHidden()) {
        m_sideBar->show();
        Variable::setSettings("sidebarState", 1);
        Q_EMIT albumState(true);
        this->toolBarPostionSet();
        albumChange();
        m_albumState = false;
        if (m_timerSidebar->isActive()) {
            m_timerSidebar->stop();
        }
    } else {
        m_sideBar->hide();
        Variable::setSettings("sidebarState", 2);
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        Q_EMIT albumState(false);
        m_albumState = true;
        this->toolBarPostionSet();
    }
}
//先保留--默认打开相册
// void KyView::defaultSidebar()
//{
//    m_sideBar->show();
//    Q_EMIT albumState(true);
//    albumChange();
//    m_albumState = false;
//}
//检测鼠标位置--顶栏和工具栏的出现和隐藏
void KyView::mouseMoveEvent(QMouseEvent *event)
{
    //手指长按时，过滤2像素差的鼠标移动事件
    if (qAbs(event->pos().x() - m_pressPosRecord.x()) < 2 && qAbs(event->pos().y() - m_pressPosRecord.y()) < 2) {
        return;
    }
    //    if (Variable::g_mainlineVersion) {
    if (displayMode == DisplayMode::NormalMode) {
        //鼠标移入特定区域显示/隐藏控件
        if (m_openImage->isHidden()) {
            int y = this->mapFromGlobal(QCursor().pos()).y();
            hoverChange(y);
        } else {
            this->toolBarHideOrShow(false);
            m_titlebar->show();
        }
    } else if (displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
        this->toolBarHideOrShow(false);
        m_titlebar->show();
    }
    //    }
    //窗口拖拽
    if (m_titlebar == nullptr) {
        return;
    }
    // 去除光标位置判断，修复wayland下鼠标拖动卡顿问题，新版本中用QML进行编写看图界面，打开图片时光标拖动不影响窗口.
    if (m_titlebar->isHidden()){ // || !m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (!m_mousePress) {
        return;
    }
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }
    QPoint pos = this->mapToGlobal(event->pos());

    //处理触摸屏-拖动后，触摸释放信号不被发出问题-如果是触摸
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果没有正在捕获鼠标
        if (!this->mouseGrabber()) {
            //抓取鼠标
            this->grabMouse();
            //进行释放--避免是触摸的时候，事件一直无法被释放，无法进行后续拖拽点击等事件
            this->releaseMouse();
        }
    }
    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        XEvent xEvent;
        memset(&xEvent, 0, sizeof(XEvent));

        Display *display = QX11Info::display();
        xEvent.xclient.type = ClientMessage;
        xEvent.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
        xEvent.xclient.display = display;
        xEvent.xclient.window = (XID)(this->winId());
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = pos.x() * device;
        xEvent.xclient.data.l[1] = pos.y() * device;
        xEvent.xclient.data.l[2] = 8;
        xEvent.xclient.data.l[3] = Button1;
        xEvent.xclient.data.l[4] = 1;

        XUngrabPointer(display, CurrentTime);
        XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()), False,
                   SubstructureNotifyMask | SubstructureRedirectMask, &xEvent);
        XFlush(display);
    } else {
        hLayoutFlag = hOrVMode->defaultModeCapture();
        if (hLayoutFlag != deviceMode::PCMode) {
            return;
        }else{
            this->windowHandle()->startSystemMove();
            leftPoint = leftPoint + (event->globalPos() - m_dragPos);
            //kdk::WindowManager::setGeometry(this->windowHandle(),QRect(leftPoint, QSize(this->width(), this->height())));
        }
	}
	

    event->accept();
}
void KyView::mousePressEvent(QMouseEvent *event)
{
    //只能是鼠标左键移动和改变大小
    if (event->button() == Qt::LeftButton) {
        m_mousePress = true;
        this->m_dragPos = event->globalPos();
    }
    m_pressPosRecord = event->pos();
}

void KyView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_mousePress = false;
    m_pressPosRecord = QPoint(0, 0);
}

//拖拽主窗口--各控件需要改变位置或尺寸
void KyView::resizeEvent(QResizeEvent *event)
{
    m_recordMarkOldPos = m_markWid->pos();
    titlebarChange();
    openImageChange();
    showImageChange();
    toolbarChange();
    naviChange();
    inforChange();
    albumChange();
    markChange();
    brushSettingsChange();
    if (m_isReplacing) {
        mattingChange();
    }
    if (m_isScanning) {
        scannerChange();
    }
    if (this->isMaximized() || this->isFullScreen() || m_nowFullScreen || m_isMaxAndFull) {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的全屏图标
        m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
    } else {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的恢复图标
        m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
    }
    if(displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
        return;
    }
    Variable::g_widSize = this->size();
    Interaction::getInstance()->changeWidgetSize(Variable::g_widSize);
}
//鼠标离开事件
void KyView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_titleState = false;
    //点出顶栏的下拉菜单时，不隐藏
    if (!m_titlebar->isHidden() && !m_titlebar->g_menu->m_menu->isHidden() || displayMode == DisplayMode::OCRMode) {
        return;
    }
    if (m_openImage->isHidden()) {
        if (!m_timernavi->isActive()) {
            if (!m_clickController) {
                m_timernavi->start(2500);
            }
        }
        m_showImageWidget->g_next->hide();
        m_showImageWidget->g_back->hide();
    }

    inforChange();
}
//鼠标进入事件
void KyView::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_titleState = true;
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    if (m_sideBar->isHidden()) {
        return;
    }
    if (m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        m_showImageWidget->g_back->show();
        m_showImageWidget->g_next->show();
    }
    if ((m_timer->remainingTime() < 2000 && m_timer->isActive()) || !(m_timer->isActive())) {
        m_timer->stop();
        if (!m_clickController) {
            m_timer->start(2500);
        }
    }
}

void KyView::paintEvent(QPaintEvent *event)
{
    //使用主题给的代码适配毛玻璃效果
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QColor color = qApp->palette().color(QPalette::Base);
    color.setAlphaF(m_tran);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    m_showImageWidget->changeQmlcomponentColor(color, m_tran);
    if (m_isReplacing) {
        m_showImageWidget->m_replaceBackgroundWid->changeQmlColor(color);
    }
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void KyView::keyReleaseEvent(QKeyEvent *event)
{
    if (displayMode == DisplayMode::NormalMode && m_openImage->isHidden()) {
        switch (event->key()){
        case Qt::Key_Control:
            m_keyState = "keyNone";
            break;
        case Qt::Key_Alt:
            m_keyState = "keyNone";
            break;
        default:
            break;
        }
    }

}
// F1快捷键打开用户手册
void KyView::keyPressEvent(QKeyEvent *event)
{
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        kdk::kabase::UserManualManagement userManualTest;
        if (!userManualTest.callUserManual("pictures")) {
            qCritical() << "user manual call fail!";
        }
    }
    //全屏
    if (displayMode == DisplayMode::NormalMode) {
        if (event->key() == Qt::Key_F11) {
            this->fullScreen();
        }
        if (event->key() == Qt::Key_Escape) {
            if (this->isFullScreen() || m_nowFullScreen) {
                this->fullScreen();
            }
        }
    }

    // 新增快捷键功能
    if (displayMode == DisplayMode::NormalMode && m_openImage->isHidden()) {
        switch (event->key()){
        case Qt::Key_Control:
            m_keyState = "Ctrl";
            break;
        case Qt::Key_Alt:
            m_keyState = "Alt";
            break;
        case Qt::Key_Return:
            if (m_keyState == "Alt"){
                if (m_information->isHidden()){
                    m_information->show();
                    inforChange();
                }
                else{
                    m_information->hide();
                }
            }
            break;
        case Qt::Key_Plus:
            if (m_keyState == "Ctrl")
                m_toolbar->enlargeImageSlot();
            break;
        case Qt::Key_Equal:
            if (m_keyState == "Ctrl")
                m_toolbar->enlargeImageSlot();
            break;
        case Qt::Key_Minus:
            if (m_keyState == "Ctrl")
                m_toolbar->reduceImageSlot();
            break;
        case Qt::Key_0:
            if (m_keyState == "Ctrl")
               m_toolbar->adaptiveWidget();
            break;
        case Qt::Key_R:
            if (m_keyState == "Ctrl")
                m_toolbar->rotate();
            break;
        case Qt::Key_P:
            if (m_keyState == "Ctrl")
                m_showImageWidget->initPrint();
            break;
        case Qt::Key_O:
            Interaction::getInstance()->openImageInfile();
            break;
        default:
                break;
        }
    }

    //上一张，下一张，delete按键响应
    if (!m_openImage->isHidden()) {
        return;
    } else {
        if (displayMode == DisplayMode::NormalMode) {
            if (event->key() == Qt::Key_Delete) {
                m_toolbar->delImage();
            }
            if (event->key() == Qt::Key_Left) {
                m_showImageWidget->m_canSet = true;
                m_showImageWidget->backImage();
            }
            if (event->key() == Qt::Key_Right) {
                m_showImageWidget->m_canSet = true;
                m_showImageWidget->nextImage();
            }
            if (event->matches(QKeySequence::Copy)) {
                m_showImageWidget->copy();
            }
        }
    }
    //裁剪
    if (displayMode != DisplayMode::CuttingMode) {
        return;
    } else {
        if (event->key() == Qt::Key_Escape) {
            //退出裁剪模式--弹出保存提示框
            m_showImageWidget->exitCut();
            m_showImageWidget->cutFinishUi();
        }
        if (event->key() == 16777220 || event->key() == Qt::Key_Enter) {
            //保存并退出裁剪模式
            if (m_showImageWidget->btnWidState()) {
                m_showImageWidget->needSave();
            }
        }
    }
}
//添加左键双击事件--最大化和还原
void KyView::mouseDoubleClickEvent(QMouseEvent *event)
{
    //判断左键双击
    if (event->button() != Qt::LeftButton) {
        return;
    }
    this->dealMouseDouble();
}

//拖文件进界面
void KyView::dragEnterEvent(QDragEnterEvent *event)
{
    if (operayteMode == OperayteMode::Cut || operayteMode == OperayteMode::Ocr || operayteMode == OperayteMode::Sign) {
        return;
    }
    //获得所有可被查看的图片格式
    QStringList formatList;
    QString format;
    format = "";
    for (const QString &str : Variable::SUPPORT_FORMATS) {
        format = str;
        formatList.append(format);
    }
    //检查当前拖入的否为url，暂时只支持以url形式拖入
    if (!event->mimeData()->hasUrls()) {
        return;
    }
    //极端情况下，拖拽文件时list有可能为空，不做异常处理时，会闪退。
    QList<QUrl> list = event->mimeData()->urls();
    if (list.isEmpty()) {
        return;
    }
    QUrl url = list.first();
    if (url.isEmpty()) {
        return;
    }
    QString str = url.fileName();
    if (str.isEmpty()) {
        return;
    }
    //判断图片是否支持被查看
    //    if (formatList.contains(QFileInfo(str).suffix().toLower())) {
    event->acceptProposedAction();
    //    } else {
    //    event->ignore(); //否则不接受鼠标事件
    //    }
}
//放下文件之后响应
void KyView::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    //判断是否为空
    if (urls.isEmpty()) {
        return;
    }
    //拿路径，发信号打开图片
    QString path = urls.first().toLocalFile();
    if (path == "") {
        return;
    }
    QFileInfo info(path);
    Variable::setSettings("imagePath", info.absolutePath());
    m_showImageWidget->openImage(path);
}

void KyView::dealMouseDouble()
{
    //在标题栏的图片名字处双击不响应全屏或恢复
    if (m_titlebar->g_imageName->geometry().contains(this->mapFromGlobal(QCursor::pos())) && m_isLeave == true) {
        //有问题--后续需要更改
        if (m_titlebar->g_imageName->x() != 0) {
            return;
        }
    }
    if (displayMode != DisplayMode::NormalMode) {
        return;
    }
    if (!m_isResponseDbClick) {
        return;
    }
    if (!m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))
        && !m_toolbar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        //在信息栏处双击不响应全屏
        if (!m_information->isHidden() && (m_information->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            return;
        }
        //在导航栏处双击不响应全屏
        if (!m_navigator->isHidden() && (m_navigator->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            return;
        }
        //图片区域双击全屏
        this->fullScreen();

    } else if (m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        //标题栏处双击最大化/还原
        if(this->isFullScreen() || m_nowFullScreen){
            return;
        }else{
            this->changOrigSize();
        }
    }
}

void KyView::initGrabGesture()
{
    //手势需要关闭主题事件
    this->setProperty("useStyleWindowManager", false);
}

bool KyView::event(QEvent *event)
{
    //处理失焦时，毛玻璃特效失效问题
    if (event->type() == QEvent::ActivationChange) {
        this->update();
    }
    //判断列表中是否只有一张图，一张图片左右按钮不显示--解决静止鼠标，按钮仍然显示的问题
    if (m_openImage != nullptr && m_openImage->isHidden()) {
        if (m_showImageWidget != nullptr) {
            if (m_showImageWidget->g_buttonState == false) {
                this->m_showImageWidget->g_next->hide();
                this->m_showImageWidget->g_back->hide();
            }
        }
    }
    //解决点击下拉菜单出现，鼠标移出窗体，点击窗体外部，两秒后两栏未隐藏的问题。
    if (m_titleState == false && displayMode != DisplayMode::OCRMode) {
        if (m_titlebar->g_menu->m_menu->isHidden() && !m_titlebar->isHidden()) {
            if (!m_timeNomove->isActive()) {
                if (!m_clickController) {
                    m_timeNomove->start(2500);
                }
            }
        }
    }
    avoidChange();
    //使用X11接管窗口移动事件后，鼠标离开事件不触发，此处作为备用方案
    if (m_mousePress) {
        if (event->type() == QEvent::Move) {
            //本次不响应切换手势
            m_panTriggered = false;

            x11EventEnd();
        }
    }
    return QWidget::event(event);
}

bool KyView::eventFilter(QObject *obj, QEvent *event){
    if(obj == m_information)
    {
        if (!m_information->isHidden() && m_information->geometry().contains(this->mapFromGlobal(QCursor::pos()))){
            if (m_openImage->isHidden()) {
                int y = m_information->mapFromGlobal(QCursor().pos()).y();
                hoverChange(y);
            } else {
                m_toolbar->hide();
                m_titlebar->show();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void KyView::x11EventEnd()
{
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }
    QPoint globalPoints = QCursor::pos();

    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        XEvent xEvent;
        memset(&xEvent, 0, sizeof(XEvent));
        Display *display = QX11Info::display();
        xEvent.type = ButtonRelease;
        xEvent.xbutton.button = Button1;
        xEvent.xbutton.window = this->effectiveWinId();
        if (globalPoints.y() < 5) {
            xEvent.xbutton.x = globalPoints.x() * device;
            xEvent.xbutton.y = globalPoints.y() * device;
        } else {
            xEvent.xbutton.x = this->mapFromGlobal(globalPoints).x() * device;
            xEvent.xbutton.y = this->mapFromGlobal(globalPoints).y() * device;
        }
        xEvent.xbutton.x_root = globalPoints.x() * device;
        xEvent.xbutton.y_root = globalPoints.y() * device;
        xEvent.xbutton.display = display;

        //在release之前，先模拟一次press操作，防止release不生效
        XEvent xEvent2 = xEvent;
        xEvent2.type = ButtonPress;
        XSendEvent(display, this->effectiveWinId(), False, ButtonPressMask, &xEvent2);
        XFlush(display);

        XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xEvent);
        XFlush(display);
        m_mousePress = false;
    }
}

// 手势识别
bool KyView::gestureEvent(QEvent *event)
{
    QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
    if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture)) {
        pinchTriggered(static_cast<QPinchGesture *>(gesture));
        return true;
    }
    if (QGesture *gesture = gestureEvent->gesture(Qt::TapAndHoldGesture)) {
        tapAndHoldGesture(static_cast<QTapAndHoldGesture *>(gesture));
        return true;
    }
    return false;
}

void KyView::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags(); // 获取已改变的手势属性

    if (gesture->centerPoint() != QPointF(-1, -1)) {
        return;
    }
    if (changeFlags & QPinchGesture::RotationAngleChanged) { // 手势覆盖角度改变
        if (!m_isResize) {
            const qreal lastAngle = gesture->lastRotationAngle();
            if (touchRotate(lastAngle)) {
                m_isRotate = true;
                m_touchPoint.setX(-1); //特殊标记
            }
        }
    }

    if (changeFlags & QPinchGesture::ScaleFactorChanged) { // 两个输入点的距离改变
        if (!m_isRotate) {
            qreal scaleFactor = gesture->totalScaleFactor();
            qreal tmpNum = scaleFactor - m_lastDistance;
            QPoint zoomFocus = this->mapFromGlobal(gesture->centerPoint().toPoint());
            //避免操作太频繁，移动幅度超过某值才做处理
            if (tmpNum > m_minDistance) {
                //                Interaction::getInstance()->watchBigImage(zoomFocus);
                m_lastDistance = scaleFactor;
            } else if (tmpNum < m_minDistance * -0.5) {
                //                Interaction::getInstance()->watchSmallImage(zoomFocus);
                m_lastDistance = scaleFactor;
            }
            if (m_lastDistance != 1) {
                m_isResize = true;
                m_touchPoint.setX(-1); //特殊标记
                if (recordCurrentProportion < 150) {
                    m_minDistance = 0.4;
                } else if (recordCurrentProportion < 300) {
                    m_minDistance = 0.3;
                } else if (recordCurrentProportion < 500) {
                    m_minDistance = 0.2;
                } else {
                    m_minDistance = 0.1;
                }
            }
        }
    }

    if (gesture->state() == Qt::GestureFinished) { // 手势结束
        m_rotateLevel = 0;
        m_isRotate = false;
        m_isResize = false;
        m_lastDistance = 1;
        m_minDistance = 0.5;
        m_needCLoseGrabGesture = false;
    }
}

bool KyView::touchRotate(const qreal &lastAngle)
{
    bool result = false;
    // 最后的手势覆盖角度介于-45度和45度之间
    if (lastAngle > -45 && lastAngle < 45 && lastAngle != 0) {
        if (m_rotateLevel == -1) {
            //顺时针转到此区域
            Interaction::getInstance()->rotate();
            result = true;
        } else if (m_rotateLevel == 1) {
            //逆时针转到此区域
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        //记录区域标签
        m_rotateLevel = 0;
        return result;
    }

    if (lastAngle > 45 && lastAngle < 135) {
        if (m_rotateLevel == 0) {
            Interaction::getInstance()->rotate();
            result = true;
        } else if (m_rotateLevel == 2) {
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        m_rotateLevel = 1;
        return result;
    }
    if (lastAngle > 135 && lastAngle < 180) {
        if (m_rotateLevel == 1) {
            Interaction::getInstance()->rotate();
            result = true;
        }
        m_rotateLevel = 2;
        return result;
    }

    if (lastAngle < -45 && lastAngle > -135) {
        if (m_rotateLevel == 0) {
            Interaction::getInstance()->rotate(false);
            result = true;
        } else if (m_rotateLevel == -2) {
            Interaction::getInstance()->rotate();
            result = true;
        }
        m_rotateLevel = -1;
        return result;
    }
    if (lastAngle < -90 && lastAngle > -180) {
        if (m_rotateLevel == -1) {
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        m_rotateLevel = -2;
        return result;
    }
    return result;
}

void KyView::setWindowSize()
{
    if (hLayoutFlag == PADHorizontalMode || hLayoutFlag == PADVerticalMode) {
        m_isResponseDbClick = false;
        Variable::platForm = "pc";
        toolbarHeightValue = TOOLBAR_HEIGHT_INTEL;
        titlebarHeightValue = BAR_HEIGHT_INTEL;
        m_miniSize = MINI_SIZE_INTEL;
        QScreen *screen = QGuiApplication::primaryScreen();
        this->resize(screen->geometry().width(), screen->geometry().height());
    } else {
        m_isResponseDbClick = true;
        Variable::platForm = "normal";
        toolbarHeightValue = BAR_HEIGHT;
        titlebarHeightValue = BAR_HEIGHT;
        m_miniSize = MINI_SIZE;
        this->resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    this->setTitleBtnHide();
}

void KyView::setTitleBtnHide()
{
    //处理标题栏，工具栏，标注栏的显示
    if (m_titlebar != nullptr) {
        m_titlebar->setBtnShowOrHide();
    }
    if (m_toolbar != nullptr) {
        m_toolbar->platformType();
        m_toolbar->initControlQss();
    }
    if (m_markWid != nullptr) {
        m_markWid->resetMarkWidQss();
    }
    if (m_brushSettings != nullptr) {
        if (!m_brushSettings->isHidden()) {
            if (m_markWid != nullptr) {
                m_brushSettings->setSize(m_markWid->getTextState());
                m_brushSettings->move(m_brushSettings->x(), m_markWid->y() - 6 - m_brushSettings->height());
            }
        }
    }
}

QString KyView::getAppVersion()
{
    FILE *file = NULL;
    char *line = NULL;
    size_t size = 0;
    ssize_t read;
    char *q = NULL;
    QString version = nullptr;
    file = popen("dpkg -l lingmo-photo-viewer", "r");
    if(NULL == file)
        return version;
    while((read = getline(&line, &size, file)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';
        QString content = line;
        QStringList list = content.split(" ");
        list.removeAll("");
        if (list.size() >= 3)
            version = list.at(2);
    }
    free(line);
    pclose(file);
    return version;
}

void KyView::initAboutDialog()
{
    m_aboutDialog->setAppIcon(QIcon::fromTheme("lingmo-photo-viewer"));
    m_aboutDialog->setAppName(tr("Pictures"));
    if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
        m_aboutDialog->setAppVersion(getAppVersion()+tr(":Version"));
    } else {
        m_aboutDialog->setAppVersion(tr("Version:")+getAppVersion());
    }
    m_aboutDialog->setBodyText(tr("A system picture tool that can quickly open common formats. It provides zoom,flip "
                                  "and other processing simplely."));
}

void KyView::toolBarPostionSet()
{
    if (Variable::g_mainlineVersion) {
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
        return;
    }
    if (m_showImageWidget != nullptr) {
        if (!m_toolbar->isHidden()) {
            double tempPoint = m_toolbar->y() - 20;
            m_showImageWidget->setTiffToolbarPos(tempPoint);
        }
    }
}

void KyView::toolBarHideOrShow(bool needShow)
{
    if (m_showImageWidget == nullptr) {
        return;
    }
    double tempPoint;
    if (needShow) {
        m_toolbar->raise();
        m_toolbar->show();
        tempPoint = m_toolbar->y() - 20;
        m_showImageWidget->setTiffToolbarPos(tempPoint);
    } else {
        m_toolbar->hide();
        tempPoint = this->height() - 20;
        m_showImageWidget->setTiffToolbarPos(tempPoint);
    }
}

void KyView::restoreAllBallState()
{
    if (m_recordSidebarStateBeforeOtherOperate) {
        m_sideBar->show();
        this->albumPosChange();
        this->toolBarPostionSet();
    }
    if (m_recordInfoStateBeforeOtherOperate) {
        m_information->show();
        this->inforChange();
    }
}

void KyView::tapAndHoldGesture(QTapAndHoldGesture *gesture)
{
    if (m_panTriggered) {
        return;
    }
    m_touchPoint = gesture->position();
    if (!m_openImage->isHidden()) {
        return;
    }
    m_panTriggered = true;
}

void KyView::changeEvent(QEvent *event)
{
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = windowState();

        if (state == Qt::WindowNoState) {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的全屏图标
            m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
            if (!m_isPcModeStarted) {
                m_isPcModeStarted = true;
                resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
                int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
                leftPoint = QPoint((sw - this->width()) / 2, (sh - this->height()) / 2);
                qDebug() << "= "<< __FUNCTION__<<" "<<__LINE__ << this->geometry() << " leftPoint = " << leftPoint;
                kdk::WindowManager::setGeometry(this->windowHandle(), QRect(leftPoint, QSize(this->width(), this->height())));
             } else {
                QScreen *screen = QGuiApplication::primaryScreen();
                this->move((screen->geometry().width() - this->width()) / 2,
                           (screen->geometry().height() - this->height()) / 2);
            }

            }
        } else {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
            m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
        }
    }
    if (m_nowFullScreen) {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
        m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
    } else {
        if (this->isMaximized() || m_isMaxAndFull) {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
            m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
        } else {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的全屏图标
            m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
        }
    }
    return;
}

void KyView::closeEvent(QCloseEvent *event)
{
    if (m_showImageWidget && m_showImageWidget->m_replaceBackgroundWid)
    {
        m_showImageWidget->m_replaceBackgroundWid->closeColorDialog();
    }

    QWidget::closeEvent(event);
}
