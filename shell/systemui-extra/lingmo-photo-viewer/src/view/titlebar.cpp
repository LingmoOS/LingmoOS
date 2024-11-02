#include "titlebar.h"
#include "kyview.h"
#include "sizedate.h"
TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    //    this->setWindowOpacity(0.7);
    //布局
    g_titleWid = new QWidget(this);
    m_titleLayout = new QHBoxLayout(this);
    //左上角图标
    m_logoBtn = new QPushButton(this);
    m_logoBtn->setFixedSize(LOGO_BTN); //重置图标大小
    m_logoBtn->setIcon(QIcon::fromTheme("lingmo-photo-viewer", QIcon(":/res/res/kyview_logo.png")));
    m_logoBtn->setIconSize(LOGO_BTN);
    m_logoBtn->setFlat(true);
    m_logoBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_backButton = new QPushButton();
    m_backButton->setFixedSize(LOGO_BTN);
    //    m_backButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setIcon(QIcon::fromTheme("lingmo-start-symbolic"));
    //左上角APP名字
    m_logolb = new QLabel(this);
    m_logolb->setAttribute(Qt::WA_TranslucentBackground);
    //    m_logolb->setFont(m_ft);
    //中间图片名字
    g_imageName = new QLabel(this);
    //    g_imageName->setFont(m_ft);
    g_imageName->setAttribute(Qt::WA_TranslucentBackground);
    g_imageName->hide();
    g_myEdit = new Edit(this);
    g_myEdit->setMaxLength(83);
    g_myEdit->hide();
    //窗口四联--菜单
    g_menu = new menuModule(this);
    g_menu->setFocusPolicy(Qt::NoFocus);
    g_menu->setAttribute(Qt::WA_TranslucentBackground);
    //窗口四联--最小化按钮
    m_minibtn = new QPushButton(this);
    m_minibtn->setToolTip(tr("minimize"));
    m_minibtn->setFocusPolicy(Qt::NoFocus);                           //设置焦点类型
    m_minibtn->setIcon(QIcon::fromTheme("window-minimize-symbolic")); //主题库的最小化图标
    m_minibtn->setProperty("isWindowButton", 0x1);
    m_minibtn->setProperty("useIconHighlightEffect", 0x2);
    m_minibtn->setFlat(true);
    //窗口四联--最大化/还原按钮
    g_fullscreen = new QPushButton(this);
    g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的最小化图标

    g_fullscreen->setToolTip(tr("full screen"));
    g_fullscreen->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    g_fullscreen->setProperty("isWindowButton", 0x1);
    g_fullscreen->setProperty("useIconHighlightEffect", 0x2);
    g_fullscreen->setFlat(true);
    //窗口四联--关闭按钮
    m_closebtn = new QPushButton(this);
    m_closebtn->setToolTip(tr("close"));
    m_closebtn->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标

    m_closebtn->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    m_closebtn->setProperty("isWindowButton", 0x2);
    m_closebtn->setProperty("useIconHighlightEffect", 0x8);
    m_closebtn->setFlat(true);

    this->platformType();
    this->initConnect();
    this->initControlQss();
    this->initFontSize();
}

TitleBar::~TitleBar()
{
    if(m_backButton) {
        delete m_backButton;
        m_backButton = nullptr;
    }
}
//顶栏图片名字
void TitleBar::showImageName(QString name, QString imagePath)
{
    if (name == "") {
        return;
    }
    if (m_oldPath == "") {
        m_oldPath = imagePath;
    } else {
        //如果重命名框正在显示，且图片路径一致，则说明非切换图片
        if (!g_myEdit->isHidden()) {
            if (m_oldPath == imagePath) {
                return;
            } else {
                g_myEdit->hide();
            }
        }
    }
    m_oldPath = imagePath;
    QFileInfo imageName(imagePath);
    m_oldName = name;
    m_imagePath = imagePath;
    g_myEdit->setText(imageName.completeBaseName());
    m_origName = imageName.completeBaseName();
    longText(g_imageName, name);
    g_imageName->show();
}
//重命名
void TitleBar::needRename(int mode)
{
    if (!g_myEdit->isHidden()) {
        return;
    }
    if (g_imageName->isHidden()) {
        return;
    }
    if (!m_renameEnable) {
        return;
    }
    if (0 == mode) {
        if (!g_imageName->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            return;
        }
    }
    editSelected();
}

void TitleBar::setRenameEnable(bool isOpenSucess, bool isCanOpertate)
{
    m_renameEnable = isOpenSucess && isCanOpertate;
}

void TitleBar::setBtnShowOrHide()
{
    this->platformType();
}

