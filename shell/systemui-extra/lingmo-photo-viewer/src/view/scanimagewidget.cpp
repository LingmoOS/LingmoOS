#include "scanimagewidget.h"

ScanImageWidget::ScanImageWidget(QWidget *parent) : QWidget(parent)
{
    //初始化

    this->initComponents();
    this->initLayout();
    this->platformType();

    this->initQmlObject();
    this->initPrintWidget();
    this->initMaskWidget();

    this->initGsetting();
    this->initConnect();

    //绘制阴影
    this->setShadowEffect(m_compareWidget);
    this->setShadowEffect(m_toolBtnWid);
    this->setShadowEffect(m_cutBtnWid);

    //毛玻璃
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    //水印内容
    lastwmcontent = tr("Piracy must be investigated");
}

void ScanImageWidget::setShadowEffect(QWidget *widget)
{
    QGraphicsDropShadowEffect *effect= new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);
    effect->setColor(TOOL_COLOR);
    effect->setBlurRadius(BLUR_RADIUS);
    widget->setGraphicsEffect(effect);
}

void ScanImageWidget::initComponents()
{
    //窗口和布局
    m_layout = new QVBoxLayout(this);

    m_toolWid = new QFrame(this);
    m_toolLayout = new QHBoxLayout(m_toolWid);

    m_toolBtnWid = new QWidget(this);
    m_toolBtnLayout = new QHBoxLayout(m_toolBtnWid);

    m_zoomWid = new QWidget(this);
    m_saveWid = new QWidget(m_toolWid);
    m_saveLayout = new QHBoxLayout(m_saveWid);

    //工具栏按钮 - 对比
    m_compareWidget = new QWidget(this);
    m_compare = new kdk::KBorderlessButton(m_compareWidget);
    m_compare->setFocusPolicy(Qt::NoFocus);
    m_compare->setText(tr("Contrast"));
    m_compare->setToolTip(tr("Long press to view the original image"));

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

    m_line1 = new QLabel(this);
    m_line1->setFixedSize(TOOL_LINE);
    //水印
    m_watermark = new kdk::KBorderlessButton(this);
    m_watermark->setFocusPolicy(Qt::NoFocus);
    m_watermark->setAttribute(Qt::WA_TranslucentBackground);
    m_watermark->setToolTip(tr("Add waterMark"));
    //裁剪
    m_cutImage = new kdk::KBorderlessButton(this);
    m_cutImage->setFocusPolicy(Qt::NoFocus);
    m_cutImage->setAttribute(Qt::WA_TranslucentBackground);
    m_cutImage->setToolTip(tr("Crop"));
    //打印
    m_print = new kdk::KBorderlessButton(this);
    m_print->setFocusPolicy(Qt::NoFocus);
    m_print->setAttribute(Qt::WA_TranslucentBackground);
    m_print->setToolTip(tr("Print"));

    m_line2 = new QLabel(this);
    m_line2->setFixedSize(TOOL_LINE);
    //取消
    m_cancelBtn = new QPushButton(this);
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn->setAttribute(Qt::WA_TranslucentBackground);
    m_cancelBtn->setToolTip(tr("cancel"));

    //保存
    m_saveBtn = new QPushButton(m_saveWid);
    m_saveBtn->setText(tr("Save"));
    m_saveBtn->setFocusPolicy(Qt::NoFocus);
    m_saveBtn->setAttribute(Qt::WA_TranslucentBackground);
    //分割线
    m_line3 = new QLabel(m_saveWid);
    //保存为PDF
    m_savePDF = new QAction();
    m_savePDF->setText(tr("Save as PDF"));
    //保存为图片
    m_saveImage = new QAction();
    m_saveImage->setText(tr("Save as image"));
    //下拉菜单
    m_pullDownBtn = new QToolButton(m_saveWid);
    m_pullDownBtn->setFixedSize(QSize(16,16));
    m_pullDownBtn->setFocusPolicy(Qt::NoFocus);
    m_pullDownBtn->setAttribute(Qt::WA_TranslucentBackground);
    m_saveMenu = new QMenu();
    m_saveMenu->setFocusPolicy(Qt::NoFocus);
    m_saveMenu->addAction(m_savePDF);
    m_saveMenu->addAction(m_saveImage);

    this->setBtnIcon();
    this->toolBtnIconSize(TOOL_BUTTON);

    //裁剪工具栏
    this->initCutWidget();
}

