#include "replacebackgroundwidget.h"
ReplaceBackgroundWidget::ReplaceBackgroundWidget(QWidget *parent)
    : QWidget(parent),
      m_colorDialog(nullptr)
{
    m_layout = new QHBoxLayout();
    m_rightStackWidget = new QStackedWidget();
    m_showImageWidget = new QWidget(this);

    this->initMattingWidget();
    this->initCutWidget();
    this->initLayout();
    this->initQmlObject();
    this->initConnect();
    this->initGsetting();
    this->setColorBtnStyle();
    this->toolBtnIconSize(TOOL_BUTTON);
    this->colorButtonSize(COLOR_BUTTON);
    this->pictureBtnSize(PICTURE_BUTTON);
    this->setBtnIcon();
    this->platformType();
    this->setFormatedText();
    this->initQWidgetObject();

    m_toolWid->move(int((m_showImageWidget->width() - m_toolWid->width()) / 2),
                    m_showImageWidget->height() - m_toolWid->height() - 6);
    m_toolWid->show();
    m_cutBtnWid->move(int((m_showImageWidget->width() - m_cutBtnWid->width()) / 2),
                      m_showImageWidget->height() - m_cutBtnWid->height() - 6);
    m_cutBtnWid->hide();

    //绘制阴影
    this->setShadowEffect(m_compareWidget);
    this->setShadowEffect(m_toolBtnWid);
    this->setShadowEffect(m_cutBtnWid);
    this->setShadowEffect(m_rightStackWidget);
    //毛玻璃
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

void ReplaceBackgroundWidget::setShadowEffect(QWidget *widget)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);
    effect->setColor(TOOL_COLOR);
    effect->setBlurRadius(BLUR_RADIUS);
    widget->setGraphicsEffect(effect);
}

void ReplaceBackgroundWidget::initMattingWidget()
{
    //工具栏
    m_toolBtnWid = new QWidget(this);
    m_toolBtnWid->setWindowFlags(Qt::FramelessWindowHint);
    m_toolBtnWid->setFocusPolicy(Qt::NoFocus);
    m_toolBtnLayout = new QHBoxLayout(m_toolBtnWid);

    //对比
    m_compareWidget = new QWidget(this);
    m_compareWidget->setFocusPolicy(Qt::NoFocus);
    m_compareWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_compare = new kdk::KBorderlessButton(m_compareWidget);
    m_compare->setFocusPolicy(Qt::NoFocus);
    m_compare->setText(tr("compare"));
    m_compare->setToolTip(tr("Press and hold to view the original image"));

    //对比+工具栏布局
    m_toolWid = new QFrame(this);
    m_toolLayout = new QHBoxLayout(m_toolWid);

    //缩放widget
    m_zoomWid = new QWidget(this);
    m_zoomWid->setFixedSize(TOOLZOOM_SIZE);
    //缩小
    m_reduce = new kdk::KBorderlessButton(m_zoomWid);
    m_reduce->setFocusPolicy(Qt::NoFocus);
    m_reduce->setToolTip(tr("Zoom out"));
    m_reduce->setAttribute(Qt::WA_TranslucentBackground);
    //百分比
    m_percentage = new QLabel(m_zoomWid);
    m_percentage->setFixedSize(TOOL_PER);
    m_percentage->setAlignment(Qt::AlignCenter);
    m_percentage->setAttribute(Qt::WA_TranslucentBackground);
    m_percentage->setToolTip(tr("View scale"));
    //放大
    m_enlarge = new kdk::KBorderlessButton(m_zoomWid);
    m_enlarge->setFocusPolicy(Qt::NoFocus);
    m_enlarge->setToolTip(tr("Zoom in"));
    m_enlarge->setAttribute(Qt::WA_TranslucentBackground);
    //原尺寸
    m_originalSize = new kdk::KBorderlessButton(this);
    m_originalSize->setFocusPolicy(Qt::NoFocus);
    m_originalSize->setAttribute(Qt::WA_TranslucentBackground);
    m_originalSize->setToolTip(tr("Life size"));
    //适应窗口
    m_adaptiveWidget = new kdk::KBorderlessButton(this);
    m_adaptiveWidget->setFocusPolicy(Qt::NoFocus);
    m_adaptiveWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_adaptiveWidget->setToolTip(tr("Window widget"));
    //旋转
    m_rotate = new kdk::KBorderlessButton(this);
    m_rotate->setFocusPolicy(Qt::NoFocus);
    m_rotate->setAttribute(Qt::WA_TranslucentBackground);
    m_rotate->setToolTip(tr("Rorate right"));
    //水平翻转
    m_flipH = new kdk::KBorderlessButton(this);
    m_flipH->setFocusPolicy(Qt::NoFocus);
    m_flipH->setAttribute(Qt::WA_TranslucentBackground);
    m_flipH->setToolTip(tr("Flip horizontally"));
    //垂直翻转
    m_flipV = new kdk::KBorderlessButton(this);
    m_flipV->setFocusPolicy(Qt::NoFocus);
    m_flipV->setAttribute(Qt::WA_TranslucentBackground);
    m_flipV->setToolTip(tr("Flip vertically"));
    //裁剪
    m_cutImage = new kdk::KBorderlessButton(this);
    m_cutImage->setFocusPolicy(Qt::NoFocus);
    m_cutImage->setAttribute(Qt::WA_TranslucentBackground);
    m_cutImage->setToolTip(tr("Crop"));

    m_mattingWidget = new QWidget();
    m_mattingWidget->setAutoFillBackground(true);

    m_mattingTopWidget = new QWidget();
    m_mattingMidWidget = new QWidget();
    m_mattingBottomWidget = new QWidget();
    m_colorCustomWidget = new QWidget();
    m_colorBtnWidget= new QWidget();
    m_picBtnWidget = new QWidget();
    m_picBtnWidget->setStyleSheet("QWidget{border:0px;background-color:rgb(255, 255, 255)}");

    m_scrollArea = new QScrollArea();
    m_scrollArea->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet("QScrollArea{border:0px;background-color:rgb(255, 255, 255)}");

    m_mattingLayout = new QVBoxLayout();
    m_mattingTopLayout = new QHBoxLayout();
    m_mattingMidLayout = new QVBoxLayout();
    m_mattingBottomLayout = new QHBoxLayout();
    m_customBtnLayout = new QHBoxLayout();
    m_colorCustomLayout = new QHBoxLayout();
    m_colorBtnLayout = new QGridLayout();
    m_picBtnLayout = new QGridLayout();
    m_picBtnLayout->setAlignment(Qt::AlignTop);

    m_bgLabel1 = new QLabel();
    m_bgLabel1->setPixmap(QPixmap(":/res/res/ai/changebackground.svg"));
    m_bgLabel2 = new QLabel();
    m_bgLabel2->setText(tr("Background replacement"));
    m_bgLabel2->setStyleSheet("QLabel{color:grey;}");

    m_pureColor = new QLabel();
    m_pureColor->setText(tr("Pure color"));
    m_pureColor->setFocusPolicy(Qt::NoFocus);
    m_pureColor->setAttribute(Qt::WA_TranslucentBackground);
    m_pureColor->setStyleSheet("QLabel{color:grey;}");

    m_customBtn = new QPushButton(this);
    m_customBtn->setFocusPolicy(Qt::NoFocus);
    m_customBtn->setFixedSize(QSize(60, 24));
    m_customBtn->setAttribute(Qt::WA_TranslucentBackground);

    m_customBtnLeft = new QLabel();
    m_customBtnLeft->setFocusPolicy(Qt::NoFocus);
    m_customBtnLeft->setFixedSize(QSize(24, 24));

    m_customBtnRight = new QLabel();
    m_customBtnRight->setFocusPolicy(Qt::NoFocus);
    m_customBtnRight->setFixedSize(QSize(36, 24));

    m_transparent = new QPushButton();
    m_transparent->setFocusPolicy(Qt::NoFocus);

    m_white = new QPushButton();
    m_white->setFocusPolicy(Qt::NoFocus);

    m_grey = new QPushButton();
    m_grey->setFocusPolicy(Qt::NoFocus);

    m_dimGrey = new QPushButton();
    m_dimGrey->setFocusPolicy(Qt::NoFocus);

    m_dark = new QPushButton();
    m_dark->setFocusPolicy(Qt::NoFocus);

    m_blue = new QPushButton();
    m_blue->setFocusPolicy(Qt::NoFocus);

    m_red = new QPushButton();
    m_red->setFocusPolicy(Qt::NoFocus);

    m_darkOrange = new QPushButton();
    m_darkOrange->setFocusPolicy(Qt::NoFocus);

    m_orange = new QPushButton();
    m_orange->setFocusPolicy(Qt::NoFocus);

    m_green = new QPushButton();
    m_green->setFocusPolicy(Qt::NoFocus);

    m_purple = new QPushButton();
    m_purple->setFocusPolicy(Qt::NoFocus);

    m_pink = new QPushButton();
    m_pink->setFocusPolicy(Qt::NoFocus);

    m_pic = new QLabel();
    m_pic->setText(tr("Picture"));
    m_pic->setFocusPolicy(Qt::NoFocus);
    m_pic->setStyleSheet("QLabel{color:grey;}");

    m_addPicBtn = new kdk::KPushButton();
    m_addPicBtn->setFocusPolicy(Qt::NoFocus);
    m_addPicBtn->setIcon(QIcon(":/res/res/addplus.svg"));
    m_addPicBtn->setIconSize(QSize(24, 24));
    m_addPicBtn->setIconColor(QColor(180, 180, 180));
    m_addPicBtn->setBackgroundColor(QColor(232, 232, 232));

    m_pic1 = new QPushButton();
    m_pic1->setFocusPolicy(Qt::NoFocus);
    QString picPath1 = QString(":/res/res/backgroundpictures/1.jpg");
    m_picBtnToPathMap[m_pic1] = picPath1;

    m_pic2 = new QPushButton();
    m_pic2->setFocusPolicy(Qt::NoFocus);
    QString picPath2 = QString(":/res/res/backgroundpictures/2.jpg");
    m_picBtnToPathMap[m_pic2] = picPath2;

    m_pic3 = new QPushButton();
    m_pic3->setFocusPolicy(Qt::NoFocus);
    QString picPath3 = QString(":/res/res/backgroundpictures/3.jpg");
    m_picBtnToPathMap[m_pic3] = picPath3;

    m_pic4 = new QPushButton();
    m_pic4->setFocusPolicy(Qt::NoFocus);
    QString picPath4 = QString(":/res/res/backgroundpictures/4.jpg");
    m_picBtnToPathMap[m_pic4] = picPath4;

    m_pic5 = new QPushButton();
    m_pic5->setFocusPolicy(Qt::NoFocus);
    QString picPath5 = QString(":/res/res/backgroundpictures/5.jpg");
    m_picBtnToPathMap[m_pic5] = picPath5;

    m_list << m_addPicBtn << m_pic1 << m_pic2 << m_pic3 << m_pic4 << m_pic5;

    m_saveBtn = new QPushButton();
    m_saveBtn->setText(tr("Save copy"));
    m_saveBtn->setFixedSize(QSize(100, 36));
    m_saveBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn= new QPushButton();
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedSize(QSize(100, 36));
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);
}

