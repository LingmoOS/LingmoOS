#include "toolbar.h"
#include "kyview.h"
#include "sizedate.h"
#include "global/interactiveqml.h"
ToolBar::ToolBar(QWidget *parent) : QWidget(parent)
{

    //    this->resize(678 +4 ,40 + 4);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocusPolicy(Qt::NoFocus);
    //布局
    g_tooleWid = new QFrame(this);
    m_toolLayout = new QHBoxLayout(this);
    //缩放widget
    m_zoomWid = new QWidget(this);

    //缩小
    m_reduce = new kdk::KBorderlessButton(m_zoomWid);

    m_reduce->setFocusPolicy(Qt::NoFocus);
    m_reduce->move(0, 0);
    m_reduce->setToolTip(tr("Zoom out"));
    m_reduce->setAttribute(Qt::WA_TranslucentBackground);
    //    m_reduce->setToolTip(tr("缩小图片"));
    //    m_reduce->setAutoRepeat(true);
    //    m_reduce->setAutoRepeatDelay(1000);
    //百分比
    m_percentage = new QLabel(m_zoomWid);
    m_percentage->setFixedSize(TOOL_PER);
    m_percentage->setAlignment(Qt::AlignCenter);

    m_percentage->setAttribute(Qt::WA_TranslucentBackground);
    m_percentage->setToolTip(tr("View scale"));
    //    m_percentage->setFont(m_ft);
    //    m_percentage->setText("98%");
    //放大
    m_enlarge = new kdk::KBorderlessButton(m_zoomWid);

    m_enlarge->setFocusPolicy(Qt::NoFocus);

    m_enlarge->setToolTip(tr("Zoom in"));
    m_enlarge->setAttribute(Qt::WA_TranslucentBackground);
    //    m_enlarge->setToolTip(tr("放大图片"));
    //    m_enlarge->setAutoRepeat(true);
    //    m_enlarge->setAutoRepeatDelay(15);
    //原始尺寸
    m_originalSize = new kdk::KBorderlessButton(this);

    m_originalSize->setFocusPolicy(Qt::NoFocus);
    m_originalSize->setToolTip(tr("Life size"));
    m_originalSize->setAttribute(Qt::WA_TranslucentBackground);
    //    m_originalSize->setToolTip(tr("原始尺寸"));
    //适应窗口
    m_adaptiveWidget = new kdk::KBorderlessButton(this);

    m_adaptiveWidget->setFocusPolicy(Qt::NoFocus);
    m_adaptiveWidget->setToolTip(tr("Window widget"));
    m_adaptiveWidget->setAttribute(Qt::WA_TranslucentBackground);
    //    m_adaptiveWidget->setToolTip(tr("图像适应窗口"));
    //旋转
    m_rotate = new kdk::KBorderlessButton(this);
    m_rotate->setFocusPolicy(Qt::NoFocus);
    m_rotate->setAttribute(Qt::WA_TranslucentBackground);
    m_rotate->setToolTip(tr("Rorate right"));
    //    m_rotate->setToolTip(tr("旋转"));
    //水平翻转
    m_flipH = new kdk::KBorderlessButton(this);

    m_flipH->setFocusPolicy(Qt::NoFocus);
    m_flipH->setToolTip(tr("Flip horizontally"));
    m_flipH->setAttribute(Qt::WA_TranslucentBackground);
    //    m_flipH->setToolTip(tr("水平镜像"));
    //垂直翻转
    m_flipV = new kdk::KBorderlessButton(this);
    m_flipV->setFocusPolicy(Qt::NoFocus);
    m_flipV->setToolTip(tr("Flip vertically"));
    m_flipV->setAttribute(Qt::WA_TranslucentBackground);
    //    m_flipV->setToolTip(tr("垂直镜像"));
    //裁剪
    m_cutImage = new kdk::KBorderlessButton(this);

    m_cutImage->setFocusPolicy(Qt::NoFocus);
    m_cutImage->setToolTip(tr("Crop"));
    m_cutImage->setAttribute(Qt::WA_TranslucentBackground);
    //    cutImage->setToolTip(tr("裁剪"));

    //智能处理
    m_edit = new kdk::KBorderlessButton(this);
    m_edit->setToolTip(tr("Intelligent processing"));
    m_edit->setAttribute(Qt::WA_TranslucentBackground);
    //m_edit->setArrow(false);
    m_edit->setFocusPolicy(Qt::NoFocus);
    //m_edit->setAutoRaise(true);
    QPixmap icon(":/res/res/ai/arrowup.svg");
    changeEditIcon(icon);

    m_editMenu = new QMenu();

    //滤镜
//    m_filter = new QAction(m_editMenu);
//    m_filter->setText(tr("添加滤镜"));
    // OCR
    m_ocr = new QAction(m_editMenu);
    m_ocr->setText(tr("Text recognition"));
    //扫描黑白件
    m_scanner = new QAction(m_editMenu);
    m_scanner->setText(tr("Scanned black and white"));
    //抠图换背景
    m_imageMatting = new QAction(m_editMenu);
    m_imageMatting->setText(tr("Cutout for background"));

    //m_editMenu->addAction(m_filter);
    m_editMenu->addAction(m_ocr);
    m_editMenu->addAction(m_scanner);
#if !defined(__loongarch64) && !defined(__riscv)
    m_editMenu->addAction(m_imageMatting);
#endif

    //m_edit->setMenu(m_editMenu);

    m_line1 = new QLabel(this);
    m_line1->setFixedSize(TOOL_LINE);

    //标注
    m_labelbar = new kdk::KBorderlessButton(this); //
    m_labelbar->setFocusPolicy(Qt::NoFocus);
    m_labelbar->setToolTip(tr("Markup"));
    //    m_labelbar->setToolTip(tr("标注"));

    m_line2 = new QLabel(this);
    m_line2->setFixedSize(TOOL_LINE);
    //侧边栏--相册
    m_sidebar = new kdk::KBorderlessButton(this);

    m_sidebar->setFocusPolicy(Qt::NoFocus);
    m_sidebar->setToolTip(tr("Sidebar"));
    m_sidebar->setAttribute(Qt::WA_TranslucentBackground);
    //    m_sidebar->setToolTip(tr("缩略图侧栏"));
    //信息栏
    m_information = new kdk::KBorderlessButton(this); //
    m_information->setFocusPolicy(Qt::NoFocus);
    m_information->setToolTip(tr("Get info"));
    m_information->setAttribute(Qt::WA_TranslucentBackground);
    //    m_information->setDown(true);
    //    m_information->setBackgroundColor(QColor(255, 255, 255, 0));



    //    m_information->setToolTip(tr("信息"));
    //删除
    m_delImage = new kdk::KBorderlessButton(this);
    m_delImage->setFocusPolicy(Qt::NoFocus);
    m_delImage->setToolTip(tr("Delete"));
    m_delImage->setAttribute(Qt::WA_TranslucentBackground);
    //    m_delImage->setToolTip(tr("删除"));
    //绘制阴影
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(TOOL_COLOR);       //设置阴影颜色，也可以setColor(QColor(220,220,220))
    effect->setBlurRadius(BLUR_RADIUS); //设定阴影的模糊半径，数值越大越模糊
    g_tooleWid->setGraphicsEffect(effect);

    this->platformType();
    this->initGsetting();
    this->initControlQss();
    this->initConnect();
    this->initFontSize();
    this->updateCheckBtnQss();
    this->btnIconSize(TOOL_BUTTON);
}
//更新改名后的文件信息
void ToolBar::updateFileInfo(QFileInfo fileInfo)
{
    m_imagePath = fileInfo.absoluteFilePath();
}
//布局
void ToolBar::initControlQss()
{

    if (platForm.contains(Variable::platForm)) {
        checkBtnSize = "width: 48px; height: 48px;";
        m_toolLayout->addWidget(m_zoomWid, 0, Qt::AlignCenter);
        //        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_adaptiveWidget, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_originalSize, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_rotate, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_flipV, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_flipH, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_cutImage, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_line1, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_edit, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        //    m_toolLayout->addWidget(m_filter,0,Qt::AlignCenter);
        //    m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_labelbar, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_line2, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_sidebar, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_information, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(10);
        m_toolLayout->addWidget(m_delImage, 0, Qt::AlignCenter);
        m_toolLayout->setContentsMargins(10, 0, 10, 0);
        g_tooleWid->setLayout(m_toolLayout);
        g_tooleWid->move(2, 2);
        g_tooleWid->resize(this->width() - 4, this->height() - 4);
        //        m_line2->hide();
    } else {
        checkBtnSize = "width: 24px; height: 24px;";
        //        m_line2->show();
        m_toolLayout->addWidget(m_zoomWid, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_adaptiveWidget, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_originalSize, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_rotate, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_flipV, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_flipH, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_cutImage, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(5);
        m_toolLayout->addWidget(m_line1, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(5);
        m_toolLayout->addWidget(m_labelbar, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(5);
        //    m_toolLayout->addWidget(m_filter,0,Qt::AlignCenter);
        //    m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_edit, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_line2, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_sidebar, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_information, 0, Qt::AlignCenter);
        m_toolLayout->setSpacing(20);
        m_toolLayout->addWidget(m_delImage, 0, Qt::AlignCenter);
        m_toolLayout->setContentsMargins(14, 6, 18, 14);
        g_tooleWid->setLayout(m_toolLayout);
        g_tooleWid->move(4, 4);
        g_tooleWid->resize(this->width() - 8, this->height() - 8);
    }
}
//信号槽
void ToolBar::initConnect()
{
    connect(m_reduce, &kdk::KBorderlessButton::clicked, this, &ToolBar::reduceImageSlot);
    connect(m_enlarge, &kdk::KBorderlessButton::clicked, this, &ToolBar::enlargeImageSlot);
    connect(m_rotate, &kdk::KBorderlessButton::clicked, this, &ToolBar::rotate);
    connect(m_originalSize, &kdk::KBorderlessButton::clicked, this, &ToolBar::originalSize);
    connect(m_adaptiveWidget, &kdk::KBorderlessButton::clicked, this, &ToolBar::adaptiveWidget);
    connect(m_flipH, &kdk::KBorderlessButton::clicked, this, &ToolBar::flipH);
    connect(m_flipV, &kdk::KBorderlessButton::clicked, this, &ToolBar::flipV);
    connect(m_cutImage, &kdk::KBorderlessButton::clicked, this, &ToolBar::cutImage);
    //    connect(m_filter,&QPushButton::clicked,this,&ToolBar::filter);
    connect(m_labelbar, &kdk::KBorderlessButton::clicked, this, &ToolBar::labelbar);
    connect(m_sidebar, &kdk::KBorderlessButton::clicked, this, &ToolBar::sidebar);
    connect(m_information, &kdk::KBorderlessButton::clicked, this, &ToolBar::information);
    connect(m_delImage, &kdk::KBorderlessButton::clicked, this, &ToolBar::delImage);

    connect(m_edit, &kdk::KBorderlessButton::clicked, this, &ToolBar::showEditMenu);
    connect(m_edit, &kdk::KBorderlessButton::pressed, this, &ToolBar::showEditMenu);

    connect(m_editMenu, &QMenu::aboutToShow, [=]() {
        QPixmap icon(":/res/res/ai/arrowdown.svg");
        changeEditIcon(icon);
    });
    connect(m_editMenu, &QMenu::aboutToHide, [=]() {
        QPixmap icon(":/res/res/ai/arrowup.svg");
        changeEditIcon(icon);
    });
    //connect(m_filter, &QAction::triggered, this, &ToolBar::filter);
    connect(m_ocr, &QAction::triggered, this, &ToolBar::getText);
    connect(m_scanner, &QAction::triggered, this, &ToolBar::scanner);
    connect(m_imageMatting, &QAction::triggered, this, &ToolBar::matting);
}

void ToolBar::showEditMenu()
{
    QPoint menuPos = m_edit->mapToGlobal(QPoint(0, 0)); // 获取m_edit在全局坐标系中的位置
    QSize menuSize = m_editMenu->sizeHint();
    menuPos.setX(menuPos.x() - ((menuSize.width() - m_edit->width()) / 2));
    menuPos.setY(menuPos.y() - menuSize.height() - 12);
    m_editMenu->exec(menuPos); // 在计算的位置显示菜单
}

void ToolBar::changeEditIcon(QPixmap pix)
{
    QPixmap icon1(":/res/res/ai/ai.svg");
    QPixmap icon2 = pix;

    QPixmap pixmap(40, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    painter.drawPixmap(0, 0, icon1);
    int icon2Y = (24 - icon2.height()) / 2;
    painter.drawPixmap(icon1.size().width(), icon2Y, icon2);

    m_edit->setIcon(QIcon(pixmap));
    m_edit->setIconSize(pixmap.size());
    m_edit->setFixedSize(40, 24);
}

//改变百分比的数值
void ToolBar::changePerRate(QString num, QString path)
{
    this->m_percentage->setText(num);
    m_imagePath = path;
    m_percentage->setToolTip(num);
}

void ToolBar::reduceImageSlot()
{
    operayteMode = OperayteMode::ZoomOut;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT startZoomImage();
    //缩放图片-埋点
    kdk::kabase::BuriedPoint buriedPointZoomInandOut;
    if (buriedPointZoomInandOut.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerZoomInandOut)) {
        qCritical() << "buried point fail ZoomInandOut!";
    }
}

void ToolBar::enlargeImageSlot()
{
    operayteMode = OperayteMode::ZoomIn;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT startZoomImage();
    //缩放图片-埋点
    kdk::kabase::BuriedPoint buriedPointZoomInandOut;
    if (buriedPointZoomInandOut.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerZoomInandOut)) {
        qCritical() << "buried point fail ZoomInandOut!";
    }
}

//原始大小
void ToolBar::originalSize()
{
    operayteMode = OperayteMode::LifeSize;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->watchOriginalImage();
}
//适应窗口
void ToolBar::adaptiveWidget()
{
    operayteMode = OperayteMode::Window;
    //Interaction::getInstance()->changeWidgetSize(Variable::g_widSize);
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->watchAutoImage();
}
//顺时针旋转
void ToolBar::rotate()
{
    m_judgmentOperationFlag = "rotate";
    InteractiveQml::getInstance()->setOperateWay(-1);
    //    if (platForm.contains(Variable::platForm)) {
    //        operayteMode = OperayteMode::RotateN;
    //        InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //        Interaction::getInstance()->rotate(false);
    //    } else {
    operayteMode = OperayteMode::RotateS;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->rotate();
    //    }
    Variable::g_hasRotation = true;
    if (!Variable::g_changedImageList.contains(m_imagePath)) {
        Variable::g_changedImageList.append(m_imagePath);
    }
    m_record++;
}
void ToolBar::rotateN(){
    //记录顺时针旋转的次数，点击标注后逆时针转回去

    InteractiveQml::getInstance()->setOperateWay(-1);
    operayteMode = OperayteMode::RotateN;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->rotate();
    //    }
    Variable::g_hasRotation = true;
    if (!Variable::g_changedImageList.contains(m_imagePath)) {
        Variable::g_changedImageList.append(m_imagePath);
    }
    m_ifRotate = false;
}
//水平镜像
void ToolBar::flipH()
{
    m_judgmentOperationFlag = "fliph";
    InteractiveQml::getInstance()->setOperateWay(-1);
    operayteMode = OperayteMode::FlipH;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->flipH();
    Variable::g_hasRotation = true;
    if (!Variable::g_changedImageList.contains(m_imagePath)) {
        Variable::g_changedImageList.append(m_imagePath);
    }
}
//垂直镜像
void ToolBar::flipV()
{
    m_judgmentOperationFlag = "flipv" ;
    InteractiveQml::getInstance()->setOperateWay(-1);
    operayteMode = OperayteMode::FlipV;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->flipV();
    Variable::g_hasRotation = true;
    if (!Variable::g_changedImageList.contains(m_imagePath)) {
        Variable::g_changedImageList.append(m_imagePath);
    }
}
//裁剪
void ToolBar::cutImage()
{
    operayteMode = OperayteMode::Cut;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //    Interaction::getInstance()->cutImage();
    Q_EMIT hideTwoBar();
    Q_EMIT startCutting();
    m_sidebar->setChecked(false);
    m_information->setChecked(false);
    //裁剪-埋点
    kdk::kabase::BuriedPoint buriedPointCutting;
    if (buriedPointCutting.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                               kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerCutting)) {
        qCritical() << "buried point fail Cutting!";
    }
}
//滤镜
//void ToolBar::filter()
//{
//    qDebug() << "滤镜";
//}
//文字识别
void ToolBar::getText()
{
    operayteMode = OperayteMode::Ocr;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->getText();
    //    Q_EMIT startGetText();
    Q_EMIT viewOCRMode();
}
//扫描黑白件
void ToolBar::scanner()
{
    Q_EMIT startScanner();
}
//抠图
void ToolBar::matting()
{
    Q_EMIT startMatting();
}
//标注
void ToolBar::labelbar()
{
    //去后端拿是否操作过的列表
    if (Variable::g_hasRotation) {
        QMessageBox msg;
        msg.setParent(this);
        //标注需基于原图编辑，是否进入标注？
        msg.setText(tr("Markup needs to be edited based on the original. Do you want to enter Markup?"));
        msg.setIcon(QMessageBox::Question);
        QPushButton *sureBtn = msg.addButton(tr("Yes"), QMessageBox::AcceptRole);
        msg.addButton(tr("No"), QMessageBox::RejectRole);
        msg.exec();
        if ((QPushButton *)msg.clickedButton() == sureBtn) {
            operayteMode = OperayteMode::ChangeImage;
            InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
            Variable::g_needEnterSign = true;
            //发信号重新打开这张图片
            if(m_judgmentOperationFlag.compare("fliph") == 0){
                flipH();
                Q_EMIT markUpdateImage(m_imagePath);
                enterSign();
            }
            else if(m_judgmentOperationFlag.compare("flipv") == 0){
                flipV();
                Q_EMIT markUpdateImage(m_imagePath);
                enterSign();
            }
            else if(m_judgmentOperationFlag.compare("rotate") == 0){
                for(int i=0;i<m_record;i++){
                    rotateN();
                }
                m_record = 0;
                Q_EMIT markUpdateImage(m_imagePath);
                enterSign();
            }
            else{
                return;
            }
        }
        Variable::g_needEnterSign = false;
    } else {
        Variable::g_needEnterSign = true;
        enterSign();
    }
}
//侧边栏
void ToolBar::sidebar()
{
    Q_EMIT showSidebar();
    //图片预览-埋点
    kdk::kabase::BuriedPoint buriedPointPicturePreview;
    if (buriedPointPicturePreview.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                      kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerPicturePreview)) {
        qCritical() << "buried point fail PicturePreview!";
    }
}
//信息栏展示
void ToolBar::information()
{
    Q_EMIT showInfor();
    //查看图片信息-埋点
    kdk::kabase::BuriedPoint buriedPointPictureDetails;
    if (buriedPointPictureDetails.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                      kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerPictureDetails)) {
        qCritical() << "buried point fail PictureDetails!";
    }
}
//删除图片
void ToolBar::delImage()
{
    QString uri = "file://" + m_imagePath;
    Peony::FileOperation *trash = Peony::FileOperationUtils::trash(QStringList(uri), true);
    connect(
        trash, &Peony::FileOperation::operationFinished, this,
        [=]() {
            if (!trash->hasError()) {
                Q_EMIT isDelete(true);
                operayteMode = OperayteMode::NextImage;
                InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
                Variable::g_delOpenNext = true;
                Interaction::getInstance()->deleteImage();
                //删除图片后导致imageItem对象为空，需要自适应窗口显示一次，才能再次连接qml信号
                this->adaptiveWidget();
            }
        },
        Qt::BlockingQueuedConnection);
}
//监听主题
void ToolBar::initGsetting()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        changeStyle();
    });
    changeStyle();
    return;
}
void ToolBar::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = DEFAULT_FONT_SIZE;
    double nowFontSize = lable2BaseFontSize * double(size) / DEFAULT_FONT_SIZE; // 11为系统默认大小
    QFont font;
    font.setPointSizeF(nowFontSize);
    m_percentage->setFont(font);
}
void ToolBar::initFontSize()
{
    //只有非标准字号的控件才需要绑定
    connect(this, &ToolBar::setFontSizeSignal, this, &ToolBar::lable2SetFontSizeSlot);

    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemFontSizeChange, this, [=]() {
        //获取字号的值
        int fontSizeKey = kdk::kabase::Gsettings::getSystemFontSize().toInt();
        //发送改变信号
        if (fontSizeKey > 0) {
            if (fontSizeKey > 15) {
                fontSizeKey = 15;
            }
            Q_EMIT setFontSizeSignal(fontSizeKey);
        }
    });
    //启动时设置字号
    int fontSizeKey = DEFAULT_FONT; //系统默认字号
    int fontSizeKeyTmp = kdk::kabase::Gsettings::getSystemFontSize().toInt();
    if (fontSizeKeyTmp > 0) {
        fontSizeKey = fontSizeKeyTmp;
    }
    if (fontSizeKey > 15) {
        fontSizeKey = 15;
    }
    Q_EMIT setFontSizeSignal(fontSizeKey);
}