void TitleBar::setLeftUpBtnStyle()
{
    if (EXTERNAL_START_APP.contains(Variable::startAppName)) {
        m_logolb->setText(tr("Return"));
        m_backButton->show();
        m_logoBtn->hide();
    } else {
        m_logolb->setText(tr("Pictures"));
        m_logoBtn->show();
        m_backButton->hide();
    }
}
//名字过长显示...
void TitleBar::longText(QLabel *nameC, QString text)
{
    QFontMetrics fontWidth(nameC->font());                                             //得到每个字符的宽度
    QString elideNote = fontWidth.elidedText(text, Qt::ElideRight, this->width() / 2); //最大宽度wid像素
    nameC->setText(elideNote);                                                         //显示省略好的字符串
    //解决tooltips设置一次后一直显示的问题
    if (elideNote.contains("…", Qt::CaseInsensitive)) {
        nameC->setToolTip(text); //设置tooltips
    } else {
        nameC->setToolTip(""); //设置tooltips
    }
}
//初始化顶栏布局
void TitleBar::initControlQss()
{
    if (platForm.contains(Variable::platForm)) {
        m_titleLayout->setSpacing(0);
        m_titleLayout->setMargin(0);
        m_titleLayout->addSpacing(4);
        m_titleLayout->addWidget(m_backButton); // logo
        m_titleLayout->addWidget(m_logoBtn);    // logo
        m_titleLayout->addSpacing(4);
        m_titleLayout->addWidget(m_logolb, 0, Qt::AlignCenter); //标签
        m_titleLayout->addStretch();                            //添加伸缩
        m_titleLayout->addSpacing(40);
        m_titleLayout->addWidget(g_imageName);
        m_titleLayout->addWidget(g_myEdit);
        m_titleLayout->addStretch();                  //添加伸缩
        m_titleLayout->addWidget(g_menu->menuButton); //设置按钮
        m_titleLayout->addWidget(m_minibtn);
        m_titleLayout->addWidget(g_fullscreen);
        m_titleLayout->addWidget(m_closebtn);

    } else {
        m_titleLayout->setSpacing(0);
        m_titleLayout->setMargin(0);
        m_titleLayout->addSpacing(12);
        m_titleLayout->addWidget(m_backButton); // logo
        m_titleLayout->addWidget(m_logoBtn);    // logo
        m_titleLayout->addSpacing(8);
        m_titleLayout->addWidget(m_logolb, 0, Qt::AlignCenter); //标签
        m_titleLayout->addStretch();                            //添加伸缩
        m_titleLayout->addSpacing(40);
        m_titleLayout->addWidget(g_imageName);
        m_titleLayout->addWidget(g_myEdit);
        m_titleLayout->addStretch();                  //添加伸缩
        m_titleLayout->addWidget(g_menu->menuButton); //设置按钮
        m_titleLayout->addSpacing(2);
        m_titleLayout->addWidget(m_minibtn);
        m_titleLayout->addSpacing(2);
        m_titleLayout->addWidget(g_fullscreen);
        m_titleLayout->addSpacing(2);
        m_titleLayout->addWidget(m_closebtn);
        m_titleLayout->addSpacing(4);
    }
    this->setLayout(m_titleLayout);
}
//信号和槽
void TitleBar::initConnect()
{
    //connect(m_minibtn, &QPushButton::clicked, KyView::mutual, &KyView::showMinimized);
    connect(m_minibtn, &QPushButton::clicked, [=](){
        KyView::mutual->setWindowState(Qt::WindowMinimized);
        KyView::mutual->showMinimized();
    });
    connect(g_fullscreen, &QPushButton::clicked, this, &TitleBar::recovery);
    connect(m_closebtn, &QPushButton::clicked, Interaction::getInstance(), &Interaction::close);
    connect(m_closebtn, &QPushButton::clicked, this, &TitleBar::exitApp);
    connect(Interaction::getInstance(), &Interaction::progremExit, KyView::mutual, &KyView::close);
    connect(g_menu, &menuModule::openSignal, this, &TitleBar::openSignal);
    connect(g_menu, &menuModule::aboutShow, this, &TitleBar::aboutShow);
    connect(g_menu, &menuModule::exitAppFromMenu, this, &TitleBar::exitApp);
    connect(g_myEdit, &Edit::renamefinished, this, &TitleBar::reName);
    connect(g_myEdit, &Edit::showOrigName, this, &TitleBar::showOrigName);
    connect(Interaction::getInstance(), &Interaction::sendRenameResult, this, &TitleBar::dealRenameResult);
    connect(g_myEdit, &Edit::dealDoubleClicked, this, &TitleBar::dealDoubleClick);
    connect(m_backButton, &QPushButton::clicked, Interaction::getInstance(), &Interaction::close);
}
void TitleBar::initFontSize()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemFontSizeChange, this, [=]() {
        longText(g_imageName, m_oldName);
    });

    longText(g_imageName, m_oldName);
}
void TitleBar::platformType()
{
    if (platForm.contains(Variable::platForm)) {
        m_closebtn->setFixedSize(TITLE_BTN_INTEL);
        m_minibtn->setFixedSize(TITLE_BTN_INTEL);
        g_fullscreen->setFixedSize(TITLE_BTN_INTEL);
        g_menu->setMenuBtnSize(MBTN_SIZE_INTEL);
        g_fullscreen->hide();
        this->setFixedHeight(BAR_HEIGHT_INTEL);
    } else {
        m_closebtn->setFixedSize(TITLE_BTN);
        m_minibtn->setFixedSize(TITLE_BTN);
        g_fullscreen->setFixedSize(TITLE_BTN);
        g_menu->setMenuBtnSize(MBTN_SIZE);
        g_fullscreen->show();
        this->setFixedHeight(BAR_HEIGHT);
    }
}