void ReplaceBackgroundWidget::initCutWidget()
{
    m_cutWidget = new QWidget();
    m_cutWidget->setAutoFillBackground(true);

    m_cutTopWid = new QWidget();
    m_cutMidWid = new QWidget();
    m_cutBottomWid = new QWidget();
    m_sizeWid = new QListWidget();
    m_scaleWid = new QListWidget();

    m_cutTopLayout = new QHBoxLayout();
    m_cutMidLayout = new QVBoxLayout();
    m_sizeLayout = new QVBoxLayout();
    m_cutBottomLayout = new QVBoxLayout();
    m_scaleLayout = new QVBoxLayout();
    m_cutLayout = new QVBoxLayout();

    m_cutLabel1 = new QLabel();
    m_cutLabel1->setPixmap(QPixmap(":/res/res/ai/cutImage.svg"));
    m_cutLabel2 = new QLabel();
    m_cutLabel2->setText(tr("Size cutting"));
    m_cutLabel2->setStyleSheet("QLabel{color:grey;}");

    m_sizeLabel = new QLabel();
    m_sizeLabel->setText(tr("Size"));
    m_sizeLabel->setFocusPolicy(Qt::NoFocus);
    m_sizeLabel->setStyleSheet("QLabel{color:grey;}");

    m_lifeSize = new QListWidgetItem(m_sizeWid);
    m_lifeSize->setText(tr("Original size"));
    m_lifeSize->setSizeHint(QSize(0, 36));
    m_freeSize = new QListWidgetItem(m_sizeWid);
    m_freeSize->setText(tr("Free size"));
    m_freeSize->setSizeHint(QSize(0, 36));
    m_standardOneInch = new QListWidgetItem(m_sizeWid);
    m_standardOneInch->setText(tr("Standard 1 inch：295*413px"));
    m_standardOneInch->setSizeHint(QSize(0, 36));
    m_standardTwoInch = new QListWidgetItem(m_sizeWid);
    m_standardTwoInch->setText(tr("Standard 2 inch：413*626px"));
    m_standardTwoInch->setSizeHint(QSize(0, 36));
    m_smallOneInch = new QListWidgetItem(m_sizeWid);
    m_smallOneInch->setText(tr("Small 1 inch：259*377px"));
    m_smallOneInch->setSizeHint(QSize(0, 36));
    m_smallTwoInch = new QListWidgetItem(m_sizeWid);
    m_smallTwoInch->setText(tr("Small 2 inch：413*531px"));
    m_smallTwoInch->setSizeHint(QSize(0, 36));
    m_idCardSize = new QListWidgetItem(m_sizeWid);
    m_idCardSize->setText(tr("Id card：358*441px"));
    m_idCardSize->setSizeHint(QSize(0, 36));
    m_sizeWid->setFixedHeight(255);

    m_scaleLabel = new QLabel();
    m_scaleLabel->setText(tr("Proportion"));
    m_scaleLabel->setFocusPolicy(Qt::NoFocus);
    m_scaleLabel->setStyleSheet("QLabel{color:grey;}");

    m_originalScale = new QListWidgetItem(m_scaleWid);
    m_originalScale->setText(tr("Original proportion"));
    m_originalScale->setSizeHint(QSize(0, 36));
    m_fourToThree = new QListWidgetItem(m_scaleWid);
    m_fourToThree->setText(tr("4:3 Standard landscape"));
    m_fourToThree->setSizeHint(QSize(0, 36));
    m_threeTofour = new QListWidgetItem(m_scaleWid);
    m_threeTofour->setText(tr("3:4 Standard portrait"));
    m_threeTofour->setSizeHint(QSize(0, 36));
    m_sixteenToNine = new QListWidgetItem(m_scaleWid);
    m_sixteenToNine->setText(tr("16:9 Video landscape"));
    m_sixteenToNine->setSizeHint(QSize(0, 36));
    m_nineToSixteen = new QListWidgetItem(m_scaleWid);
    m_nineToSixteen->setText(tr("9:16 Video portrait"));
    m_nineToSixteen->setSizeHint(QSize(0, 36));
    m_oneToOne = new QListWidgetItem(m_scaleWid);
    m_oneToOne->setText(tr("1:1 Square"));
    m_oneToOne->setSizeHint(QSize(0, 36));
    m_scaleWid->setFixedHeight(220);
    QString styleSheet = "QListWidget{"
                         "background-color: rgb(240, 240, 240);"
                         "border:none}"
                         "QListWidget::item:selected {"
                         "background-color: #0062F0;"
                         "color: white;"
                         "}";
    m_sizeWid->setStyleSheet(styleSheet);
    m_scaleWid->setStyleSheet(styleSheet);

    QPalette pal = qApp->palette();
    QColor textColor = QColor(38, 38, 38, 255);
    pal.setBrush(QPalette::Text, textColor);
    m_sizeWid->setPalette(pal);
    m_scaleWid->setPalette(pal);

    m_cutBtnWid = new QWidget(this);
    m_cutBtnWid->setFixedSize(96, 44);
    m_cutCancel = new QPushButton(m_cutBtnWid);
    m_cutCancel->setFixedSize(24, 24);
    m_cutCancel->setToolTip(tr("Cancel"));
    m_cutCancel->move(16, 10);
    m_cutCancel->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/cancel.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}");
    m_cutSave = new QPushButton(m_cutBtnWid);
    m_cutSave->setFixedSize(24, 24);
    m_cutSave->setToolTip(tr("Save"));
    m_cutSave->move(20 + m_cutCancel->width() + 8, 10);
    m_cutSave->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/save.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}");

}