void ScanImageWidget::initLayout()
{
    //工具栏 - 保存按钮区域
    m_saveLayout->addWidget(m_saveBtn, 0, Qt::AlignCenter);
    m_saveLayout->addWidget(m_line3, 0, Qt::AlignCenter);
    m_saveLayout->setSpacing(0);
    m_saveLayout->addWidget(m_pullDownBtn, 0, Qt::AlignCenter);
    m_saveLayout->setContentsMargins(0, 5, 0, 5);
    //工具栏
    m_toolBtnLayout->addWidget(m_zoomWid, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_adaptiveWidget, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_originalSize, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_rotate, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_flipV, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_flipH, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_line1, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_watermark, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_cutImage, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_print, 0, Qt::AlignCenter);
    m_toolBtnLayout->addWidget(m_line2, 0, Qt::AlignCenter);

    QSpacerItem *spacerItem = new QSpacerItem(TOOL_BUTTON.width()+20, TOOL_BUTTON.height(),  QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_toolBtnLayout->addSpacerItem(spacerItem);

    m_toolBtnLayout->addWidget(m_saveWid, 0, Qt::AlignCenter);
    m_toolBtnLayout->setSpacing(20);
    m_toolBtnWid->setLayout(m_toolBtnLayout);
    m_toolBtnLayout->setContentsMargins(6, 6, 6, 6);

    m_toolLayout->addWidget(m_compareWidget);
    m_toolLayout->addWidget(m_toolBtnWid);
    m_toolLayout->setSpacing(10);
    m_toolWid->setLayout(m_toolLayout);
    //裁剪工具栏
    m_cutBtnLayout->addWidget(m_cutCancel, 0, Qt::AlignCenter);
    m_cutBtnLayout->setSpacing(8);
    m_cutBtnLayout->addWidget(m_cutSave, 0, Qt::AlignCenter);
    m_cutBtnWid->setLayout(m_cutBtnLayout);
}

void ScanImageWidget::initConnect()
{
    //工具栏
    connect(m_compare, &kdk::KBorderlessButton::pressed, this, &ScanImageWidget::comparePressed);
    connect(m_compare, &kdk::KBorderlessButton::released, this,&ScanImageWidget::compareReleased);
    connect(m_reduce, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::reduceImage);
    connect(m_enlarge, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::enlargeImage);
    connect(m_rotate, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::rotate);
    connect(m_originalSize, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::originalSize);
    connect(m_adaptiveWidget, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::adaptiveWidget);
    connect(m_flipH, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::flipH);
    connect(m_flipV, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::flipV);
    connect(m_watermark, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::addWatermark);
    connect(m_cutImage, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::cutImage);
    connect(m_print, &kdk::KBorderlessButton::clicked, this, &ScanImageWidget::print);          //打印
    //取消和保存
    connect(m_cancelBtn, &QPushButton::clicked, this, &ScanImageWidget::exitScanImage);
    connect(m_saveBtn, &QPushButton::clicked, this, &ScanImageWidget::needSave);
    connect(m_pullDownBtn, &QToolButton::clicked, this, &ScanImageWidget::showSaveMenu);
    connect(m_savePDF, &QAction::triggered, this, &ScanImageWidget::saveAsPDF);
    connect(m_saveImage, &QAction::triggered, this, &ScanImageWidget::needSave);

    //PDF保存界面
    connect(m_sizeA4Btn, &QRadioButton::toggled, this, &ScanImageWidget::printSize);
    connect(m_direVBtn, &QRadioButton::toggled, this, &ScanImageWidget::printDire);
    connect(m_cancelPrintBtn, &QPushButton::clicked, this, &ScanImageWidget::cancelPrint);
    connect(m_confirmPrintBtn, &QPushButton::clicked, this, &ScanImageWidget::confirmPrint);    //PDF保存
    connect(m_printDialog, &QDialog::rejected, this,&ScanImageWidget::cancelPrint);
    //返回结果
    connect(Interaction::getInstance(), &Interaction::returnScanAndOrigImage, this, &ScanImageWidget::getScanAndOrigImage);
    connect(Interaction::getInstance(), &Interaction::returnScannerFlipAndOrigImage, this, &ScanImageWidget::getFlipAndOrigImage);
    connect(Interaction::getInstance(), &Interaction::returnScannerSaveResult, this, &ScanImageWidget::saveResult);
    //裁剪
    connect(m_cutCancel, &QPushButton::clicked, this, &ScanImageWidget::exitCut);
    connect(m_cutSave, &QPushButton::clicked, this, &ScanImageWidget::needSaveCut);
}

void ScanImageWidget::setBtnIcon()
{
    m_compare->setIcon(QIcon(":/res/res/ai/compare.svg"));
    m_reduce->setIcon(QIcon(":/res/res/sp1/reduce.svg"));
    m_enlarge->setIcon(QIcon(":/res/res/sp1/enlarge.svg"));
    m_originalSize->setIcon(QIcon(":/res/res/sp1/originalSize.svg"));
    m_adaptiveWidget->setIcon(QIcon(":/res/res/sp1/adaptiveWidget.svg"));
    m_rotate->setIcon(QIcon(":/res/res/sp1/rotate.svg"));
    m_flipH->setIcon(QIcon(":/res/res/sp1/flipH.svg"));
    m_flipV->setIcon(QIcon(":/res/res/sp1/flipV.svg"));

    m_watermark->setIcon(QIcon(":/res/res/ai/watermark.svg"));
    m_cutImage->setIcon(QIcon(":/res/res/ai/cutImage.svg"));
    m_print->setIcon(QIcon(":/res/res/ai/print.svg"));

    m_cancelBtn->setIcon(QIcon(":/res/res/cancel.svg"));
    m_pullDownBtn->setIcon(QIcon(":/res/res/ai/arrowdown-white.svg"));
}

void ScanImageWidget::toolBtnIconSize(QSize size)
{
    m_compare->setIconSize(size);
    m_reduce->setIconSize(size);
    m_enlarge->setIconSize(size);
    m_originalSize->setIconSize(size);
    m_adaptiveWidget->setIconSize(size);
    m_rotate->setIconSize(size);
    m_flipH->setIconSize(size);
    m_flipV->setIconSize(size);

    m_watermark->setIconSize(size);
    m_cutImage->setIconSize(size);
    m_print->setIconSize(size);
    m_cancelBtn->setIconSize(size);
}

void ScanImageWidget::toolButtonSize(QSize size)
{

    m_compare->setMinimumSize(size);

    m_reduce->setFixedSize(size);
    m_enlarge->setFixedSize(size);
    m_originalSize->setFixedSize(size);
    m_adaptiveWidget->setFixedSize(size);
    m_rotate->setFixedSize(size);
    m_flipH->setFixedSize(size);
    m_flipV->setFixedSize(size);

    m_watermark->setFixedSize(size);
    m_cutImage->setFixedSize(size);
    m_print->setFixedSize(size);
    m_cancelBtn->setFixedSize(size);

}

void ScanImageWidget::platformType()
{
//    normalIconPath = "sp1";
    if (platForm.contains(Variable::platForm)) {
        m_toolBtnWid->resize(TOOLBAR_SIZE_INTEL);
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE_INTEL);
        widRadius = "12px";
        this->toolButtonSize(TOOL_BUTTON_INTEL);
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5,
                           (m_reduce->height() - m_percentage->height()) / 2 - 3);
    } else {
        m_toolWid->setFixedSize(810, 54);
        m_toolBtnWid->setFixedSize(QSize(700, 44));
        m_compareWidget->setFixedSize(QSize(80, 44));
        m_compare->setFixedSize(QSize(80, 44));
        m_saveWid->setFixedSize(QSize(85, 32));
        m_saveBtn->setFixedSize(QSize(68, 22));
        m_line3->setFixedSize(QSize(1, 20));
        m_zoomWid->setFixedSize(TOOLZOOM_SIZE);
        widRadius = "6px";
        this->toolButtonSize(TOOL_BUTTON);
        m_percentage->setFixedSize(TOOL_PER);
        m_percentage->move(m_reduce->x() + m_reduce->width() + 5, 0);
    }
    m_enlarge->move(m_zoomWid->width() - m_enlarge->width(), 0);
}