void ToolBar::changeStyle()
{
    //需要重新梳理此处的QSS
    nowThemeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle) {
        this->setStyleSheet("QWidget{border-radius:" + widRadius + ";border:none;color:rgb(255,255,255);}");
        g_tooleWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.66);border-radius:" + widRadius
                                  + ";border:none;}");
        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(255,255,255);}");
        m_line1->setStyleSheet("QLabel{border: 1px solid #727272;background-color: #727272;}");
        m_line2->setStyleSheet("QLabel{border: 1px solid #727272;background-color: #727272;}");
    } else {
        this->setStyleSheet("QWidget{border-radius:" + widRadius + ";border:none;color:rgb(0,0,0);}");
        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        g_tooleWid->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);border:none;border-radius:" + widRadius
                                  + ";}");
        m_line1->setStyleSheet("QLabel{border: 1px solid #393939;background-color: #393939;}");
        m_line2->setStyleSheet("QLabel{border: 1px solid #393939;background-color: #393939;}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(0,0,0);}");
    }
}

void ToolBar::setButtonState(bool isOpenSuccess, bool isCanOperate)
{
    // isOpenSuccess---是否没有损坏；isCanOperate---是否可操作及其他
    m_isOpenSuccess = isOpenSuccess;
    m_isCanOperate = isCanOperate;
    m_reduce->setEnabled(isOpenSuccess);                   //缩小
    m_enlarge->setEnabled(isOpenSuccess);                  //放大
    m_originalSize->setEnabled(isOpenSuccess);             //原始尺寸
    m_adaptiveWidget->setEnabled(isOpenSuccess);           //适应图片
    m_rotate->setEnabled(isOpenSuccess && isCanOperate);   //旋转
    m_flipH->setEnabled(isOpenSuccess && isCanOperate);    //水平翻转
    m_flipV->setEnabled(isOpenSuccess && isCanOperate);    //垂直翻转
    m_cutImage->setEnabled(isOpenSuccess && isCanOperate); //裁剪
    m_ocr->setEnabled(isOpenSuccess && isCanOperate);      // OCR
    //    m_filter->setEnabled(isOpenSuccess);//滤镜
    m_delImage->setEnabled(isOpenSuccess && isCanOperate); //删除图片
    if (SIGN_NOT_SUPPORT.contains(m_imageSuffix)) {
        m_labelbar->setEnabled(false); //标注栏
    } else {
        m_labelbar->setEnabled(isOpenSuccess && isCanOperate); //标注栏
    }
    if (!OCR_SUPPORT.contains(m_imageSuffix)) {
        m_ocr->setEnabled(false);
    }
}