void ReplaceBackgroundWidget::initLayout()
{
    m_toolBtnLayout->addWidget(m_zoomWid, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_adaptiveWidget, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_originalSize, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_rotate, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_flipH, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_flipV, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnLayout->addWidget(m_cutImage, 0, Qt::AlignCenter);
    m_toolBtnWid->setLayout(m_toolBtnLayout);

    m_toolLayout->addWidget(m_compareWidget);
    m_toolLayout->setSpacing(10);
    m_toolLayout->addWidget(m_toolBtnWid);
    m_toolWid->setLayout(m_toolLayout);

    m_mattingTopLayout->addWidget(m_bgLabel1);
    m_mattingTopLayout->addWidget(m_bgLabel2);
    m_mattingTopLayout->addStretch();
    m_mattingTopLayout->setContentsMargins(20, 0, 20, 0);
    m_mattingTopWidget->setLayout(m_mattingTopLayout);

    m_customBtnLayout->addWidget(m_customBtnLeft);
    m_customBtnLayout->addWidget(m_customBtnRight);
    m_customBtnLayout->setSpacing(0);
    m_customBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_customBtn->setLayout(m_customBtnLayout);

    m_colorCustomLayout->addWidget(m_pureColor, 0, Qt::AlignLeft);
    m_colorCustomLayout->addWidget(m_customBtn, 0, Qt::AlignRight);
    m_colorCustomLayout->setContentsMargins(0, 0, 0, 0);
    m_colorCustomWidget->setLayout(m_colorCustomLayout);

    m_colorBtnLayout->addWidget(m_transparent, 0, 0);
    m_colorBtnLayout->addWidget(m_white, 0, 1);
    m_colorBtnLayout->addWidget(m_grey, 0, 2);
    m_colorBtnLayout->addWidget(m_dimGrey, 0, 3);
    m_colorBtnLayout->addWidget(m_dark, 0, 4);
    m_colorBtnLayout->addWidget(m_blue, 0, 5);
    m_colorBtnLayout->addWidget(m_red, 1, 0);
    m_colorBtnLayout->addWidget(m_darkOrange, 1, 1);
    m_colorBtnLayout->addWidget(m_orange, 1, 2);
    m_colorBtnLayout->addWidget(m_green, 1, 3);
    m_colorBtnLayout->addWidget(m_purple, 1, 4);
    m_colorBtnLayout->addWidget(m_pink, 1, 5);
    m_colorBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_colorBtnWidget->setLayout(m_colorBtnLayout);

    m_picBtnLayout->addWidget(m_addPicBtn, 0, 0);
    m_picBtnLayout->addWidget(m_pic1, 0, 1);
    m_picBtnLayout->addWidget(m_pic2, 0, 2);
    m_picBtnLayout->addWidget(m_pic3, 1, 0);
    m_picBtnLayout->addWidget(m_pic4, 1, 1);
    m_picBtnLayout->addWidget(m_pic5, 1, 2);
    m_picBtnLayout->setContentsMargins(20, 0, 20, 0);
    m_picBtnWidget->setLayout(m_picBtnLayout);
    m_scrollArea->setWidget(m_picBtnWidget);

    m_mattingMidLayout->addWidget(m_colorCustomWidget);
    m_mattingMidLayout->addWidget(m_colorBtnWidget);
    m_mattingMidLayout->addWidget(m_pic, 0, Qt::AlignLeft);
    m_mattingMidLayout->setContentsMargins(20, 0, 20, 0);
    m_mattingMidWidget->setLayout(m_mattingMidLayout);

    m_mattingBottomLayout->addWidget(m_cancelBtn);
    m_mattingBottomLayout->setSpacing(20);
    m_mattingBottomLayout->addWidget(m_saveBtn);
    m_mattingBottomLayout->setContentsMargins(20, 0, 20, 10);
    m_mattingBottomWidget->setLayout(m_mattingBottomLayout);

    m_mattingLayout->addWidget(m_mattingTopWidget);
    m_mattingLayout->addWidget(m_mattingMidWidget);
    m_mattingLayout->addWidget(m_scrollArea);
    m_mattingLayout->addStretch();
    m_mattingLayout->addWidget(m_mattingBottomWidget);
    m_mattingLayout->setContentsMargins(0, 0, 0, 0);
    m_mattingWidget->setLayout(m_mattingLayout);

    m_cutTopLayout->addWidget(m_cutLabel1);
    m_cutTopLayout->addWidget(m_cutLabel2);
    m_cutTopLayout->addStretch();
    m_cutTopLayout->setContentsMargins(10, 0, 10, 0);
    m_cutTopWid->setLayout(m_cutTopLayout);

    m_cutMidLayout->addWidget(m_sizeLabel, 0, Qt::AlignLeft);
    m_cutMidLayout->addWidget(m_sizeWid);
    m_cutMidLayout->setContentsMargins(10, 0, 10, 0);
    m_cutMidWid->setLayout(m_cutMidLayout);

    m_cutBottomLayout->addWidget(m_scaleLabel, 0, Qt::AlignLeft);
    m_cutBottomLayout->addWidget(m_scaleWid);
    m_cutBottomLayout->setContentsMargins(10, 0, 10, 0);
    m_cutBottomWid->setLayout(m_cutBottomLayout);

    m_cutLayout->addWidget(m_cutTopWid);
    m_cutLayout->addWidget(m_cutMidWid);
    m_cutLayout->addWidget(m_cutBottomWid);
    m_cutLayout->addStretch();
    m_cutLayout->setSpacing(10);
    m_cutLayout->setContentsMargins(0, 0, 0, 0);
    m_cutWidget->setLayout(m_cutLayout);

    m_rightStackWidget->addWidget(m_mattingWidget);
    m_rightStackWidget->addWidget(m_cutWidget);

    m_layout->addWidget(m_showImageWidget);
    m_layout->addWidget(m_rightStackWidget);
    m_layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_layout);
}

void ReplaceBackgroundWidget::initConnect()
{
    //对比
    connect(m_compare, &QPushButton::pressed, this, [=]() {
        comparePressed();
    });
    connect(m_compare, &QPushButton::released, this, [=]() {
        compareReleased();
    });
    //工具栏
    connect(m_reduce, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::reduceImage);
    connect(m_enlarge, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::enlargeImage);
    connect(m_rotate, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::rotate);
    connect(m_originalSize, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::originalSize);
    connect(m_adaptiveWidget, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::adaptiveWidget);
    connect(m_flipH, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::flipH);
    connect(m_flipV, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::flipV);
    connect(m_cutImage, &kdk::KBorderlessButton::clicked, this, &ReplaceBackgroundWidget::cutImage);
    //添加图片
    connect(m_addPicBtn, &kdk::KPushButton::clicked, this, &ReplaceBackgroundWidget::addPicture);
    connect(m_pic1, &QPushButton::clicked, this, &ReplaceBackgroundWidget::picReplaceBackground);
    connect(m_pic2, &QPushButton::clicked, this, &ReplaceBackgroundWidget::picReplaceBackground);
    connect(m_pic3, &QPushButton::clicked, this, &ReplaceBackgroundWidget::picReplaceBackground);
    connect(m_pic4, &QPushButton::clicked, this, &ReplaceBackgroundWidget::picReplaceBackground);
    connect(m_pic5, &QPushButton::clicked, this, &ReplaceBackgroundWidget::picReplaceBackground);
    //纯色
    connect(m_customBtn, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_transparent, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_white, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_grey, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_dimGrey, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_dark, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_blue, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_red, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_darkOrange, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_orange, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_green, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_purple, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    connect(m_pink, &QPushButton::clicked, this, &ReplaceBackgroundWidget::startReplaceBackground);
    //保存取消
    connect(m_saveBtn, &QPushButton::clicked, this, &ReplaceBackgroundWidget::needSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ReplaceBackgroundWidget::exitReplaceBackground);

    connect(Interaction::getInstance(), &Interaction::returnReplaceAndOrigImage, this, &ReplaceBackgroundWidget::getReplaceAndOrigImage);
    connect(Interaction::getInstance(), &Interaction::returnFlipAndOrigImage, this, &ReplaceBackgroundWidget::getFlipAndOrigImage);

    connect(m_sizeWid, &QListWidget::itemClicked, this, &ReplaceBackgroundWidget::onSizeItemClicked);
    connect(m_sizeWid, &QListWidget::itemSelectionChanged, this, &ReplaceBackgroundWidget::onSizeItemSelectionChanged);
    connect(m_scaleWid, &QListWidget::itemClicked, this, &ReplaceBackgroundWidget::onScaleItemClicked);
    connect(m_scaleWid, &QListWidget::itemSelectionChanged, this, &ReplaceBackgroundWidget::onScaleItemSelectionChanged);
    connect(m_cutCancel, &QPushButton::clicked, this, &ReplaceBackgroundWidget::toMattingPage);
    connect(m_cutSave, &QPushButton::clicked, this, &ReplaceBackgroundWidget::cutDone);
}

void ReplaceBackgroundWidget::setBtnIcon()
{
    //m_transparent->setIcon(QIcon(":/res/res/ai/transparent.svg"));

    m_compare->setIcon(QIcon(":/res/res/ai/compare.svg"));
    m_reduce->setIcon(QIcon(":/res/res/sp1/reduce.svg"));
    m_enlarge->setIcon(QIcon(":/res/res/sp1/enlarge.svg"));
    m_originalSize->setIcon(QIcon(":/res/res/sp1/originalSize.svg"));
    m_adaptiveWidget->setIcon(QIcon(":/res/res/sp1/adaptiveWidget.svg"));
    m_rotate->setIcon(QIcon(":/res/res/sp1/rotate.svg"));
    m_flipH->setIcon(QIcon(":/res/res/sp1/flipH.svg"));
    m_flipV->setIcon(QIcon(":/res/res/sp1/flipV.svg"));
    m_cutImage->setIcon(QIcon(":/res/res/sp1/cutImage.svg"));
}

void ReplaceBackgroundWidget::toolBtnIconSize(QSize size)
{
    m_compare->setIconSize(size);
    m_reduce->setIconSize(size);
    m_enlarge->setIconSize(size);
    m_originalSize->setIconSize(size);
    m_adaptiveWidget->setIconSize(size);
    m_rotate->setIconSize(size);
    m_flipH->setIconSize(size);
    m_flipV->setIconSize(size);
    m_cutImage->setIconSize(size);
}

void ReplaceBackgroundWidget::toolButtonSize(QSize size)
{
    m_reduce->setFixedSize(size);
    m_enlarge->setFixedSize(size);
    m_originalSize->setFixedSize(size);
    m_adaptiveWidget->setFixedSize(size);
    m_rotate->setFixedSize(size);
    m_flipH->setFixedSize(size);
    m_flipV->setFixedSize(size);
    m_cutImage->setFixedSize(size);
}

void ReplaceBackgroundWidget::colorButtonSize(QSize size)
{
    m_transparent->setFixedSize(size);
    m_transparent->setIconSize(size);

    m_white->setFixedSize(size);
    m_grey->setFixedSize(size);
    m_dimGrey->setFixedSize(size);
    m_dark->setFixedSize(size);
    m_blue->setFixedSize(size);
    m_red->setFixedSize(size);
    m_darkOrange->setFixedSize(size);
    m_orange->setFixedSize(size);
    m_green->setFixedSize(size);
    m_purple->setFixedSize(size);
    m_pink->setFixedSize(size);
}

void ReplaceBackgroundWidget::pictureBtnSize(QSize size)
{
    m_addPicBtn->setFixedSize(size);
    m_pic1->setFixedSize(size);
    m_pic2->setFixedSize(size);
    m_pic3->setFixedSize(size);
    m_pic4->setFixedSize(size);
    m_pic5->setFixedSize(size);

    m_pic1->setIconSize(size);
    m_pic2->setIconSize(size);
    m_pic3->setIconSize(size);
    m_pic4->setIconSize(size);
    m_pic5->setIconSize(size);
}

void ReplaceBackgroundWidget::platformType()
{
    //normalIconPath = "sp1";
    if (platForm.contains(Variable::platForm)) {
        m_toolBtnWid->resize(TOOLBAR_SIZE_INTEL);
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE_INTEL);
        widRadius = "12px";
        this->toolButtonSize(TOOL_BUTTON_INTEL);
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5,
                           (m_reduce->height() - m_percentage->height()) / 2 - 3);
    } else {
        m_toolWid->setFixedSize(560, 54);
        m_toolBtnWid->setFixedSize(QSize(450, 44));
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE);
        widRadius = "6px";
        this->toolButtonSize(TOOL_BUTTON);
        m_compareWidget->setFixedSize(QSize(80, 44));
        m_compare->setFixedSize(QSize(80, 44));
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5, 0);
    }
    m_enlarge->move(m_zoomWid->width() - m_enlarge->width(), 0);
}