void ScanImageWidget::initGsetting()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        changeStyle();
    });
    changeStyle();
    return;
}

void ScanImageWidget::changeStyle()
{
    nowThemeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("lingmo-dark" == nowThemeStyle || "lingmo-black" == nowThemeStyle) {
        m_toolBtnWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.66);border-radius:" + widRadius+ ";border:none;}");
        m_compareWidget->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.66);border:none;border-radius:" + widRadius + ";}");

        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(255,255,255);}"
                                    "QLabel:disabled{background-color:transparent;color:rgb(80,80,80);}");

        m_line1->setStyleSheet("QLabel{border: 1px solid #727272;}"
                               "QLabel:disabled {border: 1px solid #505050;}");
        m_line2->setStyleSheet("QLabel{border: 1px solid #727272;"
                                "QLabel:disabled {border: 1px solid #505050;}");
    } else {
        m_toolBtnWid->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);border:none;border-radius:" + widRadius + ";}");
        m_compareWidget->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);border:none;border-radius:" + widRadius + ";}");
        m_cancelBtn->setStyleSheet("QPushButton{background-color:rgba(255,255,255,1);border:none;border-radius:" + widRadius + ";}");
        m_zoomWid->setStyleSheet("QWidget{background-color:transparent;border:none;}");
        m_percentage->setStyleSheet("QLabel{background-color:transparent;color:rgb(0,0,0);}"
                                    "QLabel:disabled {background-color:transparent;color: rgb(200, 200, 200);}");

        m_line1->setStyleSheet("QLabel{border: 1px solid #393939;}"
                               "QLabel:disabled {border: 1px solid #AAAAAA;}");
        m_line2->setStyleSheet("QLabel{border: 1px solid #393939;}"
                               "QLabel:disabled {border: 1px solid #AAAAAA;}");
    }
    m_saveWid->setStyleSheet("QWidget{border:none;border-radius:5px;background:rgba(29, 114, 242, 1);color:white;}"
                             "QWidget::pressed{border:none;border-radius:5px;background:rgba(29, 114, 242, 1);color:white;}" //#0062F0
                             "QWidget:disabled{border:none;border-radius:5px;background:#8099C6;color:#cccccc;}");
    m_saveBtn->setStyleSheet("QPushButton { border:none; padding-top: 2px; padding-bottom: 3px; }");
    m_line3->setStyleSheet("QLabel{border: 1px solid rgba(255,255,255,0.2)}"
                           "QLabel:disabled {border: 1px solid rgba(192,192,192,0.2);}");
}

void ScanImageWidget::initMaskWidget()
{
    //创建遮罩Widget
    m_maskWidget = new QWidget(this);
    m_maskWidget->resize(m_showImageQml->size());
    m_maskWidget->setStyleSheet("QWidget{background-color: rgba(0, 0, 0, 80);}");
    m_maskWidget->hide();
    //图标
    m_loadingIco = new QLabel(m_maskWidget);
    m_loadingIco->setFixedSize(QSize(32,32));
    QPixmap pixmap(":/res/res/ai/scanning.svg");
    m_loadingIco->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadingIco->setFocusPolicy(Qt::NoFocus);
    m_loadingIco->setAttribute(Qt::WA_TranslucentBackground);
    //文本
    m_loadingText = new QLabel(m_maskWidget);
    m_loadingText->setFixedSize(QSize(82,20));
    m_loadingText->setText(tr("Intelligent scanning in progress..."));
    m_loadingText->setStyleSheet("QLabel{background-color:transparent;border:none;color: white;}");
    //图标+文本
    m_loadWidget = new QWidget(m_maskWidget);
    m_loadWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_loadWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_loadWidget->setFixedSize(QSize(82,64));
    //布局
    QVBoxLayout *layout = new QVBoxLayout(m_loadWidget);
    layout->addWidget(m_loadingIco,0, Qt::AlignCenter);
    layout->addSpacing(12);
    layout->addWidget(m_loadingText,0, Qt::AlignCenter);
}

void ScanImageWidget::initQmlObject()
{
    m_showImageQml = new QQuickWidget(this);
    QQmlEngine *engine = m_showImageQml->engine();
    m_ScannerImageProvider = new ScannerImageProvider();
    engine->addImageProvider(QLatin1String("scannerImage"), m_ScannerImageProvider);
    qmlRegisterType<ScannerInteractiveQml>("ScannerInteractive", 1, 0, "ScannerInteractive");

    m_showImageQml->setSource(QUrl(QStringLiteral("qrc:/scanner/Scanner.qml")));
    m_showImageQml->setClearColor(Qt::transparent);
    m_showImageQml->installEventFilter(this);
    m_showImageQml->lower();

    m_qmlObj = new QObject(this);
    m_qmlObj = m_showImageQml->rootObject();
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlScannerImageRect, &scannerImageLoader, &scannerImageItem, &targatImage)) {
        return;
    }

    //操作方式
    connect(ScannerInteractiveQml::getInstance(), SIGNAL(operateWayChanged(QVariant)), m_qmlObj,
            SLOT(setOperateWay(QVariant)));

    changeQmlSizePos(m_showImageQml->width(), m_showImageQml->height(), 0, 0);
    changeQmlColor();
}

void ScanImageWidget::zoomToChangeWid()
{
    if(!m_isEnlarged){
        sizeChanged( QSize(this->width(),this->height()),0,0);
        m_isEnlarged = true;
    }
}