void TitleBar::dealEditState()
{
    if (!g_myEdit->isHidden()) {
        g_myEdit->hide();
    }
}
//双击改名
void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (g_myEdit->isHidden()) {
        needRename(0);
    } else {
        g_myEdit->hide();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_backButton != nullptr && !m_backButton->isHidden()) {
        if (m_logolb->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            Interaction::getInstance()->close();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::dealDoubleClick(bool isleaveOrNot)
{
    Q_EMIT doubleClick(isleaveOrNot);
}
//告诉后端重命名
void TitleBar::reName()
{
    //判断新老名字是否一样，无需进行重命名。（且一样的情况下，qfile会返回失败）
    QFileInfo filename(m_imagePath);
    if (filename.completeBaseName() == g_myEdit->Text) {
        g_imageName->show();
        return;
    }
    if (g_myEdit->Text.indexOf(".") == 0) {
        //此文件将被隐藏（文件名以“.”开头的将会成为隐藏属性文件）
        m_msg->warning(
            this, tr("Warning"),
            tr("This file will be hidden(the file whose name begins with \".\" will be the hidden property file.)"));
    } else {
        for (int i = 0; i < SPECIALCHAR.length(); i++) {
            if (g_myEdit->Text.contains(SPECIALCHAR.at(i))) {
                //含非法文件名
                m_msg->warning(this, tr("Warning"), tr("the file name is illegal"));
                g_myEdit->setText(m_origName);
                //                g_imageName->show();
                return;
            }
        }
    }
    Interaction::getInstance()->reName(m_imagePath, g_myEdit->Text);
}
//
void TitleBar::dealRenameResult(int code, QFileInfo newfile)
{
    if (code == 0) {
        //刷新g_imageName显示的形式
        longText(g_imageName, newfile.fileName());
        m_origName = newfile.completeBaseName();
        m_oldName = newfile.fileName();
        g_imageName->show();
        g_myEdit->setText(newfile.completeBaseName());
        Variable::renameLoadedImagePath(m_imagePath, newfile.filePath());
        Q_EMIT updateImageWidName(newfile.absoluteFilePath());
        Q_EMIT updateInformation(newfile);
        Q_EMIT updateFileInfo(newfile);
        m_imagePath = newfile.filePath();
        return;
    } else if (code == -1) {
        //文件不存在(已被删除)！
        m_msg->warning(this, tr("Warning"), tr("File does not exist (or has been deleted)!"));
    } else if (code == -2) {
        //此名称已被占用，请选取其他名称！
        m_msg->warning(this, tr("Warning"), tr("This name has been occupied, please choose another！"));
    } else if (code == -3) {
        //此为只读文件，请修改权限后操作！
        m_msg->warning(this, tr("Warning"),
                       tr("This is a read-only file, please modify the permissions before operation！"));
    } else if (code == -4) {
        //其他错误，重命名失败！
        m_msg->warning(this, tr("Warning"), tr("Other error, rename failed！"));
    }
    g_imageName->show();
    g_myEdit->setText(newfile.completeBaseName());
}

void TitleBar::showOrigName()
{
    g_imageName->show();
    g_myEdit->hide();
    g_myEdit->setText(m_origName);
}
//选中改名
void TitleBar::editSelected()
{
    g_imageName->hide();
    g_myEdit->setParent(this);
    g_myEdit->adjustSize();
    g_myEdit->show();
    g_myEdit->setFocus();
    g_myEdit->selectAll();
}