void ReplaceBackgroundWidget::initGsetting()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        changeStyle();
    });
    changeStyle();
    return;
}

void ReplaceBackgroundWidget::changeStyle()
{
    nowThemeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    QPalette pal = qApp->palette();
    QColor backgroundColor;
    if ("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle) {
        m_toolBtnWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.66);"
                                 "border:none;"
                                 "border-radius:" + widRadius + ";}");
        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        m_compareWidget->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.66);"
                                       "border:none;"
                                       "border-radius:" + widRadius + ";}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(255,255,255);}");
        m_cutBtnWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.72);border-radius:6px;}");
        backgroundColor = QColor(15, 15, 15, 165);
    } else {
        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        m_toolBtnWid->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);"
                                    "border:none;"
                                    "border-radius:" + widRadius + ";}");
        m_compareWidget->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);"
                                       "border:none;"
                                       "border-radius:" + widRadius + ";}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(0,0,0);}");
        m_cutBtnWid->setStyleSheet("QWidget{background-color:rgba(255, 255, 255, 1);border-radius:6px;}");
        backgroundColor = QColor(Qt::white);
    }
    m_saveBtn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:#0062F0;color:white;}"
                             "QPushButton::pressed{border:0px;border-radius:4px;background:#0062F0;color:white;}");
    m_cancelBtn->setStyleSheet("QPushButton{background-color:rgb(230, 230, 230);"
                               "border:none;"
                               "border-radius:4px"
                               "color:rgb(0, 0, 0);}");
    pal.setColor(QPalette::Background, backgroundColor);
    m_mattingWidget->setPalette(pal);
    m_cutWidget->setPalette(pal);
}

void ReplaceBackgroundWidget::setColorBtnStyle()
{
    m_customBtn->setStyleSheet("QPushButton{background-color:transparent;"
                               "border:none;"
                               "border-radius: 4px;}");
    m_customBtnLeft->setStyleSheet("QLabel{border-image: url(:/res/res/ai/colorwheel.png);"
                                   "background-color: transparent;"
                                   "border: none;"
                                   "border-top-left-radius:4px;"
                                   "border-bottom-left-radius:4px;"
                                   "}");
    m_customBtnRight->setStyleSheet("QLabel{border-image: url(:/res/res/ai/transparentselector.svg);"
                                    "background-color: transparent;"
                                    "border: none;"
                                    "border-top-right-radius:4px;"
                                    "border-bottom-right-radius:4px;"
                                    "}");

    m_transparent->setStyleSheet("QPushButton{border-image: url(:/res/res/ai/transparent.svg);"
                                 "background-color: transparent;"
                                 "border: none;"
                                 "border-radius: 4px;}");
    m_white->setStyleSheet("QPushButton{background: rgb(255, 255, 255); border:1px solid rgb(190, 190, 190); "
                              "border-radius: 4px;}");
    m_grey->setStyleSheet("QPushButton{background: rgb(190, 190, 190); border: none; border-radius: 4px;}");
    m_dimGrey->setStyleSheet("QPushButton{background: rgb(105, 105, 105); border: none; border-radius: 4px;}");
    m_dark->setStyleSheet("QPushButton{background: rgb(0, 0, 0); border: none; border-radius: 4px;}");
    m_blue->setStyleSheet("QPushButton{background: rgb(0, 0, 255); border: none; border-radius: 4px;}");
    m_red->setStyleSheet("QPushButton{background: rgb(255, 0, 0); border: none; border-radius: 4px;}");
    m_darkOrange->setStyleSheet("QPushButton{background: rgb(255, 127, 0); border: none; border-radius: 4px;}");
    m_orange->setStyleSheet("QPushButton{background: rgb(255, 165, 0); border: none; border-radius: 4px;}");
    m_green->setStyleSheet("QPushButton{background: rgb(0, 255, 0); border: none; border-radius: 4px;}");
    m_purple->setStyleSheet("QPushButton{background: rgb(160, 32, 240); border: none; border-radius: 4px;}");
    m_pink->setStyleSheet("QPushButton{background: rgb(255, 105, 180); border: none; border-radius: 4px;}");

//    m_addPicBtn->setStyleSheet("background-color:rgb(232, 232, 232);border:none;border-radius:4px;"
//                               "hover{background-color:rgb(232, 232, 232);border:none;color:#0062F0;border-radius:4px;"
//                               "pressed{background-color:rgb(232, 232, 232);color:#0062F0;border:none;border-radius:4px;");
    m_pic1->setStyleSheet("QPushButton{border-image:url(:/res/res/backgroundpictures/1.jpg);"
                          "border:none;"
                          "border-radius:4px;}");
    m_pic2->setStyleSheet("QPushButton{border-image:url(:/res/res/backgroundpictures/2.jpg);"
                          "border:none;"
                          "border-radius:4px;}");
    m_pic3->setStyleSheet("QPushButton{border-image:url(:/res/res/backgroundpictures/3.jpg);"
                          "border:none;"
                          "border-radius:4px;}");
    m_pic4->setStyleSheet("QPushButton{border-image:url(:/res/res/backgroundpictures/4.jpg);"
                          "border:none;"
                          "border-radius:4px;}");
    m_pic5->setStyleSheet("QPushButton{border-image:url(:/res/res/backgroundpictures/5.jpg);"
                          "border:none;"
                          "border-radius:4px;}");
}