bool ScanImageWidget::getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, QObject **objSecond, QObject **objThird, QObject **objFourth)
{
    if (objParent == nullptr) {
        qDebug() << "父对象为空";
        return false;
    }
    QObject *tmpFirst = objParent->findChild<QObject *>("scannerImageItem");
    if (tmpFirst == nullptr) {
        qDebug() << "scannerImageItem对象为空";
        return false;
    }
    *objFirst = tmpFirst;

    QObject *tmpSecond = tmpFirst->findChild<QObject *>("scannerImageLoader");
    if (tmpSecond == nullptr) {
        qDebug() << "scannerImageLoader对象为空";
        return false;
    }
    *objSecond = tmpSecond;

    QObject *tmpThird = tmpSecond->findChild<QObject *>("showScannerImageItem");
    if (tmpThird == nullptr) {
        qDebug() << "showScannerImageItem对象为空";
        return false;
    }
    *objThird = tmpThird;

    QObject *tmpFourth = tmpThird->findChild<QObject *>("scannerImage");
    if (tmpFourth == nullptr) {
        qDebug() << "scannerImage对象为空";
        return false;
    }

    if (objFourth != nullptr) {
        *objFourth = tmpFourth;
    }
    return true;
}

void ScanImageWidget::qmlAndWidgetConnect(QObject *obj)
{
    connect(obj, SIGNAL(sendScale(QVariant)), this, SLOT(setScale(QVariant)));
    connect(obj, SIGNAL(changeOperateWay()), this, SLOT(needChangeOperateWay()));
    connect(obj, SIGNAL(sigWheelZoom()), this, SLOT(zoomToChangeWid()));
}

void ScanImageWidget::creatImage(const int &proportion, bool defaultOpen)
{
    if(m_tempImg.isNull()){
        return;
    }
    int defaultProportion = 100 * m_showImageWidSize.width() / m_tempImg.width();
    if (m_tempImg.height() * defaultProportion / 100 > m_showImageWidSize.height()) {
        defaultProportion = 100 * m_showImageWidSize.height() / m_tempImg.height();
    }
    if (defaultOpen == false) {
        operateImage(proportion, defaultProportion);
    } else {
        defaultImage(proportion, defaultProportion);
    }

    showImage();
}

void ScanImageWidget::defaultImage(int proportion, int defaultProportion)
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
        m_tmpImageSize = m_tempImg.size() * m_proportion / 100;
    }
}

void ScanImageWidget::operateImage(int proportion, int defaultProportion)
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
    m_tmpImageSize = m_tempImg.size() * m_proportion / 100;
    //极小的图缩放后尺寸低于1的不处理
    if (judgeSizeIsZero(m_tmpImageSize)) {
        return;
    }
}

bool ScanImageWidget::judgeSizeIsZero(QSize size)
{
    if (size.width() * size.height() == 0) {
        return true;
    }
    return false;
}

void ScanImageWidget::showImage()
{
    this->qmlAndWidgetConnect(scannerImageItem);
    //设置图片
    targatImage->setProperty(QString("source").toLatin1().data(), "");
    if(isWatermark){
        m_ScannerImageProvider->setWarkmarkImage(m_watermarkImage);
        QUrl imageurl = QUrl("image://scannerImage/watermark");
        targatImage->setProperty(QString("source").toLatin1().data(), imageurl);
    }else{
        m_ScannerImageProvider->setScanImage(m_scanImage);
        QUrl imageurl = QUrl("image://scannerImage/scan");
        targatImage->setProperty(QString("source").toLatin1().data(), imageurl);
    }
    //判断传入方式进行缩放显示
    int way = int(scannerOperayteMode);
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));

    if (scannerOperayteMode == ScannerOperayteMode::NormalMode||scannerOperayteMode == ScannerOperayteMode::Window) {

        QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
        QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_POSRESET.toLatin1().data());
    }
    if (scannerOperayteMode == ScannerOperayteMode::ChangeImage
        || scannerOperayteMode == ScannerOperayteMode::RotateN || scannerOperayteMode == ScannerOperayteMode::RotateS
        || scannerOperayteMode == ScannerOperayteMode::FlipH || scannerOperayteMode == ScannerOperayteMode::FlipV
        || scannerOperayteMode == ScannerOperayteMode::NoOperate||scannerOperayteMode == ScannerOperayteMode::Cut) {

        QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    }
    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
    m_lastProportion = m_proportion;
}

void ScanImageWidget::comparePressed()
{
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    QImage tempImg = m_origImage;
    int defaultProportion = 100 * m_showImageWidSize.width() / tempImg.width();
    if (tempImg.height() * defaultProportion / 100 > m_showImageWidSize.height()) {
        defaultProportion = 100 * m_showImageWidSize.height() / tempImg.height();
    }

    //自适应窗口大小显示
    int tempProportion = 100;
    if (defaultProportion >= 100) {
        tempProportion = 100;
    } else {
        tempProportion = defaultProportion;
    }
    m_tmpImageSize = tempImg.size() * tempProportion / 100;
    //切图
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_RECORDCURRENTFLIP.toLatin1().data());
    scannerOperayteMode = ScannerOperayteMode::ChangeImage;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));

    //显示原图
    m_ScannerImageProvider->setOrigImage(m_origImage);
    QUrl imageurl = QUrl("image://scannerImage/orig");
    targatImage->setProperty(QString("source").toLatin1().data(), imageurl);

    //缩放显示
    int way = int(scannerOperayteMode);
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, tempProportion * 0.01));

    QVariant var;
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
}

void ScanImageWidget::compareReleased()
{
    //设置图片
    if(isWatermark){
        m_ScannerImageProvider->setWarkmarkImage(m_watermarkImage);
        QUrl imageurl = QUrl("image://scannerImage/watermark");
        targatImage->setProperty(QString("source").toLatin1().data(), imageurl);
    }else{
        m_ScannerImageProvider->setScanImage(m_scanImage);
        QUrl imageurl = QUrl("image://scannerImage/scan");
        targatImage->setProperty(QString("source").toLatin1().data(), imageurl);
    }

    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_SETCURRENTFLIP.toLatin1().data());
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_lastProportion * 0.01));
    QVariant var;
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    setScale(var);
}

void ScanImageWidget::exitCut()
{
    scannerOperayteMode = ScannerOperayteMode::ExitCut;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    m_toolWid->show();
    m_cancelBtn->show();
    m_cutBtnWid->hide();
}