void ToolBar::getImageType(bool canCut)
{
    //判断是否能进行裁剪
    if (m_isOpenSuccess) {
        m_cutImage->setEnabled(canCut);
        //        m_ocr->setEnabled(canCut);
    } else {
        m_cutImage->setEnabled(m_isOpenSuccess);
        //        m_ocr->setEnabled(m_isOpenSuccess);
    }
}

void ToolBar::getSuffix(QString suffix)
{
    m_imageSuffix = suffix;
}

void ToolBar::enterSign()
{
    operayteMode = OperayteMode::Sign;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Variable::g_allowChangeImage = false;
    Q_EMIT showMarkingWid();
    Variable::g_hasRotation = false;
}

void ToolBar::buttonSize(QSize acturalSize)
{
    m_reduce->setFixedSize(acturalSize);
    m_enlarge->setFixedSize(acturalSize);
    m_originalSize->setFixedSize(acturalSize);
    m_adaptiveWidget->setFixedSize(acturalSize);
    m_rotate->setFixedSize(acturalSize);
    m_flipH->setFixedSize(acturalSize);
    m_flipV->setFixedSize(acturalSize);
    m_cutImage->setFixedSize(acturalSize);
    m_sidebar->setFixedSize(acturalSize);
    m_information->setFixedSize(acturalSize);
    m_delImage->setFixedSize(acturalSize);
    //m_edit->setFixedSize(40,24);
    m_labelbar->setFixedSize(acturalSize);
}