void ReplaceBackgroundWidget::addPicture()
{
    QFileDialog m_fileDialog;
    m_fileDialog.update();
    //设置视图模式
    m_fileDialog.setViewMode(QFileDialog::Detail);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    m_fileDialog.setFileMode(QFileDialog::ExistingFiles);
    m_fileDialog.setOption(QFileDialog::HideNameFilterDetails);

    QString file_path;
    QString format;
    QString openFile = tr("Add pictures");

    //构造打开条件
    format = "(";
    for (const QString &str : Variable::SUPPORT_FORMATS)
        format += "*." + str + " ";
    format += ");;";
    //所有类型都放在一起，类型名显示不全，故增加分层，将同一类型的放在一起，分层显示
    format = format + Variable::SUPPORT_FORMATS_CLASSIFY;
    file_path = m_fileDialog.getOpenFileName(this, openFile, Variable::getSettings("imagePath").toString(), format);
    if (file_path.isEmpty()) {
        return;
    }

    QPushButton *newPicBtn = new QPushButton(this);
    newPicBtn->setFixedSize(PICTURE_BUTTON);
    newPicBtn->setStyleSheet("QPushButton{border-image:url(" + file_path +");"
                             "border:none;"
                             "border-radius:4px;}");
    int row = 0;
    int col = 0;
    m_list.insert(1, newPicBtn);
    for(int i = 0; i < m_list.size(); i++) {
        m_picBtnLayout->addWidget(m_list[i], row, col++);
        if (col == 3) {
            col = 0;
            row++;
        }
    }
    m_picBtnToPathMap[newPicBtn] = file_path;
    connect(newPicBtn, &QPushButton::clicked, this, [this, newPicBtn]() {
        // 获取按钮的图片路径
        m_saveFlag = false;
        QString path = m_picBtnToPathMap.value(newPicBtn);
        if (!path.isEmpty()) {
            QImage image(path);
            if (!image.isNull()) {
                m_loadingLabel->setText(tr("Background replacement..."));
                m_maskWidget->show();
                Interaction::getInstance()->picMatting(image);
            } else {
                qDebug() << "图片加载失败 === " << path;
                return;
            }
        } else {
            qDebug() << "no path";
        }
    });
}

void ReplaceBackgroundWidget::picReplaceBackground()
{
    m_saveFlag = false;
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    for(QPushButton *btn: m_list){
        if(clickedBtn == btn) {
            QString path = m_picBtnToPathMap.value(clickedBtn);
            if (!path.isEmpty()) {
                QImage image(path);
                if (!image.isNull()) {
                    m_loadingLabel->setText(tr("Background replacement..."));
                    m_maskWidget->show();
                    Interaction::getInstance()->picMatting(image);
                } else {
                    qDebug() << "图片加载失败 === " << path;
                    return;
                }
            } else {
                qDebug() << "no path";
            }
            break;
        }
    }
}

void ReplaceBackgroundWidget::reduceImage()
{
    mattingOperayteMode = MattingOperayteMode::ZoomOut;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    int way = int(mattingOperayteMode);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
}

void ReplaceBackgroundWidget::enlargeImage()
{
    mattingOperayteMode = MattingOperayteMode::ZoomIn;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    int way = int(mattingOperayteMode);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
}

void ReplaceBackgroundWidget::originalSize()
{
    mattingOperayteMode = MattingOperayteMode::LifeSize;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    int way = int(mattingOperayteMode);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    m_proportion = var.toInt();
    setScale(var);
}

void ReplaceBackgroundWidget::adaptiveWidget()
{
    mattingOperayteMode = MattingOperayteMode::Window;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_adaptiveProportion * 0.01));
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    m_proportion = var.toInt();
    setScale(var);
}

void ReplaceBackgroundWidget::rotate()
{
    MattingInteractiveQml::getInstance()->setOperateWay(-1);
    mattingOperayteMode = MattingOperayteMode::RotateS;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    Interaction::getInstance()->mattingRotate();
}

void ReplaceBackgroundWidget::flipH()
{
    MattingInteractiveQml::getInstance()->setOperateWay(-1);
    mattingOperayteMode = MattingOperayteMode::FlipH;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    Interaction::getInstance()->mattingFlipH();
}

void ReplaceBackgroundWidget::flipV()
{
    MattingInteractiveQml::getInstance()->setOperateWay(-1);
    mattingOperayteMode = MattingOperayteMode::FlipV;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    Interaction::getInstance()->mattingFlipV();
}

void ReplaceBackgroundWidget::cutImage()
{
    mattingOperayteMode = MattingOperayteMode::Cut;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    m_rightStackWidget->setCurrentIndex(1);
    m_cutBtnWid->show();
    m_toolWid->hide();

	QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    if (mattingImageItem)
    {
        QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_STARTCUT.toLatin1().data());
        m_isCutting = true;
        m_sizeWid->setCurrentItem(m_lifeSize);
    }
}

void ReplaceBackgroundWidget::toMattingPage()
{
    m_rightStackWidget->setCurrentIndex(0);
    m_cutBtnWid->hide();
    m_toolWid->show();
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    if (mattingImageItem)
    {
        QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_QUITCUT.toLatin1().data());
        m_isCutting = false;
        m_sizeWid->clearSelection();
        m_scaleWid->clearSelection();
    }
}

void ReplaceBackgroundWidget::cutDone()
{
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    QVariant bOK;
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_CHECKCUTSIZE.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, bOK));
    if (!bOK.toBool()){
        QMessageBox msg;
        msg.setText(tr("您未选择截图区域或选择的截图区域超过了图片大小，请重新选择！"));
        msg.setIcon(QMessageBox::Warning);
        msg.exec();
        return;
    }

    QVariant x, y, w, h;
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETCUTINFO_X.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, x));
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETCUTINFO_Y.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, y));
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETCUTINFO_W.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, w));
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETCUTINFO_H.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, h));

    QPoint tmp(x.toDouble(), y.toDouble());
    //备份保存
    Interaction::getInstance()->saveCutImage(m_proportion, tmp,
                                             QPoint(tmp.x()+w.toDouble(),
                                                    tmp.y()+h.toDouble()));

    auto mattingOperayteMode = MattingOperayteMode::NoOperate;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));

    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_QUITCUT.toLatin1().data());
    m_isCutting = false;
    m_sizeWid->clearSelection();
    m_scaleWid->clearSelection();

    m_rightStackWidget->setCurrentIndex(0);
    m_cutBtnWid->hide();
    m_toolWid->show();
}

void ReplaceBackgroundWidget::onSizeItemSelectionChanged()
{
    if (m_isCutting && m_needExecSlot)
    {
        onSizeItemClicked(m_sizeWid->currentItem());
    }
}

void ReplaceBackgroundWidget::onScaleItemSelectionChanged()
{
    if (m_isCutting && m_needExecSlot)
    {
        onScaleItemClicked(m_scaleWid->currentItem());
    }
}

void ReplaceBackgroundWidget::onSizeItemClicked(QListWidgetItem *item)
{
    m_scaleWid->clearSelection();

    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_lastProportion * 0.01));
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
    QVariant vScale;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, vScale));
    setScale(vScale);

    QVariant vRotateto;
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETROTATETO.toLatin1().data(), Q_RETURN_ARG(QVariant, vRotateto));
    bool bNeedTurn = qAbs(vRotateto.toInt() % 360) == 90 ||
            qAbs(vRotateto.toInt() % 360) == 270;

    int width, height, x, y;
    bool canOperate = true;
    bool needCheckScale = true;
    double nBaseWidth = m_replaceImage.width() * m_proportion * 0.01 - 1;
    double nBaseHeight = m_replaceImage.height() * m_proportion * 0.01 - 1;
    if(item == m_lifeSize){
        qDebug() << "原图尺寸";
        if(bNeedTurn)
        {
            height = nBaseWidth;
            width = nBaseHeight;
        }
        else
        {
            width = nBaseWidth;
            height = nBaseHeight;
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
        needCheckScale = false;
    } else if(item == m_freeSize){
        qDebug() << "自由尺寸";
        width = 0;
        height = 0;
        needCheckScale = false;
    } else if(item == m_standardOneInch){
        qDebug() << "标准一寸";
        width = 295*m_proportion/100;
        height = 413*m_proportion/100;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    } else if(item == m_standardTwoInch){
        qDebug() << "标准二寸";
        width = 413*m_proportion/100;
        height = 626*m_proportion/100;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    } else if(item == m_smallOneInch){
        qDebug() << "小一寸";
        width = 259*m_proportion/100;
        height = 377*m_proportion/100;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    } else if(item == m_smallTwoInch){
        qDebug() << "小二寸";
        width = 413*m_proportion/100;
        height = 531*m_proportion/100;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    } else if(item == m_idCardSize){
        qDebug() << "身份证照";
        width = 358*m_proportion/100;
        height = 441*m_proportion/100;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    }

    if (needCheckScale){
        double tempProportion = 1.0;
        if(bNeedTurn)
        {
            if ((nBaseWidth < height) || (nBaseHeight < width))
            {
                tempProportion = height/nBaseWidth;
                if (width/nBaseHeight>tempProportion)
                {
                    tempProportion = width/nBaseHeight;
                }
            }
        }
        else
        {
            if ((nBaseWidth < width) || (nBaseHeight < height))
            {
                tempProportion = width/nBaseWidth;
                if (height/nBaseHeight>tempProportion)
                {
                    tempProportion = height/nBaseHeight;
                }
            }
        }

        if (tempProportion != 1.0)
        {
            QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                      Q_ARG(QVariant, tempProportion * m_proportion * 0.01));
            QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
            QVariant var;
            QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
            setScale(var);
        }
    }

    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_SETCUTSIZE.toLatin1().data(),
                                  Q_ARG(QVariant, width), Q_ARG(QVariant, height),
                                  Q_ARG(QVariant, x), Q_ARG(QVariant, y),
                                  Q_ARG(QVariant, canOperate), Q_ARG(QVariant, 0));
}