void ScanImageWidget::needSaveCut()
{
    QVariant bOK;
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_CHECKCUTSIZE.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, bOK));
    if (!bOK.toBool()){
        QMessageBox msg;
        msg.setText(tr("You have not selected a cropping area or the selected cropping area exceeds the size of the image. Please reselect!"));
        msg.setIcon(QMessageBox::Warning);
        msg.exec();
        return;
    }

    QVariant x, y, w, h;
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_GETCUTINFO_X.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, x));
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_GETCUTINFO_Y.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, y));
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_GETCUTINFO_W.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, w));
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_GETCUTINFO_H.toLatin1().data()
                              , Q_RETURN_ARG(QVariant, h));

    QPoint tmp(x.toDouble(), y.toDouble());
    //备份保存
    Interaction::getInstance()->scannerSaveCutImage(m_proportion, tmp,
                                             QPoint(tmp.x()+w.toDouble(),
                                                    tmp.y()+h.toDouble()));
    //退出裁剪
    scannerOperayteMode = ScannerOperayteMode::ExitCut;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    m_toolWid->show();
    m_cancelBtn->show();
    m_cutBtnWid->hide();
}

void ScanImageWidget::changeQmlSizePos(int w, int h, int x, int y)
{
    m_qmlObj->setProperty(QString("width").toLatin1().data(), w);
    m_qmlObj->setProperty(QString("height").toLatin1().data(), h);
    m_qmlObj->setProperty(QString("x").toLatin1().data(), x);
    m_qmlObj->setProperty(QString("y").toLatin1().data(), y);
}

void ScanImageWidget::changeQmlColor()
{
    QColor color = qApp->palette().color(QPalette::Base);
    color.setAlphaF(m_tran);
    m_qmlObj->setProperty(QString("color").toLatin1().data(), color);
}

void ScanImageWidget::setFormatedText(QSize size)
{
    m_maskWidget->setFixedSize(size);

    m_maskWidget->move((m_showImageWidSize.width() - m_maskWidget->width())/2 + 32,(m_showImageWidSize.height() - m_maskWidget->height())/2 + 24);
    m_loadWidget->move((m_maskWidget->width() - m_loadWidget->width())/2,(m_maskWidget->height() - m_loadWidget->height())/2);
    m_loadWidget->show();
    m_maskWidget->show();
    //工具栏置灰
    m_toolWid->setEnabled(false);
}

void ScanImageWidget::initCutWidget()
{  
    //裁剪工具栏
    m_cutBtnWid = new QWidget(this);
    m_cutBtnWid->setFixedSize(80, 40);
    m_cutBtnLayout = new QHBoxLayout(m_cutBtnWid);
    m_cutBtnWid->hide();
    //保存和取消   
    m_cutCancel = new QPushButton(m_cutBtnWid);
    m_cutCancel->setFixedSize(QSize(24, 24));
    m_cutCancel->setToolTip(tr("Cancel"));
    m_cutCancel->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/cancel.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}");
    m_cutSave = new QPushButton(m_cutBtnWid);
    m_cutSave->setFixedSize(QSize(24, 24));
    m_cutSave->setToolTip(tr("Save"));
    m_cutSave->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/save.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}");

    m_cutBtnWid->setStyleSheet("QWidget{background-color:rgba(255, 255, 255, 1);border:none; border-radius:6px;}");
}