void ToolBar::btnIconSize(QSize iconSize)
{
    m_reduce->setIconSize(iconSize);
    m_enlarge->setIconSize(iconSize);
    m_originalSize->setIconSize(iconSize);
    m_adaptiveWidget->setIconSize(iconSize);
    m_rotate->setIconSize(iconSize);
    m_flipH->setIconSize(iconSize);
    m_flipV->setIconSize(iconSize);
    m_cutImage->setIconSize(iconSize);
    m_sidebar->setIconSize(iconSize);
    m_information->setIconSize(iconSize);
    m_delImage->setIconSize(iconSize);
    m_labelbar->setIconSize(iconSize);
    //m_edit->setIconSize(iconSize);
}

void ToolBar::updateCheckBtnQss()
{

    m_reduce->setIcon(QIcon(":/res/res/sp1/reduce.svg"));
    m_enlarge->setIcon(QIcon(":/res/res/sp1/enlarge.svg"));
    m_originalSize->setIcon(QIcon(":/res/res/sp1/originalSize.svg"));
    m_adaptiveWidget->setIcon(QIcon(":/res/res/sp1/adaptiveWidget.svg"));
    m_rotate->setIcon(QIcon(":/res/res/sp1/rotate.svg"));
    m_flipH->setIcon(QIcon(":/res/res/sp1/flipH.svg"));
    m_flipV->setIcon(QIcon(":/res/res/sp1/flipV.svg"));
    m_cutImage->setIcon(QIcon(":/res/res/sp1/cutImage.svg"));
    //    m_filter->setIcon(QIcon(":/res/res/sp1/filter.svg"));
    //    m_filter->setIconSize(QSize(24, 24));
    m_labelbar->setIcon(QIcon(":/res/res/sp1/mark.svg"));
    m_sidebar->setIcon(QIcon(":/res/res/sp1/sidebar.svg"));
    m_information->setIcon(QIcon(":/res/res/sp1/information.svg"));
    m_delImage->setIcon(QIcon(":/res/res/sp1/delImage.svg"));

    //m_edit->setIcon(QIcon(":/res/res/ai/ai.svg"));
    m_ocr->setIcon(QIcon(":/res/res/ai/ocr.svg"));
    m_scanner->setIcon(QIcon(":/res/res/ai/scanner.svg"));
    m_imageMatting->setIcon(QIcon(":/res/res/ai/cutout.svg"));
}

void ToolBar::platformType()
{
    normalIconPath = "sp1";
    if (platForm.contains(Variable::platForm)) {
        this->resize(TOOLBAR_SIZE_INTEL);
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE_INTEL);
        widRadius = "12px";
        this->buttonSize(TOOL_BUTTON_INTEL);
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5,
                           (m_reduce->height() - m_percentage->height()) / 2 - 3);
    } else {
        this->resize(TOOLBAR_SIZE);
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE);
        widRadius = "6px";
        this->buttonSize(TOOL_BUTTON);
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5, 0);
    }
    m_enlarge->move(m_zoomWid->width() - m_enlarge->width(), 0);
}