void ReplaceBackgroundWidget::onScaleItemClicked(QListWidgetItem *item)
{
    m_sizeWid->clearSelection();

    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_lastProportion * 0.01));
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
    QVariant vScale;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, vScale));
    setScale(vScale);

    QVariant vRotateto;
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_GETROTATETO.toLatin1().data(), Q_RETURN_ARG(QVariant, vRotateto));
    bool bNeedTurn = qAbs(vRotateto.toInt() % 360) == 90 ||
            qAbs(vRotateto.toInt() % 360) == 270;

    double width, height;
    double x, y;
    double nBaseWidth = m_replaceImage.width() * m_proportion * 0.01 - 1;
    double nBaseHeight = m_replaceImage.height() * m_proportion * 0.01 - 1;
    if(item == m_originalScale){
        qDebug() << "原比例";
        if(bNeedTurn)
        {
            if (nBaseWidth > nBaseHeight) {
                width = nBaseHeight;
                height = width*nBaseWidth/nBaseHeight;
            }
            else
            {
                height = nBaseWidth;
                width = height*nBaseHeight/nBaseWidth;
            }
        }
        else
        {
            if (nBaseWidth > nBaseHeight) {
                width = nBaseWidth;
                height = width*nBaseHeight/nBaseWidth;
            }
            else
            {
                height = nBaseHeight;
                width = height*nBaseWidth/nBaseHeight;
            }
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    } else if(item == m_fourToThree){
        qDebug() << "4：3";
        if(bNeedTurn)
        {
            width = nBaseHeight;
            height = width*3/4;
            if (height > nBaseWidth)
            {
                height = nBaseWidth;
                width = height*4/3;
            }
        }
        else
        {
            width = nBaseWidth;
            height = width*3/4;
            if (height > nBaseHeight)
            {
                height = nBaseHeight;
                width = height*4/3;
            }
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;

    } else if(item == m_threeTofour){
        qDebug() << "3：4";
        if(bNeedTurn)
        {
            height = nBaseWidth;
            width = height*3/4;
            if (width > nBaseHeight)
            {
                width = nBaseHeight;
                height = width*4/3;
            }
        }
        else
        {
            height = nBaseHeight;
            width = height*3/4;
            if (width > nBaseWidth)
            {
                width = nBaseWidth;
                height = width*4/3;
            }
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;

    } else if(item == m_sixteenToNine){
        qDebug() << "16：9";
        if(bNeedTurn)
        {
            width = nBaseHeight;
            height = width*9/16;
            if (height > nBaseWidth)
            {
                height = nBaseWidth;
                width = height*16/9;
            }
        }
        else
        {
            width = nBaseWidth;
            height = width*9/16;
            if (height > nBaseHeight)
            {
                height = nBaseHeight;
                width = height*16/9;
            }
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;

    } else if(item == m_nineToSixteen){
        qDebug() << "9：16";
        if(bNeedTurn)
        {
            height = nBaseWidth;
            width = height*9/16;
            if (width > nBaseHeight)
            {
                width = nBaseHeight;
                height = width*16/9;
            }
        }
        else
        {
            height = nBaseHeight;
            width = height*9/16;
            if (width > nBaseWidth)
            {
                width = nBaseWidth;
                height = width*16/9;
            }
        }
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;

    } else if(item == m_oneToOne){
        qDebug() << "1：1";
        width = nBaseWidth > nBaseHeight ? nBaseHeight : nBaseWidth;
        height = width;
        x = m_showImageWidget->width()/2 - width/2;
        y = m_showImageWidget->height()/2 - height/2;
    }

    if (m_qmlObj)
    {
        QObject *qmlMattingImageRect;
        QObject *mattingImageLoader;
        QObject *mattingImageItem;
        QObject *targatImage;
        if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
            return;
        }

        QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_SETCUTSIZE.toLatin1().data(),
                                  Q_ARG(QVariant, width), Q_ARG(QVariant, height),
                                  Q_ARG(QVariant, x), Q_ARG(QVariant, y),
                                  Q_ARG(QVariant, true), Q_ARG(QVariant, width/height));
    }
}

void ReplaceBackgroundWidget::setFormatedText()
{
    // 创建遮罩Widget
    m_maskWidget = new QWidget(m_showImageWidget);
    m_maskWidget->setFixedSize(m_showImageWidget->size());
    m_maskWidget->setStyleSheet("QWidget{background-color: rgba(0, 0, 0, 80);}");
    m_maskWidget->hide();

    //图标+文本wid
    m_loadingWidget = new QWidget(m_maskWidget);
    m_loadingWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_loadingWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_loadingWidget->setFixedSize(QSize(100,64));
    //图标
    m_loadingIcon = new QLabel(m_loadingWidget);
    m_loadingIcon->setFixedSize(QSize(32,32));
    QPixmap pixmap(":/res/res/ai/mattingloading.svg");
    m_loadingIcon->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadingIcon->setFocusPolicy(Qt::NoFocus);
    m_loadingIcon->setAttribute(Qt::WA_TranslucentBackground);
    //文本
    m_loadingLabel = new QLabel(m_loadingWidget);
    m_loadingLabel->setFixedSize(QSize(100,20));
    m_loadingLabel->setStyleSheet("QLabel{background-color:transparent;border:none;color:white;}");

    m_loadingLayout = new QVBoxLayout(m_loadingWidget);
    m_loadingLayout->addWidget(m_loadingIcon, 0, Qt::AlignCenter);
    m_loadingLayout->setSpacing(12);
    m_loadingLayout->addWidget(m_loadingLabel, 0, Qt::AlignCenter);
    m_loadingWidget->setLayout(m_loadingLayout);

    m_loadingWidget->move((m_maskWidget->width() - m_loadingWidget->width()) / 2,
                          (m_maskWidget->height() - m_loadingWidget->height()) / 2);
}

void ReplaceBackgroundWidget::startReplaceBackground()
{
    uint blue, green, red, alpha;
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn != m_customBtn) {
        m_loadingLabel->setText(tr("Background replacement..."));
        m_maskWidget->show();
    }

    if (btn == this->m_transparent) {
        m_saveFlag = true;
    } else {
        m_saveFlag = false;
    }

    if (btn == this->m_transparent){
        m_maskWidget->show();
        Interaction::getInstance()->replaceBackground(255, 255, 255, 0, false);
        return;
    } else if (btn == this->m_white){
        red = 255;
        green = 255;
        blue = 255;
    } else if (btn == this->m_grey){
        red = 190;
        green = 190;
        blue = 190;
    } else if (btn == this->m_dimGrey){
        red = 105;
        green = 105;
        blue = 105;
    } else if (btn == this->m_dark){
        red = 0;
        green = 0;
        blue = 0;
    } else if (btn == this->m_blue){
        red = 0;
        green = 0;
        blue = 255;
    } else if (btn == this->m_red){
        red = 255;
        green = 0;
        blue = 0;
        alpha = 255;
    } else if (btn == this->m_darkOrange){
        red = 255;
        green = 127;
        blue = 0;
    } else if (btn == this->m_orange){
        red = 255;
        green = 165;
        blue = 0;
    } else if (btn == this->m_green){
        red = 0;
        green = 255;
        blue = 0;
    } else if (btn == this->m_purple){
        red = 160;
        green = 32;
        blue = 240;
    } else if (btn == this->m_pink){
        red = 255;
        green = 105;
        blue = 180;
    } else if (btn == this->m_customBtn){
        if (!m_colorDialog) {
            m_colorDialog = new ColorDialog(this);
        }
        m_colorDialog->m_colorLineEdit->setText(m_colorHex);
        m_colorDialog->setStyleSheet("QDialog{border-radius: 6px}");
        m_colorDialog->show();
        connect(m_colorDialog, &ColorDialog::colorSelected, this, &ReplaceBackgroundWidget::colorSelected);
        connect(m_colorDialog, &ColorDialog::colorChanged, this, &ReplaceBackgroundWidget::colorSelected);
        int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
        int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
        QString platform = QGuiApplication::platformName();
        if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            QPoint rightPoint = QPoint((sw - this->width()) / 2 + this->width(), (sh - this->height() - BAR_HEIGHT) / 2 + BAR_HEIGHT);
            int x = rightPoint.x() - m_colorDialog->width() - m_customBtn->width() - 30;
            int y = rightPoint.y() + m_bgLabel1->height() + 10;
            kdk::WindowManager::setGeometry(m_colorDialog->windowHandle(), QRect(QPoint(x, y), QSize(m_colorDialog->width(), m_colorDialog->height())));
        } else {
            QPoint rightPoint = QPoint((sw - this->width()) / 2 + this->width(), (sh - this->height()) / 2 + BAR_HEIGHT);
            int x = rightPoint.x() - m_colorDialog->width() - m_customBtn->width() - 30;
            int y = rightPoint.y() + m_bgLabel1->height() + 10;
            m_colorDialog->move(x, y);
        }
        return;
    }
    Interaction::getInstance()->replaceBackground(blue, green, red, 255, false);
}