void ScanImageWidget::initPrintWidget()
{
    m_printDialog = new QDialog(this);
    m_printDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_printDialog->setWindowTitle(tr("Save as PDF"));
    m_printDialog->setFixedSize(400,500-40); //标题栏的高度
    m_printDialog->move((this->width()-m_printDialog->width()) / 2, (this->height()-m_printDialog->height()) / 2);
    m_printDialog->setStyleSheet("QDialog{border:1px solid rgba(0,0,0,0.04); background:#FFFFFF;}");

    m_printShowImage = new QLabel(m_printDialog);
    m_printShowImage->setFixedSize(QSize(188,266));
    m_printShowImage->setAlignment(Qt::AlignCenter);
    //纸张阴影
    m_printShowImage->setStyleSheet("QLabel{border:1px solid rgba(0,0,0,0.08); background-color:white}");
    QGraphicsDropShadowEffect *effect1 = new QGraphicsDropShadowEffect(m_printShowImage);
    effect1->setOffset(0, 2);
    effect1->setColor(QColor(0, 0, 0, 31));
    effect1->setBlurRadius(4);
    m_printShowImage->setGraphicsEffect(effect1);


    m_sizeText = new QLabel(m_printDialog);
    m_sizeText->setText(tr("Paper Size"));
    m_sizeText->setFixedSize(70,22);
    m_sizeText->setFocusPolicy(Qt::NoFocus);
    m_sizeText->setStyleSheet("QLabel{color:gray;}");
    m_direText = new QLabel(m_printDialog);
    m_direText->setText(tr("Paper Orientation"));
    m_direText->setFixedSize(70,22);
    m_direText->setFocusPolicy(Qt::NoFocus);
    m_direText->setStyleSheet("QLabel{color:gray;}");

    m_sizeA4Btn= new QRadioButton(m_printDialog);
    m_sizeA4Btn->setText(tr("A4"));
    m_sizeA4Btn->setFixedSize(45,22);
    m_sizeA4Btn->setFocusPolicy(Qt::NoFocus);
    m_sizeA4Btn->setAttribute(Qt::WA_TranslucentBackground);

    m_sizeOrigBtn= new QRadioButton(m_printDialog);
    m_sizeOrigBtn->setText(tr("Original image"));
    m_sizeOrigBtn->setFixedSize(55,22);
    m_sizeOrigBtn->setFocusPolicy(Qt::NoFocus);
    m_sizeOrigBtn->setAttribute(Qt::WA_TranslucentBackground);

    m_direVBtn= new QRadioButton(m_printDialog);
    m_direVBtn->setText(tr("Vertical"));
    m_direVBtn->setFixedSize(55,22);
    m_direVBtn->setFocusPolicy(Qt::NoFocus);
    m_direVBtn->setAttribute(Qt::WA_TranslucentBackground);

    m_direHBtn= new QRadioButton(m_printDialog);
    m_direHBtn->setText(tr("Horizontal"));
    m_direHBtn->setFixedSize(55,22);
    m_direHBtn->setFocusPolicy(Qt::NoFocus);
    m_direHBtn->setAttribute(Qt::WA_TranslucentBackground);
    m_cancelPrintBtn= new QPushButton(m_printDialog);
    m_cancelPrintBtn->setText(tr("Cancel"));
    m_cancelPrintBtn->setFixedSize(96,36);
    m_cancelPrintBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelPrintBtn->setAttribute(Qt::WA_TranslucentBackground);
    m_confirmPrintBtn= new QPushButton(m_printDialog);
    m_confirmPrintBtn->setText(tr("Ok"));
    m_confirmPrintBtn->setFixedSize(96,36);
    m_confirmPrintBtn->setFocusPolicy(Qt::NoFocus);
    m_confirmPrintBtn->setAttribute(Qt::WA_TranslucentBackground);
    m_confirmPrintBtn->setStyleSheet("QPushButton{border:0px;border-radius:5px;background:#1D72F2;color:white;}"
                             "QPushButton::pressed{border:0px;border-radius:5px;background:#1D72F2;color:white;}");
    m_cancelPrintBtn->setStyleSheet("QPushButton{border:0px;border-radius:5px;background:#E6E6E6;color:black;}"
                             "QPushButton::pressed{border:0px;border-radius:5px;background:#E6E6E6;color:black;}");

    //布局
    //大小选项
    m_sizeLayout = new QVBoxLayout();
    m_sizeBtnLayout = new QHBoxLayout();
    m_sizeBtnLayout->addWidget(m_sizeA4Btn,0,Qt::AlignCenter);
    m_sizeBtnLayout->setSpacing(18);
    m_sizeBtnLayout->addWidget(m_sizeOrigBtn,0,Qt::AlignCenter);
    m_sizeBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_sizeLayout->addWidget(m_sizeText,0,Qt::AlignLeft);
    m_sizeLayout->setSpacing(10);
    m_sizeLayout->addLayout(m_sizeBtnLayout);
    m_sizeLayout->setContentsMargins(0, 0, 42, 0);
    //方向选项
    m_direLayout = new QVBoxLayout();
    m_direBtnLayout = new QHBoxLayout();
    m_direBtnLayout->addWidget(m_direVBtn,0,Qt::AlignCenter);
    m_direBtnLayout->setSpacing(18);
    m_direBtnLayout->addWidget(m_direHBtn,0,Qt::AlignCenter);
    m_direBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_direLayout->addWidget(m_direText,0,Qt::AlignLeft);
    m_direLayout->setSpacing(10);
    m_direLayout->addLayout(m_direBtnLayout);
    m_direLayout->setContentsMargins(0, 0, 24, 0);
    //顶部
    m_printTopwid = new QWidget();
    m_printTopwid->setFixedSize(400,266+16+8); //A4高度+距离上/下边的距离
    m_printTopLayout = new QVBoxLayout(m_printTopwid);
    m_printTopLayout->addWidget(m_printShowImage,0,Qt::AlignCenter);
    m_printTopLayout->setContentsMargins(0, 16, 0, 8);
    //中部
    m_printMidwid = new QWidget();
    m_printMidwid->setFixedSize(400,54+24+32); //选项部件+距离上/下边的距离
    m_printMidLayout = new QGridLayout(m_printMidwid);
    m_printMidLayout->addLayout(m_sizeLayout,0,0);
    m_printMidLayout->addLayout(m_direLayout,0,1);
    m_printMidLayout->setContentsMargins(40, 24, 40, 24);
    //底部
    m_printBottomwid = new QWidget();
    m_printBottomwid->setFixedSize(400,36+24);;
    m_printBottomLayout = new QHBoxLayout(m_printBottomwid);
    m_printBottomLayout->addStretch();
    m_printBottomLayout->addWidget(m_cancelPrintBtn,0,Qt::AlignRight);
    m_printBottomLayout->setSpacing(12);
    m_printBottomLayout->addWidget(m_confirmPrintBtn,0,Qt::AlignRight);
    m_printBottomLayout->setContentsMargins(0, 0, 24, 24);
    //整体
    m_prinLayout = new QVBoxLayout(m_printDialog);
    m_prinLayout->addWidget(m_printTopwid,0,Qt::AlignCenter);
    m_prinLayout->setSpacing(0);
    m_prinLayout->addWidget(m_printMidwid,0,Qt::AlignCenter);
    m_prinLayout->setSpacing(0);
    m_prinLayout->addWidget(m_printBottomwid,0,Qt::AlignCenter);
    m_prinLayout->setContentsMargins(0, 0, 0, 0);

    //按钮组
    m_SizeGroupBtn = new QButtonGroup(nullptr);
    m_SizeGroupBtn->addButton(m_sizeA4Btn,0);
    m_SizeGroupBtn->addButton(m_sizeOrigBtn,1);
    m_DireGroupBtn = new QButtonGroup(nullptr);
    m_DireGroupBtn->addButton(m_direVBtn,0);
    m_DireGroupBtn->addButton(m_direHBtn,1);
}

bool ScanImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void ScanImageWidget::resizeEvent(QResizeEvent *event)
{
    m_isEnlarged = false;
    sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);

    m_toolWid->move((this->width() - m_toolWid->width())/2, this->height() - m_toolWid->height() - 6);
    m_cancelBtn->move(m_toolWid->x()+8+10+m_compareWidget->width()+564,m_toolWid->y()+10+10);
    m_cutBtnWid->move((this->width() - m_cutBtnWid->width())/2, this->height() - m_cutBtnWid->height() - 6 );
}

void ScanImageWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(m_cutBtnWid->isVisible()){
            needSaveCut();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void ScanImageWidget::sizeChanged(QSize size, int x,int y)
{
    m_showImageQml->setFixedSize(size);
    m_showImageQml->move(x,y);
    changeQmlSizePos(m_showImageQml->width(), m_showImageQml->height(), 0, 0);
    m_showImageWidSize = m_showImageQml->size();

    if(!(scannerOperayteMode == ScannerOperayteMode::NoOperate)){
       scannerOperayteMode = ScannerOperayteMode::Window;
       ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
       creatImage(-1,true);
    }
}

void ScanImageWidget::startScanImage(QImage image, QString path)
{
    sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
    m_isEnlarged = false;
    m_imagePath = path;
    //切图
    scannerOperayteMode = ScannerOperayteMode::ChangeImage;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    int way = int(scannerOperayteMode);
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
    //设置图片
    m_origImage = image.scaled(m_showImageWidSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ScannerImageProvider->setOrigImage(m_origImage);
    QUrl imageurl = QUrl("image://scannerImage/orig");
    targatImage->setProperty(QString("source").toLatin1().data(), imageurl);
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, 100 * 0.01));
    setFormatedText(m_origImage.size());
    Interaction::getInstance()->scanImage();
}

void ScanImageWidget::exitScanImage()
{
    if(printerPDF) {
        delete printerPDF;
        printerPDF = nullptr;
    }
    if(isWatermark){
        isWatermark = false;
        Interaction::getInstance()->scannerAddWatermark(false, lastwmcontent);
    }

    targatImage->setProperty(QString("source").toLatin1().data(), "");
    scannerOperayteMode = ScannerOperayteMode::NormalMode;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    Q_EMIT toBackShowImageWid();
}

void ScanImageWidget::needSave()
{
    QString format = Variable::SUPPORT_FORMATS_SAVEAS;
    QFileInfo fileinfo(m_imagePath);
    QString suffix = fileinfo.suffix();
    QString upperSuffix = suffix.toUpper();
//    format.insert(0, upperSuffix + "(*." + suffix + ");;"); //原图格式

    m_savePath = QFileDialog::getSaveFileName(this, "", m_imagePath+"/"+fileinfo.baseName()+".png", format);
    if (m_savePath.isNull()) {
        return;
    }
    Interaction::getInstance()->needSaveScanImage(m_savePath);
}

void ScanImageWidget::saveResult(bool result)
{
    if (result) {
        this->exitScanImage();
    }else{
        QFileInfo file(m_savePath);
        QMessageBox::warning(this, tr("Warning"), tr(" \"%1\" save failed,the file does not support saving in this format").arg(file.fileName()));
    }
}

void ScanImageWidget::reduceImage()
{
    scannerOperayteMode = ScannerOperayteMode::ZoomOut;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    int way = int(scannerOperayteMode);
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_OPERATEIMAGE.toLatin1().data(), Q_ARG(QVariant, way));
}

void ScanImageWidget::enlargeImage()
{
    scannerOperayteMode = ScannerOperayteMode::ZoomIn;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    creatImage(-1,true);
}

void ScanImageWidget::originalSize()
{
    zoomToChangeWid();
    scannerOperayteMode = ScannerOperayteMode::LifeSize;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    creatImage(100,true);
}

void ScanImageWidget::adaptiveWidget()
{
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    scannerOperayteMode = ScannerOperayteMode::Window;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    creatImage(-1,true);
}

void ScanImageWidget::rotate()
{
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    ScannerInteractiveQml::getInstance()->setOperateWay(-1);
    scannerOperayteMode = ScannerOperayteMode::RotateS;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    Interaction::getInstance()->scannerRotate();
}

void ScanImageWidget::flipH()
{
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    ScannerInteractiveQml::getInstance()->setOperateWay(-1);
    scannerOperayteMode = ScannerOperayteMode::FlipH;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    Interaction::getInstance()->scannerFlipH();
}

void ScanImageWidget::flipV()
{
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    ScannerInteractiveQml::getInstance()->setOperateWay(-1);
    scannerOperayteMode = ScannerOperayteMode::FlipV;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    Interaction::getInstance()->scannerFlipV();
}

void ScanImageWidget::addWatermark()
{
    if(isWatermark){
        isWatermark = false;
        //显示取消水印后的图片
        Interaction::getInstance()->scannerAddWatermark(false, lastwmcontent);
    }else{
        //弹出对话框
        if(!kinput){
            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                kinput = new kdk::KInputDialog(this);
                kinput->setWindowModality(Qt::ApplicationModal);
            }else{
                kinput = new kdk::KInputDialog(nullptr);
            }

            kinput->setOkButtonText(tr("Ok"));
            kinput->setCancelButtonText(tr("Cancel"));
            kinput->setLabelText(tr("Add watermark"));
            kinput->setTextValue(lastwmcontent);
        }

        if (kinput->exec() == QDialog::Accepted) {
            QString wmcontent = kinput->textValue();
            //显示水印图片
            if(wmcontent != ""){
                isWatermark = true;
                lastwmcontent = wmcontent;
                Interaction::getInstance()->scannerAddWatermark(true, wmcontent);
            }
        }
    }
}

void ScanImageWidget::cutImage()
{
    scannerOperayteMode = ScannerOperayteMode::Cut;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));
    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    creatImage(-1,true);
    m_toolWid->hide();
    m_cancelBtn->hide();
    m_cutBtnWid->show();

    int width, height, x, y;
    QVariant var;
    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_GETROTATETO.toLatin1().data(), Q_RETURN_ARG(QVariant, var));

    if(qAbs(var.toInt() % 360) == 90 ||
            qAbs(var.toInt() % 360) == 270)
    {
        height = m_scanImage.width()*m_proportion/100-1;
        width = m_scanImage.height()*m_proportion/100-1;
    }
    else
    {
        width = m_scanImage.width()*m_proportion/100-1;
        height = m_scanImage.height()*m_proportion/100-1;
    }
    x = m_showImageQml->width()/2 - width/2;
    y = m_showImageQml->height()/2 - height/2;

    QMetaObject::invokeMethod(scannerImageItem, SCANNERIMAGE_FUNC_SETCUTSIZE.toLatin1().data(),
                              Q_ARG(QVariant, width), Q_ARG(QVariant, height),
                              Q_ARG(QVariant, x), Q_ARG(QVariant, y));
}

void ScanImageWidget::print()
{
    m_globalPrintDialog = new QPrintDialog(&printer, this);
    connect(m_globalPrintDialog, &QPrintDialog::finished, this, [=](int result){
        if (1 != result) {
            qDebug() << "退出打印或打印失败！" << result;
        }else{
            Interaction::getInstance()->printScanImage(&printer);
        }
    });

    m_globalPrintDialog->setModal(true);
    m_globalPrintDialog->show();
}