void ReplaceBackgroundWidget::colorSelected(QColor color)
{
    m_loadingLabel->setText(tr("Background replacement..."));
    QFileInfo fileInfo(m_origImagePath);
    if (fileInfo.size() / 1024 >= 10240) // 大于10M的图片，才显示mask提示
    {
        m_maskWidget->show();
    }

    m_colorHex = m_colorDialog->m_colorLineEdit->text();

    QString labelQss = QString("QLabel{background: %1;"
                               "border-top-right-radius:4px;"
                               "border-bottom-right-radius:4px}").arg(color.name());
    m_customBtnRight->setStyleSheet(labelQss);

    uint blue, green, red;
    blue = color.blue();
    green = color.green();
    red = color.red();
    if (!m_isReplacing)
    {
        Interaction::getInstance()->replaceBackground(blue, green, red, 255, false);
        m_isReplacing = true;
    }
}

void ReplaceBackgroundWidget::initQmlObject()
{
    m_showMattingImageQml = new QQuickWidget(m_showImageWidget);
    QQmlEngine *engine = m_showMattingImageQml->engine();
    m_showMattingImage = new ShowMattingImage(this);
    engine->rootContext()->setContextProperty("MattingImage", m_showMattingImage);
    engine->addImageProvider(QLatin1String("MattingImg"), m_showMattingImage->getImgProvider());
    qmlRegisterType<MattingInteractiveQml>("MattingInteractive", 1, 0, "MattingInteractive");
    m_showMattingImageQml->setSource(QUrl(QStringLiteral("qrc:/matting/Matting.qml")));
    m_showMattingImageQml->setClearColor(Qt::transparent);
    m_showMattingImageQml->installEventFilter(this);
    m_showMattingImageQml->lower();
    m_showMattingImageQml->show();

    m_qmlObj = new QObject(this);
    m_qmlObj = m_showMattingImageQml->rootObject();
    //操作方式
    connect(MattingInteractiveQml::getInstance(), SIGNAL(operateWayChanged(QVariant)), m_qmlObj,
            SLOT(setOperateWay(QVariant)));
    connect(m_qmlObj, SIGNAL(closeChildWindow()), this, SLOT(onCloseChildWindow()));

    changeQmlSizePos(m_showImageWidget->width(), m_showImageWidget->height(), 0, 0);
}
void ReplaceBackgroundWidget::initQWidgetObject()
{
    m_showImage = new QLabel(m_showImageWidget);
    m_showImage->setFixedSize(830, 680);
    m_showImage->hide();
}

void ReplaceBackgroundWidget::qmlAndWidgetConnect(QObject *obj)
{
    connect(obj, SIGNAL(sendScale(QVariant)), this, SLOT(setScale(QVariant)));
    connect(obj, SIGNAL(changeOperateWay()), this, SLOT(needChangeOperateWay()));
    connect(obj, SIGNAL(cutFreeSize()), this, SLOT(onCutFreeSize()));
    connect(obj, SIGNAL(closeChildWindow()), this, SLOT(onCloseChildWindow()));
}

void ReplaceBackgroundWidget::setScale(QVariant scale)
{
    m_proportion = scale.toInt();
    recordMattingCurrentProportion = m_proportion;
    m_percentage->setText(QString::number(m_proportion) + "%");
}

void ReplaceBackgroundWidget::changeQmlCutSelectionPos()
{
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }

    QMetaObject::invokeMethod(mattingImageItem,
                              MATTINGIMAGE_FUNC_SETCUTPOS.toLatin1().data());
}

void ReplaceBackgroundWidget::changeQmlSizePos(int w, int h, int x, int y)
{
    m_qmlObj->setProperty(QString("width").toLatin1().data(), w);
    m_qmlObj->setProperty(QString("height").toLatin1().data(), h);
    m_qmlObj->setProperty(QString("x").toLatin1().data(), x);
    m_qmlObj->setProperty(QString("y").toLatin1().data(), y);
}

void ReplaceBackgroundWidget::changeQmlColor(QColor color)
{
    m_qmlObj->setProperty(QString("color").toLatin1().data(), color);
}

void ReplaceBackgroundWidget::closeColorDialog()
{
    if (m_colorDialog)
    {
        m_colorDialog->close();
    }
}

bool ReplaceBackgroundWidget::getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, QObject **objSecond,
                                                       QObject **objThird, QObject **objFourth)
{
    if (objParent == nullptr) {
        qDebug() << "父对象为空";
        return false;
    }
    QObject *tmpFirst = objParent->findChild<QObject *>("mattingImageItem");
    if (tmpFirst == nullptr) {
        qDebug() << "mattingImageItem对象为空";
        return false;
    }
    *objFirst = tmpFirst;

    QObject *tmpSecond = tmpFirst->findChild<QObject *>("mattingImageLoader");
    if (tmpSecond == nullptr) {
        qDebug() << "mattingImageLoader对象为空";
        return false;
    }
    *objSecond = tmpSecond;

    QObject *tmpThird = tmpSecond->findChild<QObject *>("showMattingImageItem");
    if (tmpThird == nullptr) {
        qDebug() << "showMattingImageItem对象为空";
        return false;
    }
    *objThird = tmpThird;

    QObject *tmpFourth = tmpThird->findChild<QObject *>("mattingImage");
    if (tmpFourth == nullptr) {
        qDebug() << "mattingImage对象为空";
        return false;
    }

    if (objFourth != nullptr) {
        *objFourth = tmpFourth;
    }
    return true;
}

void ReplaceBackgroundWidget::getReplaceAndOrigImage(QImage replaceImage, QImage origImage)
{
    mattingOperayteMode = MattingOperayteMode::ChangeImage;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
//    mattingTempImage = replaceImage;
    m_replaceImage = replaceImage;
    m_origImage = origImage;
    creatImage(-1, true);
    m_isReplacing = false;
}

void ReplaceBackgroundWidget::getFlipAndOrigImage(QImage flipImage, QImage origImage)
{
    //重设缩放比显示
    int defaultProportion = 100 * m_showImageWidSize.width() / flipImage.width();
    if (flipImage.height() * defaultProportion / 100 > m_showImageWidSize.height()) {
        defaultProportion = 100 * m_showImageWidSize.height() / flipImage.height();
    }
    int tempProportion = 100;
    if (defaultProportion >= 100) {
        tempProportion = 100;
    } else {
        tempProportion = defaultProportion;
    }
    m_lastProportion = tempProportion;
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, tempProportion * 0.01));
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
}

void ReplaceBackgroundWidget::creatImage(const int &proportion, bool defaultOpen)
{
    m_showImageWidSize = m_showImageWidget->size();
    QImage tempImg = m_replaceImage;
    int defaultProportion = 100 * m_showImageWidSize.width() / tempImg.width();
    if (tempImg.height() * defaultProportion / 100 > m_showImageWidSize.height()) {
        defaultProportion = 100 * m_showImageWidSize.height() / tempImg.height();
    }
    if (defaultOpen == false) {
        operateImage(proportion, defaultProportion);
    } else {
        defaultImage(proportion, defaultProportion);
    }
}

void ReplaceBackgroundWidget::defaultImage(int proportion, int defaultProportion)
{
    //自适应窗口大小显示
    if (proportion <= 0) {
        //计算缩放比
        if (defaultProportion >= 100) {
            m_proportion = 100;
            m_adaptiveProportion = 100;
        } else {
            m_proportion = defaultProportion;
            m_adaptiveProportion = defaultProportion;
        }
        //计算缩放后尺寸
        m_tmpImageSize = m_replaceImage.size() * m_proportion / 100;
    }
    showImage();
}

void ReplaceBackgroundWidget::operateImage(int proportion, int defaultProportion)
{
    //计算缩放比
    if (proportion <= 0) {
        if (defaultProportion > 1000) {
            m_proportion = 1000;
        } else {
            m_proportion = defaultProportion;
        }
    } else {
        m_proportion = proportion;
    }
    //计算缩放后尺寸
    m_tmpImageSize = m_replaceImage.size() * m_proportion / 100;
    //极小的图缩放后尺寸低于1的不处理
    if (judgeSizeIsZero(m_tmpImageSize)) {
        return;
    }
    showImage();
}

bool ReplaceBackgroundWidget::judgeSizeIsZero(QSize size)
{
    if (size.width() * size.height() == 0) {
        return true;
    }
    return false;
}

void ReplaceBackgroundWidget::showImage()
{
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    this->qmlAndWidgetConnect(mattingImageItem);
    m_showMattingImage->setImage(m_replaceImage);
    //判断传入方式进行缩放显示
    int way = int(mattingOperayteMode);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
    //适应窗口
    if (mattingOperayteMode == MattingOperayteMode::NormalMode || mattingOperayteMode == MattingOperayteMode::Window) {
        QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
        QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_POSRESET.toLatin1().data());
    }
    //需要设置qml操作的方式
    if (mattingOperayteMode == MattingOperayteMode::ChangeImage || mattingOperayteMode == MattingOperayteMode::LifeSize
        || mattingOperayteMode == MattingOperayteMode::RotateN || mattingOperayteMode == MattingOperayteMode::RotateS
        || mattingOperayteMode == MattingOperayteMode::FlipH || mattingOperayteMode == MattingOperayteMode::FlipV
        || mattingOperayteMode == MattingOperayteMode::NoOperate) {
        QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    }
    m_showImage->hide();
    m_maskWidget->hide();
    m_showMattingImageQml->show();
    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    m_proportion = var.toInt();
    m_lastProportion = m_proportion;
    setScale(var);
}

void ReplaceBackgroundWidget::comparePressed()
{
    QImage tempImg = m_origImage;
    int defaultProportion = 100 * m_showImageWidSize.width() / tempImg.width();
    if (tempImg.height() * defaultProportion / 100 > m_showImageWidSize.height()) {
        defaultProportion = 100 * m_showImageWidSize.height() / tempImg.height();
    }
    //计算自适应窗口大小显示缩放比
    int tempProportion = 100;
    if (defaultProportion >= 100) {
        tempProportion = 100;
    } else {
        tempProportion = defaultProportion;
    }
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_RECORDCURRENTFLIP.toLatin1().data());
    mattingOperayteMode = MattingOperayteMode::ChangeImage;
    MattingInteractiveQml::getInstance()->setOperateWay(int(mattingOperayteMode));
    m_showMattingImage->setImage(m_origImage);
    int way = int(mattingOperayteMode);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, tempProportion * 0.01));
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
}

void ReplaceBackgroundWidget::compareReleased()
{
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    m_showMattingImage->setImage(m_replaceImage);
    QMetaObject::invokeMethod(mattingImageItem, MATTINGIMAGE_FUNC_SETCURRENTFLIP.toLatin1().data());
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_lastProportion * 0.01));
    QVariant var;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
}

void ReplaceBackgroundWidget::needChangeOperateWay()
{
    QObject *qmlMattingImageRect;
    QObject *mattingImageLoader;
    QObject *mattingImageItem;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlMattingImageRect, &mattingImageLoader, &mattingImageItem, &targatImage)) {
        return;
    }
    //获取当前图片显示方式（1——放大/2——缩小/3——自适应）
    QVariant way;
    QMetaObject::invokeMethod(targatImage, MATTINGIMAGE_FUNC_GETOPERATEWAY.toLatin1().data(), Q_RETURN_ARG(QVariant, way));
    m_imageShowWay = way.toInt();
    if(m_imageShowWay == 1){
        mattingOperayteMode = MattingOperayteMode::ZoomIn;
    }
    if(m_imageShowWay == 2){
        mattingOperayteMode = MattingOperayteMode::ZoomOut;
    }
}

void ReplaceBackgroundWidget::onCutFreeSize()
{
    m_needExecSlot = false;
    m_sizeWid->setCurrentItem(m_freeSize);
    m_needExecSlot = true;
}

void ReplaceBackgroundWidget::needSave()
{
    QString format;
    //构造另存为文件类型
    if (m_saveFlag) {
        format = "PNG(*.png)";
    } else {
        format = Variable::SUPPORT_FORMATS_SAVEAS;
    }
    QFileDialog fileDialog(this, Qt::CustomizeWindowHint); //设置窗口标题属性自定义
    QFileInfo filename(m_origImagePath);
    QString savePath = fileDialog.getSaveFileName(nullptr, QString(), m_origImagePath + "/" +
                                                  filename.completeBaseName() + ".png", format);
    QFileInfo file(savePath);
    if (savePath.isNull()) {
        return;
    }
    if (file.fileName().indexOf(".") == 0) {
        //不能以.命名
        m_msg->warning(this, tr("Warning"), tr("save fail.name cannot begin with \".\" "));
        return;
    }
    for (int i = 0; i < SPECIALCHAR.length(); i++) {
        if (file.fileName().contains(SPECIALCHAR.at(i))) {
            m_msg->warning(this, tr("Warning"), tr("the file name is illegal"));
            return;
        }
    }
    if (file.fileName() == "") {
        m_msg->warning(this, tr("Warning"), tr("the file name is illegal"));
        return;
    }
    //如果没有后缀，或者后缀不在list里，就用原格式为后缀
    if (!Variable::SUPPORT_FORMATS_END.contains("." + file.suffix().toLower())) {
        if (Variable::SUPPORT_FORMATS_NOT.contains("." + file.suffix().toLower())) {
            savePath += +".png";
        } else {
            savePath += + "." + m_origFormat.toLower();
        }
    }
    Interaction::getInstance()->needSaveReplaceImage(savePath);
    this->exitReplaceBackground();
}

void ReplaceBackgroundWidget::exitReplaceBackground()
{
    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }
    targatImage->setProperty(QString("source").toLatin1().data(), "");
    m_percentage->clear();
    m_colorHex = "#000000";
    m_customBtnRight->setStyleSheet("QLabel{border-image: url(:/res/res/ai/transparentselector.svg);"
                                    "background-color: transparent;"
                                    "border: none;"
                                    "border-top-right-radius:4px;"
                                    "border-bottom-right-radius:4px;"
                                    "}");
    if(m_colorDialog) {
        m_colorDialog->close();
        delete m_colorDialog;
        m_colorDialog = nullptr;
    }
    Q_EMIT toBackShowImageWid();
}

void ReplaceBackgroundWidget::reMove(int W, int H)
{
    this->resize(W, H);

    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }
    QMetaObject::invokeMethod(imageItemLower, MATTINGIMAGE_FUNC_ADJUSTIMAGEPOS.toLatin1().data());
    QMetaObject::invokeMethod(imageItemLower, MATTINGIMAGE_FUNC_UPDATEPOSACCORDWIDSIZECHANGE.toLatin1().data());
}

void ReplaceBackgroundWidget::resizeEvent(QResizeEvent *event)
{
    m_rightStackWidget->setFixedSize(250, this->height());
    //m_rightStackWidget->setMinimumWidth(226);
    m_showImageWidget->setFixedSize(this->width() - m_rightStackWidget->width(), this->height());
    m_showImage->setFixedSize(m_showImageWidget->size());
    changeQmlSizePos(m_showImageWidget->width(), m_showImageWidget->height(), 0, 0);
    changeQmlCutSelectionPos();

    m_scrollArea->setFixedHeight(m_rightStackWidget->height() - m_mattingBottomWidget->height() -
                                 m_mattingMidWidget->height() - m_mattingTopWidget->height());

    m_maskWidget->setFixedSize(m_showImageWidget->size());
    m_loadingWidget->move((m_maskWidget->width() - m_loadingWidget->width()) / 2,
                          (m_maskWidget->height() - m_loadingWidget->height()) / 2);

    m_toolWid->move(int((m_showImageWidget->width() - m_toolWid->width()) / 2),
                    m_showImageWidget->height() - m_toolWid->height() - 6);
    m_cutBtnWid->move(int((m_showImageWidget->width() - m_cutBtnWid->width()) / 2),
                      m_showImageWidget->height() - m_cutBtnWid->height() - 6);
}

bool ReplaceBackgroundWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void ReplaceBackgroundWidget::mousePressEvent(QMouseEvent *event)
{
    if(m_colorDialog && m_colorDialog->isVisible()) {
        m_colorDialog->hide();
    }
}

void ReplaceBackgroundWidget::onCloseChildWindow()
{
    if(m_colorDialog && m_colorDialog->isVisible()) {
        m_colorDialog->hide();
    }
}

void ReplaceBackgroundWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (m_isCutting){
            cutDone();
            return;
        }
    }
    else if (event->key() == Qt::Key_Escape) {
        if (m_isCutting){
            toMattingPage();
            return;
        }
    }

    QWidget::keyPressEvent(event);
}