void ScanImageWidget::showSaveMenu()
{
    QPoint menuPos = m_saveWid->mapToGlobal(QPoint(0, 0)); //全局坐标系中的位置
    QSize menuSize = m_saveMenu->sizeHint();
    menuPos.setX(menuPos.x() - ((menuSize.width() - m_saveWid->width()) - 10));
    menuPos.setY(menuPos.y() - menuSize.height());
    m_saveMenu->exec(menuPos);
}

void ScanImageWidget::saveAsPDF()
{
    if(printerPDF == nullptr){
        //打印设置
        printerPDF = new QPrinter();
        printerPDF->setOutputFormat(QPrinter::PdfFormat);
        printerPDF->setResolution(72);
        m_sizeA4Btn->setChecked(1);
        m_direVBtn->setChecked(1);
        m_printShowImage->resize(188,266);
    }
    m_printShowImage->setPixmap(QPixmap::fromImage(m_printImage).scaled(m_printShowImage->size(),
                                                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QPoint menuPos = this->mapToGlobal(QPoint(0, 0));
    m_printDialog->move( menuPos.x()+ (this->width() - m_printDialog->width())/2 , menuPos.y()+(this->height() - m_printDialog ->height() )/2);
    isPrintPDF = true;
    m_printDialog->setModal(true);
    m_printDialog->show();

}

void ScanImageWidget::printSize(bool checked)
{
    if(checked){ //A4
        //更改界面图片显示
        if(m_direVBtn->isChecked()){
            m_printShowImage->setFixedSize(QSize(188,266));
        }else{
            m_printShowImage->setFixedSize(QSize(266,188));
        }
        m_printShowImage->setPixmap(QPixmap::fromImage(m_printImage).scaled(m_printShowImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
        //设置打印纸张大小为A4
        printerPDF->setPaperSize(QPrinter::A4);
        printerPDF->setResolution(72);
        m_direHBtn->setEnabled(true);
        m_direVBtn->setEnabled(true);
    }else{
        //更改界面图片显示
        QPixmap scaledPix = QPixmap::fromImage(m_printImage).scaled(m_printShowImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        m_printShowImage->setFixedSize(scaledPix.size());
        m_printShowImage->setPixmap(scaledPix);
        //设置打印纸张大小为原图
        QSizeF imageSize = m_printImage.size();
        QSizeF sizeInMillimeters = imageSize * 25.4 / printerPDF->resolution(); // 将像素转换为毫米
        printerPDF->setPaperSize(sizeInMillimeters, QPrinter::Millimeter);
        printerPDF->setOrientation(QPrinter::Portrait);  //默认值为纵向
        //禁用方向按钮组
        m_direHBtn->setEnabled(false);
        m_direVBtn->setEnabled(false);
    }

}

void ScanImageWidget::printDire(bool checked)
{
    if(checked){ //纵向
        printerPDF->setOrientation(QPrinter::Portrait);
        m_printShowImage->setFixedSize(QSize(188,266));

    }else{
        printerPDF->setOrientation(QPrinter::Landscape);
        m_printShowImage->setFixedSize(QSize(266,188));
    }
    m_printShowImage->setPixmap(QPixmap::fromImage(m_printImage).scaled(m_printShowImage->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}

void ScanImageWidget::cancelPrint()
{
    isPrintPDF = false;
    m_printDialog->hide();
}

void ScanImageWidget::confirmPrint()
{
    m_printDialog->hide();
    //保存路径
    QFileInfo fileInfo (m_imagePath);
    QString openPath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".pdf";
    QString saveFormat = "PDF(*.pdf);;";
    m_savePath = QFileDialog::getSaveFileName(nullptr,"", openPath, saveFormat);
    //设置路径
    printerPDF->setOutputFileName(m_savePath);
    //保存文件
    Interaction::getInstance()->scannerSavePDF(printerPDF);

}

void ScanImageWidget::getScanAndOrigImage(QImage scanImage, QImage origImage)
{
    if(scannerOperayteMode == ScannerOperayteMode::NormalMode){ //水印状态保存退出时
        return;
    }

    scannerOperayteMode = ScannerOperayteMode::ChangeImage;
    ScannerInteractiveQml::getInstance()->setOperateWay(int(scannerOperayteMode));

    if(isWatermark){
        m_watermarkImage = scanImage;
    }else{
        m_scanImage = scanImage;
    }
    m_origImage = origImage;
    m_printImage = scanImage;
    m_tempImg = scanImage;

    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    creatImage(-1, true);

    //去遮罩和工具栏可用
    m_maskWidget->hide();
    m_toolWid->setEnabled(true);
}

void ScanImageWidget::setScale(QVariant scale)
{
    m_proportion = scale.toInt();
    recordScannerCurrentProportion = m_proportion;
    m_percentage->setText(QString::number(m_proportion) + "%");
    m_percentage->setToolTip(QString::number(m_proportion) + "%");
}
void ScanImageWidget::getFlipAndOrigImage(QImage flipImage, QImage origImage)
{
    m_printImage = flipImage;
    m_tempImg = flipImage;

    if(m_isEnlarged){
        sizeChanged( QSize(this->width()-32-32,this->height()-24-16-m_toolWid->height()-6),32,24);
        m_isEnlarged = false;
    }
    creatImage(-1,true);
}

void ScanImageWidget::needChangeOperateWay()
{
    //获取当前图片显示方式（1——放大/2——缩小/3——自适应）
    QVariant way;
    QMetaObject::invokeMethod(targatImage, SCANNERIMAGE_FUNC_GETOPERATEWAY.toLatin1().data(), Q_RETURN_ARG(QVariant, way));
    m_imageShowWay = way.toInt();
    if(m_imageShowWay == 1){
        scannerOperayteMode = ScannerOperayteMode::ZoomIn;
    }
    if(m_imageShowWay == 2){
        scannerOperayteMode = ScannerOperayteMode::ZoomOut;
    }
}
