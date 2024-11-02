#include "showimagewidget.h"
#include "kyview.h"
#include "sizedate.h"
#include "global/variable.h"
#include "global/interactiveqml.h"
#include <lingmostylehelper/lingmostylehelper.h>
#include "windowmanager/windowmanager.h"

ShowImageWidget::ShowImageWidget(QWidget *parent) : QWidget(parent)
{
    this->initInteraction();
    this->initQWidgetObject();
    this->initQmlObject();
    this->initMenu();
    this->initChangeImageBtn();
    Variable::g_tempImage.load("qrc:/res/res/damaged_img.png");
    //图片没处理完毕时，显示转圈圈图
    m_loadingMovie = new QMovie(":/res/res/loadgif.gif");
    m_loadingMovie->setParent(this);

    this->initCutWid();
    this->initOcrWid();
    this->ocrBtnWidStyle();

    m_imagePageWid = new MultiTiffToolWid(this);

    m_leftPageBtn = new MultiTiffToolBtn("left", m_imagePageWid);
    m_rightPageBtn = new MultiTiffToolBtn("right", m_imagePageWid);
    m_pageNum = new QLabel(m_imagePageWid);
    //    QFont font;
    //    font.setPointSize(18);
    //    m_pageNum->setFont(font);
    //    m_leftPageBtn->setIcon(QIcon::fromTheme("lingmo-start-symbolic"));
    //    m_rightPageBtn->setIcon(QIcon::fromTheme("lingmo-end-symbolic"));
    m_rightPageBtn->setIconSize(QSize(36, 36));
    m_leftPageBtn->setIconSize(QSize(36, 36));

    m_pageLayout = new QHBoxLayout(this);
    m_pageLayout->addWidget(m_leftPageBtn, 0, Qt::AlignVCenter);
    m_pageLayout->addWidget(m_pageNum, 0, Qt::AlignCenter);
    m_pageLayout->addWidget(m_rightPageBtn, 0, Qt::AlignVCenter);
    m_pageLayout->setContentsMargins(6, 0, 6, 0);
    m_imagePageWid->setLayout(m_pageLayout);

    m_imagePageWid->hide();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    //绘制阴影
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(TOOL_COLOR);       //设置阴影颜色，也可以setColor(QColor(220,220,220))
    effect->setBlurRadius(BLUR_RADIUS); //设定阴影的模糊半径，数值越大越模糊
    m_btnWid->setGraphicsEffect(effect);
    m_ocrBtnWid->setGraphicsEffect(effect);
    m_widChangeHeight = makeSureHeightState();

    this->setMouseTracking(true);
    this->update();
    this->initConnect();
}

ShowImageWidget::~ShowImageWidget()
{
    if (m_imageMenu)
        delete m_imageMenu;
}

void ShowImageWidget::initConnect()
{
    connect(g_next, &QPushButton::clicked, this, &ShowImageWidget::nextImage);
    connect(g_back, &QPushButton::clicked, this, &ShowImageWidget::backImage);
    //右键菜单
    connect(m_showImage, &QLabel::customContextMenuRequested, [=](const QPoint &pos) {
        Q_UNUSED(pos);
        abnormalJudgBeforeClick();
        m_imageMenu->exec(QCursor::pos());
    });
    connect(m_showImageQml, &QQuickWidget::customContextMenuRequested, [=](const QPoint &pos) {
        Q_UNUSED(pos);
        abnormalJudgBeforeClick();
        m_imageMenu->exec(QCursor::pos());
    });
    //点击菜单选项
    connect(m_copy, &QAction::triggered, this, &ShowImageWidget::copy);
    connect(m_reName, &QAction::triggered, this, &ShowImageWidget::reName);
    connect(m_setDeskPaper, &QAction::triggered, this, &ShowImageWidget::setDeskPaper);
    //    connect(m_setLockPaper, &QAction::triggered, this,&ShowImageWidget::setLockPaper);
    connect(m_print, &QAction::triggered, this, &ShowImageWidget::initPrint);
    connect(m_deleteImage, &QAction::triggered, this, &ShowImageWidget::deleteImage);
    connect(m_showInFile, &QAction::triggered, this, &ShowImageWidget::showInFile);
    connect(m_cutWid, &CutWIdget::showBtnWid, this, &ShowImageWidget::showBthWid);
    connect(m_cancel, &QPushButton::clicked, this, &ShowImageWidget::exitCut);
    connect(m_save, &QPushButton::clicked, this, &ShowImageWidget::needSave);
    connect(m_cancel, &QPushButton::clicked, this, &ShowImageWidget::cutFinishUi);
    connect(m_saveAs, &QAction::triggered, this, &ShowImageWidget::saveAsOther);
    connect(m_signImage, &QAction::triggered, this, &ShowImageWidget::signImage);
    // OCR
    connect(m_ocrCancel, &kdk::KBorderlessButton::clicked, this, &ShowImageWidget::exitGetText);
    connect(m_ocrExportToTxt, &QPushButton::clicked, this, &ShowImageWidget::exportToTxt);
    connect(m_timer, &QTimer::timeout, this, &ShowImageWidget::realBuriedPoint);
    connect(m_leftPageBtn, &QPushButton::clicked, this, &ShowImageWidget::previousTiffImage);
    connect(m_rightPageBtn, &QPushButton::clicked, this, &ShowImageWidget::nextTiffImage);
}

void ShowImageWidget::sideState(int num)
{
    Q_EMIT changeSideSize(num);
}
//根据number决定界面显示:相册，左右按钮，删除时相册显示
bool ShowImageWidget::imageNum(int number)
{
    //侧栏上方新增加号，故number加1
    //判断有几张图片，分别进行处理：删除到0，显示打开界面；只有一张：不显示左右按钮。
    if (number == 1) {
        Q_EMIT clearImage();
        //点击删除按钮删除全部文件时，此标志位应该重设为默认状态，防止之后继续打开图片造成相册大小有误差
        m_isDelete = false;
        return true;
    }
    if (number == 2) {
        g_buttonState = false;
    } else {
        //删除时逻辑导致item总是比实际多一张，暂时先在前端进行判断来解决相关问题
        if (number == 3 && m_isDelete) {
            g_buttonState = false;
        } else {
            g_buttonState = true;
        }
    }
    return false;
}
void ShowImageWidget::previousTiffImage()
{
    if (m_programeIsClosing) {
        return;
    }
    m_tiffPage -= 1;
    if (m_tiffPage <= 0) {
        m_tiffPage = m_imagePageList.length();
    }
    Variable::g_nextOrPreviousTiff = true;
    Variable::g_tempImage = m_imagePageList.at(m_tiffPage - 1);
    InteractiveQml::getInstance()->setImageFormat("multiTiff");
    m_pageNum->setText(QString::number(m_tiffPage) + "/" + QString::number(m_imagePageList.length()));
    operayteMode = OperayteMode::BackImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->qmlTiffImagePage(m_tiffPage);
    Interaction::getInstance()->watchAutoImage();
}

void ShowImageWidget::nextTiffImage()
{
    if (m_programeIsClosing) {
        return;
    }
    m_tiffPage += 1;
    if (m_tiffPage >= m_imagePageList.length() + 1) {
        m_tiffPage = 1;
    }
    Variable::g_nextOrPreviousTiff = true;
    Variable::g_tempImage = m_imagePageList.at(m_tiffPage - 1);
    InteractiveQml::getInstance()->setImageFormat("multiTiff");
    m_pageNum->setText(QString::number(m_tiffPage) + "/" + QString::number(m_imagePageList.length()));
    operayteMode = OperayteMode::NextImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Interaction::getInstance()->qmlTiffImagePage(m_tiffPage);
    Interaction::getInstance()->watchAutoImage();
}

void ShowImageWidget::multiTiffOperate()
{
    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;

    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }

    //初始化信号槽连接
    this->qmlAndWidgetConnectTiff(imageItemLower);
    //    if (m_tiffPage) {
    int tempWidth = m_imagePageList.at(m_tiffPage - 1).width();
    int tempHeight = m_imagePageList.at(m_tiffPage - 1).height();
    Variable::setLoadedImageMap(m_imagePath + QString::number(m_tiffPage - 1), m_imagePageList.at(m_tiffPage - 1));
    QMetaObject::invokeMethod(targatImage, "changeImage",
                              Q_ARG(QVariant, "image://main/" + m_imagePath + QString::number(m_tiffPage - 1)),
                              Q_ARG(QVariant, tempWidth), Q_ARG(QVariant, tempHeight), Q_ARG(QVariant, true),
                              Q_ARG(QVariant, m_tiffPage - 1), Q_ARG(QVariant, Variable::g_hasRotation));
    //    }
    int way = int(operayteMode);
    //判断传入方式进行缩放显示
    //    if (m_tiffPage - 1 <= 0 && !Variable::g_changedImageList.contains(m_imagePath)) {
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_OPERATE.toLatin1().data(), Q_ARG(QVariant, way));
    //    }
    if (operayteMode != OperayteMode::LifeSize) {
        //如果切换图片，则关闭导航栏
        this->needCloseNavigator();
    }
    //设置当前缩放比
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                              Q_ARG(QVariant, m_proportion * 0.01));
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_POSRESET.toLatin1().data());
    //    }
    /* 其余方式不需要设置qml操作，qml里实现其他操作 */

    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    m_proportion = var.toInt();
    //发送给工具栏
    setScale(var);
    return;
}


void ShowImageWidget::qmlAndWidgetConnectTiff(QObject *obj)
{
    //    if (m_hasConnectedTiff) {
    //        return;
    //    }
    //    m_hasConnectedTiff = true;
    connect(obj, SIGNAL(sendScale(QVariant)), this, SLOT(setScale(QVariant)));
    connect(obj, SIGNAL(needShowNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)), this,
            SLOT(showNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)));
    connect(obj, SIGNAL(closeNavigator()), this, SLOT(needCloseNavigator()));
    connect(obj, SIGNAL(buriedPoint()), this, SLOT(zoomBuriedPoint()));
}

void ShowImageWidget::changeImageFromViewSwipe(QVariant imagePath, QVariant nextOrBack)
{
    //    operayteMode = OperayteMode::ChangeImage;
    //    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //        this->openImage(imagePath.toString());
    if (nextOrBack.toBool()) {
        this->nextImage();
    } else {
        this->backImage();
    }
}


void ShowImageWidget::copyFromQml()
{
    m_copyFromQml = true;
    this->copy();
}

//根据图片是否为空决定是否显示转圈圈
int i = 0;
void ShowImageWidget::imageNUll()
{
    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }
    //判断如果是裁剪，需要遮罩蒙层
    if (operayteMode == OperayteMode::Cut) {
        int way = int(operayteMode);
        //判断传入方式进行缩放显示
        QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_OPERATE.toLatin1().data(), Q_ARG(QVariant, way));
        return;
    }

    //初始化信号槽连接
    this->qmlAndWidgetConnectNormal(imageItemLower);
    //    if (m_imageSize != "0x0" || m_imageSize != "-") {
    //        int tempWidth = m_imageSize.mid(0, m_imageSize.indexOf("x")).toInt();
    //        int tempHeight = m_imageSize.mid(m_imageSize.indexOf("x") + 1, -1).toInt();
    //        //        QMetaObject::invokeMethod(targatImage, "changeImage", Q_ARG(QVariant, ""), Q_ARG(QVariant,
    //        tempWidth),
    //        //                                  Q_ARG(QVariant, tempHeight), Q_ARG(QVariant, false));
    //    } else {
    //        //更新图片
    //        //                targatImage->setProperty(IMAGE_SOURCE.toLatin1().data(), "image://thumb/" +
    //        m_imagePath);
    //    }
    int way = int(operayteMode);
    //判断传入方式进行缩放显示
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_OPERATE.toLatin1().data(), Q_ARG(QVariant, way));

    //首次打开-无状态
    if (operayteMode == OperayteMode::NoOperate) {
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    }
    //同一张图片: 退出ocr，退出裁剪，适应窗口，原图展示，ocr
    if (operayteMode == OperayteMode::NormalMode || operayteMode == OperayteMode::Window
        || operayteMode == OperayteMode::Ocr) {
        if (targatImage->property("curScale").toDouble() != m_proportion * 0.01) {
            QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                      Q_ARG(QVariant, m_proportion * 0.01));
        }
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_POSRESET.toLatin1().data());
    }
    //切图
    if (operayteMode == OperayteMode::ChangeImage || operayteMode == OperayteMode::NextImage
        || operayteMode == OperayteMode::BackImage || operayteMode == OperayteMode::LifeSize
        || operayteMode == OperayteMode::RotateN || operayteMode == OperayteMode::RotateS
        || operayteMode == OperayteMode::FlipH || operayteMode == OperayteMode::FlipV
        || operayteMode == OperayteMode::Sign) {
        if (operayteMode != OperayteMode::LifeSize) {
            //如果切换图片，则关闭导航栏
            this->needCloseNavigator();
        }

        //设置当前缩放比
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    }
    //    if (operayteMode == OperayteMode::ChangeImage || operayteMode == OperayteMode::NextImage
    //        || operayteMode == OperayteMode::BackImage) {
    //        this->needCloseNavigator();
    //        //设置当前缩放比
    //        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
    //                                  Q_ARG(QVariant, m_proportion * 0.01));
    //    }
    /* 其余方式不需要设置qml操作，qml里实现其他操作 */

    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    m_proportion = var.toInt();
    //发送给工具栏
    setScale(var);
    if (Variable::g_needEnterSign) {
        Q_EMIT enterSign();
    }
    return;
}

void ShowImageWidget::showMovieImage()
{
    if (m_proportion == 0) {
        return;
    }
    if (m_imagePath == "") {
        return;
    }
    //避免出现静图也使用动图的qml加载的情况
    if (!m_movieImage || !MOVIE_IMAGE_FORMAT.contains(m_paperFormat)) {
        m_recordMovieBefore = true;
        operayteMode = OperayteMode::NoOperate;

        InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
        imageNUll();
        return;
    }

    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 1, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }

    //初始化信号槽连接
    this->qmlAndWidgetConnectMovie(imageItemLower);

    QString path = "file://" + m_imagePath;

    //    if (m_imageSize != "0x0" || m_imageSize != "-") {
    //        int tempWidth = m_imageSize.mid(0, m_imageSize.indexOf("x")).toInt();
    //        int tempHeight = m_imageSize.mid(m_imageSize.indexOf("x") + 1, -1).toInt();
    //        //        QMetaObject::invokeMethod(targatImage, "changeImage", Q_ARG(QVariant, path), Q_ARG(QVariant,
    //        //        tempWidth),
    //        //                                  Q_ARG(QVariant, tempHeight));

    //    } else {
    //        //        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_SETSOURCE.toLatin1().data(), Q_ARG(QVariant,
    //        path));
    //    }
    int way = int(operayteMode);
    //判断传入方式进行缩放显示
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_OPERATE.toLatin1().data(), Q_ARG(QVariant, way));

    //首次打开-无状态
    if (operayteMode == OperayteMode::NoOperate) {
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    }
    //同一张图片: 退出ocr，退出裁剪，适应窗口，原图展示，ocr
    if (operayteMode == OperayteMode::NormalMode || operayteMode == OperayteMode::Window
        || operayteMode == OperayteMode::Ocr) {
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_POSRESET.toLatin1().data());
    }

    //切图
    if (operayteMode == OperayteMode::ChangeImage || operayteMode == OperayteMode::NextImage
        || operayteMode == OperayteMode::BackImage || operayteMode == OperayteMode::LifeSize
        || operayteMode == OperayteMode::RotateN || operayteMode == OperayteMode::RotateS
        || operayteMode == OperayteMode::FlipH || operayteMode == OperayteMode::FlipV) {
        if (operayteMode != OperayteMode::LifeSize) {
            //如果切换图片，则关闭导航栏
            this->needCloseNavigator();
        }
        //设置当前缩放比
        QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_CHANGESCALE.toLatin1().data(),
                                  Q_ARG(QVariant, m_proportion * 0.01));
    } else {
        qmlImageRect->setProperty(IMAGE_NEW_LOADED.toLatin1().data(), 0);
    }

    m_recordMovieBefore = true;
    operayteMode = OperayteMode::NormalMode;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //返回当前缩放比
    QVariant var;
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    //发送给工具栏
    setScale(var);
    return;
}

bool ShowImageWidget::operatorBeforeShowImage(QPixmap pixmap)
{
    if (pixmap.isNull()) {
        if (m_paperFormat == "gif" || m_paperFormat == "apng" || Variable::g_currentImageType == "multiTiff") {
            return false;
        }
        this->m_showImage->setMovie(m_loadingMovie);
        m_loadingMovie->start();
        m_showImage->show();
        m_showImageQml->hide();
        return true;
    }
    if (m_loadingMovie->state() != QMovie::NotRunning) {
        m_loadingMovie->stop();
    }
    if (m_isCutting) {
        m_showImage->setPixmap(pixmap);
        m_showImage->show();
        m_showImageQml->hide();
        Q_EMIT perRate(QString::number(m_proportion) + "%", m_imagePath);
        return true;
    }
    if (!m_isOpenSuccess) {
        m_showImage->setPixmap(pixmap);
        m_showImage->show();
        m_showImageQml->hide();
        Q_EMIT perRate(QString::number(m_proportion) + "%", m_imagePath);
        return true;
    }
    m_showImage->hide();
    m_showImageQml->show();
    return false;
}
//根据图片类型刷新右键菜单内容
void ShowImageWidget::imageMenu(QFileInfo info, QString imageSize, QString colorSpace)
{
    QString filename = info.fileName();
    //区分动图和静态图
    if (m_movieImage) {
        //设置壁纸--动图在传来时是一帧一帧，只判断并添加一次右键菜单选项
        //        if (m_canSet) {
        //            m_canSet = false;
        Q_EMIT titleName(filename, m_imagePath);                        //给顶栏图片的名字
        Q_EMIT changeInfor(info, imageSize, colorSpace, m_paperFormat); //给信息栏需要的信息
        //设置右键菜单
        setMenuAction();
        //判断图片是否打开失败或者为无权限修改的图片
        stateUpdate();
        //给工具栏确定裁剪按钮的状态--必须在stateUpdate后
        Q_EMIT sendImageType(false);
        //        }
    } else {
        Q_EMIT titleName(filename, m_imagePath);                        //给顶栏图片的名字
        Q_EMIT changeInfor(info, imageSize, colorSpace, m_paperFormat); //给信息栏需要的信息
        setMenuAction();
        //判断图片是否打开失败或者为无权限修改的图片
        stateUpdate();
        //给工具栏确定裁剪按钮的状态--必须在stateUpdate后
        if (!Variable::SUPPORT_FORMATS_NOT.contains("." + m_paperFormat.toLower()) && (m_isOpenSuccess && m_delOrNot)) {
            Q_EMIT sendImageType(true);
        } else {
            Q_EMIT sendImageType(false);
        }
    }
}

void ShowImageWidget::albumChangeImage(bool isChange)
{
    qmlAndWidgetDisconnectNormal();
    m_canSet = isChange;
}
//下一张
void ShowImageWidget::nextImage()
{
    qmlAndWidgetDisconnectNormal();
    Variable::g_nextOrPreviousTiff = false;
    operayteMode = OperayteMode::NextImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_canSet = true;
    Interaction::getInstance()->nextImage();
}
//上一张
void ShowImageWidget::backImage()
{
    qmlAndWidgetDisconnectNormal();
    Variable::g_nextOrPreviousTiff = false;
    operayteMode = OperayteMode::BackImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_canSet = true;
    Interaction::getInstance()->backImage();
}
//备用，图片加载过慢时，ui的处理--暂缓
void ShowImageWidget::delayShow(bool isLoading)
{
    if (isLoading == false) {
        if (m_loadingMovie->state() != QMovie::NotRunning) {
            m_loadingMovie->stop();
        }
        return;
    } else {
        this->m_showImage->setMovie(m_loadingMovie);
        m_loadingMovie->start();
        Q_EMIT toShowImage(); //给主界面--展示图片
    }
}

void ShowImageWidget::albumSlot(bool isOpen)
{
    m_isOpen = isOpen;
}

void ShowImageWidget::isDelete(bool isDel)
{
    qmlAndWidgetDisconnectNormal();
    if (!m_delOrNot) {
        m_isDelete = m_delOrNot;
        return;
    }
    m_isDelete = isDel;
    //    Q_EMIT toSelectHigh(isDel);
}

//复制
void ShowImageWidget::copy()
{
    //复制到剪切板
    //    QClipboard *clipBoard=QApplication::clipboard();
    //    clipBoard->setPixmap(m_pic);
    //    复制为文件
    QList<QUrl> copyfile;

    QUrl url = QUrl::fromLocalFile(m_imagePath); //待复制的文件
    if (url.isValid()) {
        copyfile.push_back(url);
    } else {
        return;
    }
    QMimeData *data = new QMimeData;
    data->setUrls(copyfile);
    data->setText(m_imagePath);

    QClipboard *clipBoard = QApplication::clipboard();
    clipBoard->setMimeData(data);
    //复制-埋点
    kdk::kabase::BuriedPoint buriedPointCopy;
    if (buriedPointCopy.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                            kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerCoyp)) {
        qDebug() << "buried point fail Copy!";
    }
}

void ShowImageWidget::updateImagePath(QString newImagePath)
{
    m_imagePath = newImagePath;
}

void ShowImageWidget::setScale(QVariant scale)
{
    m_proportion = scale.toInt();
    recordCurrentProportion = m_proportion;
    Q_EMIT perRate(QString::number(m_proportion) + "%", m_imagePath);
}


void ShowImageWidget::getDropImagePath(QVariant pathUrl)
{
    QString imagePath = pathUrl.toUrl().path().toLocal8Bit();
    if (imagePath.startsWith("file://")) {
        imagePath = imagePath.remove("file://");
    }
    if (imagePath == "") {
        return;
    }
    QFileInfo info(imagePath);
    if (info.exists()) {
        //        //获得所有可被查看的图片格式
        //        QStringList formatList;
        //        QString format;
        //        format = "";
        //        for (const QString &str : Variable::SUPPORT_FORMATS) {
        //            format = str;
        //            formatList.append(format);
        //        }
        //        if (formatList.contains(info.suffix().toLower())) {
        //            Variable::setSettings("imagePath", info.absolutePath());
        //            operayteMode = OperayteMode::ChangeImage;
        //            InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
        //            this->openImage(imagePath);
        //        }
        Variable::setSettings("imagePath", info.absolutePath());
        this->openImage(imagePath);
    }
    return;
}

void ShowImageWidget::operatyImage()
{
    //正式加载图片
    if (m_movieImage && m_paperFormat != "tif" && m_paperFormat != "tiff") {
        //动图
        showMovieImage();
    } else {
        if (m_movieImage && (m_paperFormat == "tif" || m_paperFormat == "tiff")) {
            multiTiffOperate();
        } else {
            //静态图
            imageNUll();
        }
    }
}
//设置为桌面壁纸
void ShowImageWidget::setDeskPaper()
{
    Interaction::getInstance()->setAsBackground();
}
//设置为锁屏壁纸
void ShowImageWidget::setLockPaper()
{
    qDebug() << "设置为锁屏壁纸";
}
//初始化打印
void ShowImageWidget::initPrint()
{
    m_printDialog = new QPrintDialog(this);
    connect(m_printDialog, SIGNAL(accepted(QPrinter *)), this, SLOT(acceptPrint(QPrinter *)));
    connect(m_printDialog, &QPrintDialog::finished, this, &ShowImageWidget::finishPrint);
    m_printDialog->setModal(true);
    m_printDialog->show();
}
//打印开始
void ShowImageWidget::acceptPrint(QPrinter *printer)
{
    Interaction::getInstance()->printImage(printer);
}
//开始背景替换
void ShowImageWidget::enterReplaceBackground()
{
    //背景替换页面切换
    if (m_replaceBackgroundWid == nullptr) {
        m_replaceBackgroundWid = new ReplaceBackgroundWidget(this);
        Q_EMIT initMattingConnect();
        connect(m_replaceBackgroundWid, &ReplaceBackgroundWidget::toBackShowImageWid, this, &ShowImageWidget::exitReplaceBackground);
    }
    m_isReplacing = true;
    Q_EMIT toReplaceBackground();
    operayteMode = OperayteMode::Matting;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_replaceBackgroundWid->m_origImagePath = m_imagePath;
    m_replaceBackgroundWid->m_origFormat = m_paperFormat;
    m_replaceBackgroundWid->m_saveFlag = true;
    QPixmap pixmap(m_imagePath);
    m_replaceBackgroundWid->m_showImage->setPixmap(pixmap.scaled(m_replaceBackgroundWid->m_showImage->size(),
                                                                 Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_replaceBackgroundWid->m_showImage->setAlignment(Qt::AlignCenter);
    m_replaceBackgroundWid->m_showImage->show();
    m_replaceBackgroundWid->m_showMattingImageQml->hide();
    m_replaceBackgroundWid->m_loadingLabel->setText(tr("Intelligent cutout..."));
    m_replaceBackgroundWid->m_maskWidget->show();
    m_replaceBackgroundWid->m_maskWidget->raise();
    Interaction::getInstance()->replaceBackground(255, 255, 255, 0, true);
    m_replaceBackgroundWid->show();
    m_showImageQml->hide();
}
//退出背景替换
void ShowImageWidget::exitReplaceBackground()
{
    m_isReplacing = false;
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_replaceBackgroundWid->hide();
    m_showImageQml->show();
    Q_EMIT quitReplaceBackground();
}

void ShowImageWidget::enterScanImage()
{
    if (m_scanImageWid == nullptr) {
        m_scanImageWid = new ScanImageWidget(this);
        Q_EMIT initScannerConnect();
        connect(m_scanImageWid, &ScanImageWidget::toBackShowImageWid, this, &ShowImageWidget::exitScanImage);
    }
    Q_EMIT toScanImage();  //kyview
    m_isScanning = true;
    operayteMode = OperayteMode::Scanner;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_scanImageWid->startScanImage(m_pic.toImage(),m_imagePath);
    m_scanImageWid->show();
    m_showImageQml->hide();
}

void ShowImageWidget::exitScanImage()
{
    m_isScanning = false;
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_scanImageWid->hide();
    m_showImageQml->show();
    Q_EMIT quitScanImage();
}
//开始裁剪
void ShowImageWidget::startCutColor()
{
    QObject *qmlImageRect;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect)) {
        return;
    }
    //拿当前显示的图片类型
    int imageType = qmlImageRect->property(IMAGE_TYPE.toLatin1().data()).toInt();
    //动图无裁剪
    if (imageType == 1) {
        return;
    }
    displayMode = DisplayMode::CuttingMode;
    m_isCutting = true;

    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }

    //从qml拿位置和尺寸
    QVariant x;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETLEFTUPPOSACCORDWID_X.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, x));
    QVariant y;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETLEFTUPPOSACCORDWID_Y.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, y));
    QVariant w;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETVISIBLESIZEACCORDWID_W.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, w));
    QVariant h;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETVISIBLESIZEACCORDWID_H.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, h));
    QVariant scale;
    QMetaObject::invokeMethod(targatImage, IMAGE_FUNC_SENDSCALE.toLatin1().data(), Q_RETURN_ARG(QVariant, scale));

    QVariant xImage;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_X.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, xImage));

    QVariant yImage;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_GETLEFTUPPOSACCORDIMAGE_Y.toLatin1().data(),
                              Q_RETURN_ARG(QVariant, yImage));

    double xPos = x.toDouble();
    double yPos = y.toDouble();

    if (xPos == 0) {
        xPos = (this->width() - w.toDouble()) / 2;
    }
    if (yPos == 0) {
        yPos = (this->height() - h.toDouble()) / 2;
    }
    //给后端发信号，进行裁剪--发送的都是相对于在原图上的位置和大小以及缩放比也是处理好的
    Interaction::getInstance()->qmlStartCutDisplay(
        QPointF(xImage.toDouble(), yImage.toDouble()),
        QSizeF(w.toDouble() / (scale.toDouble() * 0.01), h.toDouble() / (scale.toDouble() * 0.01)),
        scale.toDouble() * 0.01);

    //设置widget界面控件的位置和尺寸
    m_showImage->resize(QSize(w.toDouble(), h.toDouble()));
    m_showImage->move(xPos, yPos);
    m_cutWid->show();
    m_cutWid->resize(m_showImage->size());
    m_cutWid->move(m_showImage->pos());
}
//退出裁剪
void ShowImageWidget::exitCutColor()
{
    m_cutWid->hide();
}
//开始OCR
void ShowImageWidget::startGetText()
{
    ocrMode();
    m_ocrWid->show();
    m_ocrBtnWid->show();
    m_ocrWid->setFormatedText(tr("OCR recognition..."));
}

void ShowImageWidget::setGetTextResult(QVector<QString> result)
{
    m_ocrWid->setFormatedText(result);
    QString text;
    for (QString line : result) {
        if (line.isEmpty()) {
            continue;
        }
        text += line + '\n';
    }
        if (text.isEmpty()) {
        m_ocrExportToTxt->setEnabled(false);
    } else {
        m_ocrExportToTxt->setEnabled(true);
    }
}
//结束OCR
void ShowImageWidget::exitGetText()
{
    operayteMode = OperayteMode::NormalMode;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_ocrWid->clear();
    m_ocrWid->hide();
    m_ocrBtnWid->hide();

    changeQmlSizePos(this->width(), this->height(), 0, 0);

    if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
        m_showImageQml->resize(this->width(), this->height());
        m_showImageQml->move(0, 0);
    }

    m_showImage->resize(this->width(), this->height());
    m_showImage->move((this->width() - m_showImage->width()) / 2, (this->height() - m_showImage->height()) / 2);
    Interaction::getInstance()->changeWidgetSize(this->m_showImage->size());
    Q_EMIT viewExitOCRMode();
}

void ShowImageWidget::ocrMode()
{
    if (displayMode == DisplayMode::OCRMode) {
        m_showImage->resize(this->width() / 2, this->height() - m_widChangeHeight);
        changeQmlSizePos(this->width() / 2, this->height() - m_widChangeHeight, 0, m_widChangeHeight);
        m_ocrWid->resize(this->width() / 2, this->height() - m_widChangeHeight);
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width() / 2, this->height() - m_widChangeHeight);
            m_showImageQml->move(this->width() / 2, 0);
            m_showImage->move(this->width() / 2, m_widChangeHeight);
            m_ocrWid->move(0, m_widChangeHeight);
        } else {
            m_showImage->move(0, m_widChangeHeight);
            m_ocrWid->move(this->width() / 2, m_widChangeHeight);
        }
        m_ocrBtnWid->move((this->width() - m_ocrBtnWid->width()) / 2, this->height() - m_ocrBtnWid->height() - 8);
    } else {
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width(), this->height());
            m_showImageQml->move(0, 0);
        }
        changeQmlSizePos(this->width(), this->height(), 0, 0);
        m_showImage->resize(this->width(), this->height());
        m_showImage->move((this->width() - m_showImage->width()) / 2, (this->height() - m_showImage->height()) / 2);
    }
    m_ocrWid->resize(this->width() / 2, this->height());
    if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
        m_ocrWid->move(0, m_widChangeHeight);
    } else {
        m_ocrWid->move(this->width() / 2, m_widChangeHeight);
    }
    m_ocrBtnWid->move((this->width() - m_ocrBtnWid->width()) / 2, this->height() - m_ocrBtnWid->height() - 8);
    Interaction::getInstance()->changeWidgetSize(this->m_showImage->size());
}

void ShowImageWidget::exportToTxt()
{
    QString content = m_ocrWid->toPlainText();
    QFileDialog fileDialog(this, Qt::CustomizeWindowHint);
    QFileInfo filename(m_imagePath);
    QString savePath =
        fileDialog.getSaveFileName(this, QString(), m_path + "/" + filename.completeBaseName() + ".txt", ("*.txt"));
    QFileInfo info(savePath);
    if (savePath.isNull()) {
        return;
    }
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_msg->warning(this, tr("Warning"), tr("save file failed!"));
    } else {
        QTextStream stream(&file);
        stream << content;
        stream.flush();
        file.close();
    }
}

void ShowImageWidget::saveAsOther()
{
    QString format;
    //构造另存为文件类型
    format = Variable::SUPPORT_FORMATS_SAVEAS;

    QFileDialog fileDialog(this, Qt::CustomizeWindowHint); //设置窗口标题属性自定义

    QFileInfo filename(m_imagePath);

    QString savePath =
        fileDialog.getSaveFileName(nullptr, QString(), m_path + "/" + filename.completeBaseName() + ".png", format);
    QFileInfo file(savePath);
    if (savePath.isNull()) {
        return;
    }
    if (file.fileName().indexOf(".") == 0) {
        //        qDebug()<<"不能以.命名";
        m_msg->warning(this, tr("Warning"), tr("save fail.name cannot begin with \".\" "));
        return;
    }
    for (int i = 0; i < SPECIALCHAR.length(); i++) {
        if (file.fileName().contains(SPECIALCHAR.at(i))) {
            //            qDebug()<<"命名非法";
            //含非法文件名
            m_msg->warning(this, tr("Warning"), tr("the file name is illegal"));
            return;
        }
    }
    if (file.fileName() == "") {
        //        qDebug()<<"命名非法";
        //含非法文件名
        m_msg->warning(this, tr("Warning"), tr("the file name is illegal"));
        return;
    }
    //如果没有后缀，或者后缀不在list里，就用原格式为后缀
    if (!Variable::SUPPORT_FORMATS_END.contains("." + file.suffix().toLower())) {
        if (Variable::SUPPORT_FORMATS_NOT.contains("." + file.suffix().toLower())) {
            savePath += +".png";
        } else {
            savePath += +"." + m_paperFormat.toLower();
        }
    }
    Variable::g_needSaveas = true;
    Interaction::getInstance()->needSaveAs(savePath);
}
//打印结束
void ShowImageWidget::finishPrint(int result)
{
    //失败才打印
    if (1 != result) {
        qDebug() << "退出打印或打印失败！" << result;
    }
}
//删除
void ShowImageWidget::deleteImage()
{
    //    Interaction::getInstance()->deleteImage();
    Q_EMIT needDelete();
}
//在文件夹中显示
void ShowImageWidget::showInFile()
{
    Interaction::getInstance()->openImageInfile();
}
//重命名
void ShowImageWidget::reName()
{
    Q_EMIT toRename(1);
    Q_EMIT isRename();
}

void ShowImageWidget::signImage()
{
    Q_EMIT startSignToToolbar();
}

void ShowImageWidget::abnormalJudgBeforeClick()
{
    setMenuEnable();
    QFile fileImage(m_imagePath);
    QFile fileSignApp(QStandardPaths::findExecutable(SIGN_APP_NAME));
    if (!fileImage.exists()) {
        m_signImage->setEnabled(false);
        m_copy->setEnabled(false);
        m_setDeskPaper->setEnabled(false);
        m_print->setEnabled(false);
        //        m_deleteImage->setEnabled(false);
        m_showInFile->setEnabled(false);
        m_reName->setEnabled(false);
        m_saveAs->setEnabled(false);
    }
}
//显示右键菜单栏
void ShowImageWidget::setMenuAction()
{
    //拿到可设置为壁纸的图片格式
    QStringList formatList;
    QString format;
    format = "";
    for (const QString &str : Variable::BACKGROUND_SUPPORT_FORMATS) {
        format = str;
        formatList.append(format);
    }
    //判断是否为可设置为壁纸的类型
    if (formatList.contains(m_paperFormat)) {
        m_imageMenu->insertAction(m_showInFile, m_setDeskPaper);
        m_imageMenu->insertAction(m_setDeskPaper, m_reName);
    } else {
        m_imageMenu->removeAction(m_setDeskPaper);
        m_imageMenu->insertAction(m_showInFile, m_reName);
    }
    setMenuEnable();
}
//设置右键菜单是否选项可点击
void ShowImageWidget::setMenuEnable()
{

    m_copy->setEnabled(m_isOpenSuccess);
    m_setDeskPaper->setEnabled(m_isOpenSuccess);
    m_setLockPaper->setEnabled(m_isOpenSuccess);
    m_print->setEnabled(m_isOpenSuccess);
    m_signImage->setEnabled(m_isOpenSuccess && m_delOrNot);
    if (m_movieImage || m_paperFormat == "svg" || m_paperFormat == "dib" || m_paperFormat == "exr") {
        m_saveAs->setEnabled(false);
    } else {
        m_saveAs->setEnabled(m_isOpenSuccess);
    }

    if (m_delOrNot == false) {
        m_reName->setEnabled(m_delOrNot);
        m_deleteImage->setEnabled(m_delOrNot);
    } else {
        m_reName->setEnabled(m_isOpenSuccess);
        m_deleteImage->setEnabled(m_isOpenSuccess);
    }
    if (SIGN_NOT_SUPPORT.contains(m_paperFormat.toLower()) || m_movieImage) {
        m_signImage->setEnabled(false);
    }
}

void ShowImageWidget::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::startWithOpenImage, this,
            &ShowImageWidget::startWithOpenImage); //启动时打开图片
    connect(Interaction::getInstance(), &Interaction::openFinish, this,
            &ShowImageWidget::openFinish); //图片打开完成，获取数据
    connect(Interaction::getInstance(), &Interaction::delayShow, this, &ShowImageWidget::delayShow);
    connect(Interaction::getInstance(), &Interaction::changeDelState, this, &ShowImageWidget::canDelete);
    connect(Interaction::getInstance(), &Interaction::isCutNotSave, this,
            &ShowImageWidget::cutQueryWid); //弹出关闭询问弹窗
    //    connect(Interaction::getInstance(), &Interaction::exitOcrModeSign, this,
    //            &ShowImageWidget::viewExitOCRMode); //弹出关闭询问弹窗
}
//双击或带参数打开
void ShowImageWidget::startWithOpenImage(QString path)
{
    if (path != Variable::API_TYPE) {
        kdk::LingmoUIStyleHelper::self()->removeHeader(KyView::mutual);			
        KyView::mutual->show();
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            kdk::WindowManager::setGeometry(KyView::mutual->windowHandle(), 
					QRect(KyView::mutual->leftPoint, QSize(KyView::mutual->width(), KyView::mutual->height())));
        }
        Q_EMIT decideMainWidDisplay(true);
    }
    Q_EMIT startWayToSetTitleStyle();
    if (path == "") {
        Q_EMIT decideMainWidDisplay(true);
        return;
    } else {
        Q_EMIT decideMainWidDisplay(false);
    }
    operayteMode = OperayteMode::ChangeImage;
    //双击和右键打开图片，将路径写入
    QFileInfo info(path);
    Variable::setSettings("imagePath", info.absolutePath());
    openImage(path);
}
//从权限判断是否可删除
void ShowImageWidget::canDelete(bool delOrNot)
{
    m_delOrNot = delOrNot;
}
//展示工具栏
void ShowImageWidget::showBthWid(bool isShow)
{
    if (isShow) {
        m_btnWid->show();
        m_btnWid->move((this->width() - m_btnWid->width()) / 2, this->height() - m_btnWid->height() - 8);
        return;
    }
    m_btnWid->hide();
}

int ShowImageWidget::positionCorrection(int windowSize, int pictureSize)
{
    int actualPos = windowSize / 2 - pictureSize / 2;
    if (actualPos < 0) {
        actualPos = 0;
    }
    return actualPos;
}
//退出裁剪
void ShowImageWidget::exitCut()
{
    operayteMode = OperayteMode::ExitCut;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_cutWid->hide();
    m_btnWid->hide();
    Interaction::getInstance()->exitCut();
    //发信号给主界面，告知退出裁剪状态
    m_isCutting = false; //可进行放大缩小
    Q_EMIT needExitCut();
}
//保存裁剪图片并退出
void ShowImageWidget::needSave()
{
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    m_cutWid->hide();
    m_btnWid->hide();
    m_showImage->hide();
    //备份保存
    Interaction::getInstance()->needSaveCutImage(m_proportion);
    //弹出保存询问
    //    exitCut();
    m_isCutting = false; //可进行放大缩小
    Q_EMIT needExitCut();
}

bool ShowImageWidget::btnWidState()
{
    if (m_btnWid != nullptr) {
        if (m_btnWid->isHidden()) {
            return false;
        } else {
            return true;
        }
    }
}
//打开图片
void ShowImageWidget::openImage(QString path)
{
    if (displayMode == DisplayMode::OCRMode) {
        exitGetText();
    }
    Interaction::getInstance()->openImage(path);
}
//拿到图片信息，进行处理
void ShowImageWidget::openFinish(QVariant var)
{
    ImageAndInfo package = var.value<ImageAndInfo>();
    m_isOpenSuccess = package.openSuccess;
    QPixmap pixmap = package.image; //图片
    m_pic = pixmap;

    m_displaySizeOfPicture = package.displaySizeOfPicture;
    m_actualClickPos = package.actualClickPos;
    //拿到返回信息
    QFileInfo info = package.info;       //详情信息
    int proportion = package.proportion; //比例
    m_proportion = proportion;
    m_imageSize = package.imageSize;
    QString colorSpace = package.colorSpace;
    QString num;
    int number = package.imageNumber; //在队列中的标签
    m_movieImage = package.movieImage;
    m_paperFormat = package.realFormat;
    if (MOVIE_IMAGE_FORMAT.contains(m_paperFormat)) {
        m_movieImage = true;
    }
    m_imagePath = info.absoluteFilePath();
    m_imagePageList.clear();
    m_imagePageList = package.imageList;
    if (m_imagePageList.length() > 1 && (m_paperFormat == "tiff" || m_paperFormat == "tif")) {
        //区分正常操作和两种切换图片的情况
        if ((m_backImagePath != m_imagePath || operayteMode == OperayteMode::NextImage
             || operayteMode == OperayteMode::BackImage || operayteMode == OperayteMode::ChangeImage)
            && !Variable::g_nextOrPreviousTiff) {
            Variable::g_tempImage = m_imagePageList.at(0);
            m_tiffPage = 1;
            m_pageNum->setText(QString::number(m_tiffPage) + "/" + QString::number(m_imagePageList.length()));
            m_imagePageWid->show();
            Interaction::getInstance()->qmlTiffImagePage(m_tiffPage);
            if (m_imagePageList.length() > 1) {
                for (int i = 0; i < m_imagePageList.length(); i++) {
                    Variable::setLoadedImageMap(m_imagePath + QString::number(i), m_imagePageList.at(i));
                }
            }
        } else {
            Variable::g_nextOrPreviousTiff = false;
        }
        QMetaObject::invokeMethod(m_qmlObj, "setCurrentPath", Q_ARG(QVariant, m_imagePath),
                                  Q_ARG(QVariant, "multiTiff"));
        Variable::g_currentImageType = "multiTiff";

    } else {
        m_tiffPage = 1;
        Interaction::getInstance()->qmlTiffImagePage(m_tiffPage);
        Variable::g_tempImage = package.originImage.toImage();
        m_imagePageWid->hide();
        QMetaObject::invokeMethod(m_qmlObj, "setCurrentPath", Q_ARG(QVariant, m_imagePath), Q_ARG(QVariant, "normal"));
        Variable::g_currentImageType = "normal";
    }

    Variable::g_currentPath = m_imagePath;
    InteractiveQml::getInstance()->setImageFormat(m_paperFormat);
    m_imageNeedUpdate = package.imageNeedUpdate;
    m_imageShowWay = package.imageShowWay;
    m_zoomFocus = package.zoomFocus;
    m_leftUpPos = package.leftUpPos;
    if (imageNum(number) && !Variable::g_hasIllegalSuffix) {
        return;
    }
    if (Variable::g_delOpenNext) {
        Variable::g_delOpenNext = false;
        if (number == 1) {
            Q_EMIT clearImage();
            m_isDelete = false;
            return;
        }
    }
    num = QString("%1").arg(proportion) + "%";
    m_path = info.absolutePath(); //图片的路径
    m_imagePath = info.absoluteFilePath();
    m_paperFormat = package.realFormat;
    m_typeNum = number;
    Q_EMIT sendSuffixToToolbar(m_paperFormat);
    //根据number决定界面显示

    //重置ocr导出文本按钮的状态
    if (m_ocrExportToTxt->isEnabled()) {
        m_ocrExportToTxt->setEnabled(false);
    }
    //使用返回的信息进行设置界面
    Q_EMIT toShowImage(); //给主界面--展示图片

    //根据图片类型刷新右键菜单内容
    imageMenu(info, m_imageSize, colorSpace);
    sideState(m_typeNum);
    m_isDelete = false;
    // qml加载图片前的判断
    if (operatorBeforeShowImage(pixmap)) {
        return;
    }
    bool nullImage = false;
    if (pixmap.isNull()) {
        nullImage = true;
    }
    //更新加载图片的map
    if (!m_movieImage && Variable::g_currentImageType != "multiTiff") {
        Variable::setLoadedImageMap(m_imagePath, Variable::g_tempImage);
    }
    if (package.imageDelPath != "") {
        Variable::delLoadedImage(package.imageDelPath);
    }
    bool tiffOperate = false;
    if (Variable::g_changedImageList.contains(m_imagePath)) {
        tiffOperate = true;
    }
    m_backImagePath = m_imagePath;
    InteractiveQml::getInstance()->setImageDelPath(package.imageDelPath);
    InteractiveQml::getInstance()->setLoadImageList(package.loadImageList);
    InteractiveQml::getInstance()->setCurrentImageIndex(package.currentImageIndex);
    InteractiveQml::getInstance()->setShowImageIndex(package.showImageIndex);
    InteractiveQml::getInstance()->setImageTypeMovieOrNormal(package.imageTypeMovieOrNormal);
    InteractiveQml::getInstance()->setImageList(package.allImagePathList, package.imageTypeList, nullImage,
                                                tiffOperate);
    Q_EMIT perRate(QString::number(m_proportion) + "%", m_imagePath);
    this->operatyImage();
}
//拉伸主界面时重新安排界面显示
void ShowImageWidget::reMove(int W, int H)
{
    //拉伸主界面时重新安排界面显示
    this->resize(W, H);

    if (displayMode == DisplayMode::OCRMode) {
        m_showImage->resize(this->width() / 2, this->height() - m_widChangeHeight);
        changeQmlSizePos(this->width() / 2, this->height() - m_widChangeHeight, 0, m_widChangeHeight);
        m_ocrWid->resize(this->width() / 2, this->height() - m_widChangeHeight);
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width() / 2, this->height() - m_widChangeHeight);
            m_showImageQml->move(this->width() / 2, 0);
            m_showImage->move(this->width() / 2, m_widChangeHeight);
            m_ocrWid->move(0, m_widChangeHeight);
        } else {
            m_showImage->move(0, m_widChangeHeight);
            m_ocrWid->move(this->width() / 2, m_widChangeHeight);
        }
        m_ocrBtnWid->move((this->width() - m_ocrBtnWid->width()) / 2, this->height() - m_ocrBtnWid->height() - 8);
    } else {
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width(), this->height());
            m_showImageQml->move(0, 0);
        }
        m_qmlObj->setProperty(IMAGE_WIDTH.toLatin1().data(), this->width());
        m_qmlObj->setProperty(IMAGE_HEIGHT.toLatin1().data(), this->height());
        m_showImage->resize(this->width(), this->height());
        m_showImage->move((this->width() - m_showImage->width()) / 2, (this->height() - m_showImage->height()) / 2);
    }
    g_back->move(LEFT_POS, int((this->height() - g_back->height()) / 2));
    g_next->move(W - LEFT_POS - g_next->width(), int((H - g_next->height()) / 2));

    m_imagePageWid->move(this->width() / 2 - m_imagePageWid->width() / 2,
                         this->height() - m_imagePageWid->height() - 66);
    if (m_firstChangeSize) {
        return;
    }
    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;

    int imageType = 0;
    if (m_movieImage && m_paperFormat != "tif" && m_paperFormat != "tiff") {
        imageType = 1;
    }
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, imageType, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }
    bool needRecord = false;
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_ADJUSTIMAGEPOS.toLatin1().data(), Q_ARG(QVariant, needRecord));
    QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_UPDATE_POS_ACCORD_WIDSIZE_CHANGE.toLatin1().data(),
                              Q_ARG(QVariant, needRecord));
}

void ShowImageWidget::resizeEvent(QResizeEvent *event)
{
    if (displayMode == DisplayMode::OCRMode) {
        m_showImage->resize(this->width() / 2, this->height() - m_widChangeHeight);
        changeQmlSizePos(this->width(), this->height(), 0, m_widChangeHeight);
        m_ocrWid->resize(this->width() / 2, this->height() - m_widChangeHeight);
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width() / 2, this->height() - m_widChangeHeight);
            m_showImageQml->move(this->width() / 2, 0);
            m_showImage->move(this->width() / 2, m_widChangeHeight);
            m_ocrWid->move(0, m_widChangeHeight);
        } else {
            m_showImage->move(0, m_widChangeHeight);
            m_ocrWid->move(this->width() / 2, m_widChangeHeight);
        }
        m_ocrBtnWid->move((this->width() - m_ocrBtnWid->width()) / 2, this->height() - m_ocrBtnWid->height() - 8);
    } else {
        if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
            m_showImageQml->resize(this->width(), this->height());
            m_showImageQml->move(0, 0);
        }
        changeQmlSizePos(this->width(), this->height(), 0, 0);
        m_showImage->resize(this->width(), this->height());
        m_showImage->move((this->width() - m_showImage->width()) / 2, (this->height() - m_showImage->height()) / 2);
    }

    //只有首次的时候才会去自适应图片大小
    if (m_firstChangeSize) {
        Interaction::getInstance()->changeWidgetSize(this->m_showImage->size());
        m_firstChangeSize = false;
    }
    //    return QWidget::resizeEvent(event);
}
//滚轮放大缩小
bool ShowImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    //裁剪后使qmenu隐藏
    if (displayMode != DisplayMode::NormalMode) {
        m_imageMenu->hide();
    }
    if (obj == m_cutWid && QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
        switch (event->type()){
        case QEvent::TouchBegin:
            return m_cutWid->touchBeginEventProcess(event);
        case QEvent::TouchUpdate:
            return m_cutWid->touchUpdateEventProcess(event);
        case QEvent::TouchEnd:
            return m_cutWid->touchEndEventProcess(event);
        }
    }
    return QObject::eventFilter(obj, event);
}

//裁剪的工具栏按钮样式设置
void ShowImageWidget::cutBtnWidStyle()
{
    m_save->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/save.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/save_hover.png);}");
    m_cancel->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;border-image: url(:/res/res/cancel.png);}"
        "QPushButton::hover{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}"
        "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;border-image: "
        "url(:/res/res/cancel_hover.png);}");
}
// OCR的工具栏按钮样式设置
void ShowImageWidget::ocrBtnWidStyle()
{
    m_ocrExportToTxt->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
                                    "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
                                    "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}");
}

void ShowImageWidget::stateUpdate()
{
    //判断图片是否打开失败或者为无权限修改的图片

    Q_EMIT fileState(m_isOpenSuccess, m_delOrNot);
}

int ShowImageWidget::makeSureHeightState()
{
    int widHeight = BAR_HEIGHT;
    //    if (platForm.contains(Variable::platForm)) {
    //        widHeight = BAR_HEIGHT_INTEL;
    //    }
    return widHeight;
}

void ShowImageWidget::initQmlObject()
{
    //设置位置和大小
    m_showImageQml = new QQuickWidget(this);
    QQmlEngine *engine = m_showImageQml->engine();
    QMutex *mutex = new QMutex();
    engine->addImageProvider(QLatin1String("thumb"), new PQAsyncImageProviderThumb(mutex));
    engine->addImageProvider(QLatin1String("main"), new ImageProvider);
    qmlRegisterType<InteractiveQml>("Interactive", 1, 0, "Interactive");
    //    qmlRegisterSingletonType<InteractiveQml>("InteractiveQml", 1, 0, "interactiveQml",
    //    InteractiveQml::getInstance());
    m_showImageQml->setSource(QUrl(QStringLiteral("qrc:/showImageComponent.qml")));

    m_showImageQml->setClearColor(Qt::transparent);
    m_showImageQml->installEventFilter(this);
    m_showImageQml->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_showImageQml->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showImageQml->lower();
    // bug 146268的规避方案
    connect(m_showImageQml->quickWindow(), &QQuickWindow::activeFocusItemChanged, this,
            &ShowImageWidget::qmlActiveFocusChanged);

    m_qmlObj = new QObject(this);
    m_qmlObj = m_showImageQml->rootObject();


    //操作方式
    connect(InteractiveQml::getInstance(), SIGNAL(operateWayChanged(QVariant)), m_qmlObj,
            SLOT(setOperateWay(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(imageFormatChanged(QVariant)), m_qmlObj, SLOT(setFormat(QVariant)));
    connect(m_qmlObj, SIGNAL(sendDropImagePath(QVariant)), this, SLOT(getDropImagePath(QVariant)));
    connect(m_qmlObj, SIGNAL(changeImage(QVariant)), this, SLOT(needChangeImage(QVariant)));
    connect(m_qmlObj, SIGNAL(doubleSignal()), this, SIGNAL(doubleEventToMainWid()));

    connect(m_qmlObj, SIGNAL(restoreOrig()), this, SIGNAL(adaptiveWidgetSignal()));
    connect(m_qmlObj, SIGNAL(mousePressOrRelease()), this, SIGNAL(sendMousePressOrNot()));
    //操作方式
    connect(InteractiveQml::getInstance(), SIGNAL(operateWayChanged(QVariant)), m_qmlObj,
            SLOT(setOperateWay(QVariant)));
    //画笔类型及其各种参数
    connect(InteractiveQml::getInstance(), SIGNAL(painterTypeChanged(QVariant)), m_qmlObj,
            SLOT(setpainterType(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(painterThicknessChanged(QVariant)), m_qmlObj,
            SLOT(setpainterThickness(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(painterColorChanged(QVariant)), m_qmlObj,
            SLOT(setpainterColor(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textBoldChanged(QVariant)), m_qmlObj, SLOT(setBold(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textDeleteLineChanged(QVariant)), m_qmlObj,
            SLOT(setDeleteLine(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textUnderLineChanged(QVariant)), m_qmlObj,
            SLOT(setUnderLine(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textItalicsChanged(QVariant)), m_qmlObj, SLOT(setItalics(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textFontTypeChanged(QVariant)), m_qmlObj,
            SLOT(setFontType(QVariant)));
    connect(InteractiveQml::getInstance(), SIGNAL(textFontSizeChanged(QVariant)), m_qmlObj,
            SLOT(setFontSize(QVariant)));

    //滑动动效的信号槽

    connect(InteractiveQml::getInstance(), SIGNAL(updateImageDelPath(QVariant)), m_qmlObj,
            SLOT(updateImageDelPath(QVariant)));


    connect(InteractiveQml::getInstance(), SIGNAL(updateLoadImageList(QVariant)), m_qmlObj,
            SLOT(updateLoadImageList(QVariant)));

    connect(InteractiveQml::getInstance(), SIGNAL(updateCurrentImageIndex(QVariant)), m_qmlObj,
            SLOT(updateCurrentImageIndex(QVariant)));


    connect(InteractiveQml::getInstance(), SIGNAL(updateShowImageIndex(QVariant)), m_qmlObj,
            SLOT(updateShowImageIndex(QVariant)));

    connect(InteractiveQml::getInstance(), SIGNAL(updateImageTypeMovieOrNormal(QVariant)), m_qmlObj,
            SLOT(updateImageType(QVariant)));

    connect(InteractiveQml::getInstance(), SIGNAL(updateImageList(QVariant, QVariant, QVariant, QVariant)), m_qmlObj,
            SLOT(updateImageList(QVariant, QVariant, QVariant, QVariant)));


    connect(m_qmlObj, SIGNAL(changeImageFromView(QVariant, QVariant)), this,
            SLOT(changeImageFromViewSwipe(QVariant, QVariant)));

    connect(Interaction::getInstance(), &Interaction::getLeftUpPosAccordingImage, this, &ShowImageWidget::setImagePos);
    changeQmlSizePos(this->width(), this->height(), 0, 0);
    connect(m_qmlObj, SIGNAL(copyShortCuts()), this, SLOT(copyFromQml()));
    //主题图标变化
    connect(InteractiveQml::getInstance(), SIGNAL(themeIconChanged(QVariant)), m_qmlObj,
            SLOT(setThemeIcon(QVariant)));
}

void ShowImageWidget::initQWidgetObject()
{
    //中间展示图片部分
    m_showImage = new QLabel(this);
    m_showImage->resize(this->width(), this->height());
    m_showImage->move(int((this->width() - m_showImage->width()) / 2),
                      int((this->height() - m_showImage->height()) / 2));
    m_showImage->setMouseTracking(true);
    m_showImage->installEventFilter(this);
    m_showImage->setAlignment(Qt::AlignCenter);
    m_showImage->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_showImage->setContextMenuPolicy(Qt::CustomContextMenu);
    m_showImage->hide();
}

void ShowImageWidget::initMenu()
{
    //菜单栏给功能选项：复制，设置为桌面壁纸，设置为锁屏壁纸，打印，删除，在文件夹中显示
    m_copy = new QAction(tr("Copy"), this);
    m_reName = new QAction(tr("Rename"), this);
    m_setDeskPaper = new QAction(tr("Set Desktop Wallpaper"), this);
    m_setLockPaper = new QAction(tr("Set Lock Wallpaper"), this);
    m_print = new QAction(tr("Print"), this);
    m_deleteImage = new QAction(tr("Delete"), this);
    m_showInFile = new QAction(tr("Show in File"), this);
    m_saveAs = new QAction(tr("Save as"), this);
    m_signImage = new QAction(tr("Markup"), this);
    //右键菜单
    m_imageMenu = new QMenu(m_showImageQml);
    m_imageMenu->addAction(m_copy);
    //    m_imageMenu->addAction(m_reName);
    //    m_imageMenu->addAction(m_setDeskPaper);
    //    imageMenu->addAction(setLockPaper);
    m_imageMenu->addAction(m_print);
    m_imageMenu->addAction(m_deleteImage);
    m_imageMenu->addAction(m_signImage);
    m_imageMenu->addAction(m_saveAs);
    m_imageMenu->addAction(m_showInFile);
}

void ShowImageWidget::initCutWid()
{
    m_cutWid = new CutWIdget(this);
    m_cutWid->installEventFilter(this);
    m_cutWid->hide();
    //需要将这个widget移动到showimagewidget内
    m_btnWid = new QWidget(this);
    m_btnWid->hide();
    m_btnWid->setFixedSize(108, 40);
    m_cancel = new QPushButton(m_btnWid);
    m_save = new QPushButton(m_btnWid);
    m_cancel->setFixedSize(24, 24);
    m_save->setFixedSize(24, 24);
    m_save->setToolTip(tr("Save"));
    m_cancel->setToolTip(tr("Cancel"));
    if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
        m_save->move(20, 8);
        m_cancel->move(30 + m_cancel->width() + 8, 8);
    } else {
        m_cancel->move(20, 8);
        m_save->move(30 + m_cancel->width() + 8, 8);
    }
    cutBtnWidStyle();
    m_btnWid->move((this->width() - m_btnWid->width()) / 2, this->height() - m_btnWid->height() - 8);
}

void ShowImageWidget::initChangeImageBtn()
{
    //上一张，下一张按钮
    g_next = new QPushButton(this);
    g_next->resize(IMAGE_BUTTON);
    g_next->setToolTip(tr("Next"));

    g_back = new QPushButton(this);
    g_back->resize(IMAGE_BUTTON);
    g_back->setToolTip(tr("Previous"));

    g_back->move(LEFT_POS, int((this->height() - g_back->height()) / 2));
    g_next->move(this->width() - LEFT_POS - g_next->width(), int((this->height() - g_next->height()) / 2));

    g_back->setIconSize(IMAGE_ICON);
    g_next->setIconSize(IMAGE_ICON);

    g_back->setFocusPolicy(Qt::NoFocus);
    g_next->setFocusPolicy(Qt::NoFocus);
    g_back->setFlat(true);
    g_next->setFlat(true);
    g_back->setStyleSheet("QPushButton{border:none;background-color:transparent;border-radius:4px;}");
    g_next->setStyleSheet("QPushButton{border:none;background-color:transparent;border-radius:4px;}");
}

void ShowImageWidget::initOcrWid()
{
    m_ocrWid = new OCRResultWidget(this);
    m_ocrWid->hide();
    m_ocrBtnWid = new QWidget(this);
    m_ocrBtnWid->hide();
    m_ocrBtnWid->setFixedSize(148, 40);
    m_ocrCancel = new kdk::KBorderlessButton(m_ocrBtnWid);
    m_ocrCancel->setFocusPolicy(Qt::NoFocus); //设置焦点类型
                                              //    m_ocrCancel->setProperty("isWindowButton", 0x1);
                                              //    m_ocrCancel->setProperty("useIconHighlightEffect", 0x2);
    m_ocrCancel->setFlat(true);
    m_ocrCancel->setFixedSize(24, 24);
    m_ocrCancel->setToolTip(tr("Cancel"));
    m_ocrExportToTxt = new QPushButton(m_ocrBtnWid);
    m_ocrExportToTxt->setText(tr("Export"));
    m_ocrExportToTxt->setProperty("isWindowButton", 0x1);
    m_ocrExportToTxt->setProperty("useIconHighlightEffect", 0x2);
    m_ocrExportToTxt->setFlat(true);
    m_ocrExportToTxt->setFixedSize(60, 24);
    m_ocrExportToTxt->setEnabled(false);
    if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
        m_ocrExportToTxt->move(24, 8);
        m_ocrCancel->move(104, 8);
    } else {
        m_ocrCancel->move(20, 8);
        m_ocrExportToTxt->move(64, 8);
    }
}

void ShowImageWidget::changeQmlSizePos(int w, int h, int x, int y)
{
    m_qmlObj->setProperty(IMAGE_WIDTH.toLatin1().data(), w);
    m_qmlObj->setProperty(IMAGE_HEIGHT.toLatin1().data(), h);
    m_qmlObj->setProperty(IMAGE_X.toLatin1().data(), x);
    m_qmlObj->setProperty(IMAGE_Y.toLatin1().data(), y);

    QMetaObject::invokeMethod(m_qmlObj, IMAGE_FUNC_RELOADIMAGE.toLatin1().data());
}

void ShowImageWidget::setImagePos(QPointF leftUpPos)
{
    QObject *qmlImageRect;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect)) {
        return;
    }
    if (qmlImageRect->property(IMAGE_TYPE.toLatin1().data()) == 0) {

        QObject *imageLoader;
        QObject *imageItemLower;
        if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower)) {
            return;
        }
        QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_NAVICHANGEIMAGEPOS.toLatin1().data(),
                                  Q_ARG(QVariant, leftUpPos));
    } else {
        QObject *imageLoader;
        QObject *imageItemLower;
        if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 1, &imageLoader, &imageItemLower)) {
            return;
        }
        QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_NAVICHANGEIMAGEPOS.toLatin1().data(),
                                  Q_ARG(QVariant, leftUpPos));
    }
}

bool ShowImageWidget::getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, int isNormal,
                                               QObject **objSecond, QObject **objThird, QObject **objFourth)
{

    if (objParent == nullptr) {
        qDebug() << "父对象为空";
        return false;
    }
    QObject *tmpFirst1 = objParent->findChild<QObject *>("coverWid");
    if (tmpFirst1 == nullptr) {
        qDebug() << "imageItem1对象为空";
        return false;
    }

    QObject *tmpFirst2 = tmpFirst1->findChild<QObject *>("pathWid");
    if (tmpFirst2 == nullptr) {
        qDebug() << "imageItem2对象为空";
        return false;
    }
    QObject *tmpFirst = tmpFirst2->property("currentItem").value<QObject *>();

    if (tmpFirst == nullptr) {
        qDebug() << "imageItem对象为空";
        return false;
    }
    *objFirst = tmpFirst;

    if (isNormal == -1) {
        return true;
    }

    tmpFirst->setProperty(IMAGE_TYPE.toLatin1().data(), isNormal);

    QObject *tmpSecond = tmpFirst->findChild<QObject *>("imageLoader");
    if (tmpSecond == nullptr) {
        qDebug() << "imageLoader对象为空";
        return false;
    }
    *objSecond = tmpSecond;

    QObject *tmpThird;
    if (!isNormal) {
        tmpThird = tmpSecond->findChild<QObject *>("imageItemLower");
    } else {
        tmpThird = tmpSecond->findChild<QObject *>("movieItemLower");
    }
    if (tmpThird == nullptr) {
        qDebug() << "movieItemLower/imageItemLower对象为空" << isNormal;
        return false;
    }
    *objThird = tmpThird;

    QObject *tmpFourth = tmpThird->findChild<QObject *>("image");

    if (tmpFourth == nullptr) {
        qDebug() << "image对象为空";
        return false;
    }

    if (objFourth != nullptr) {
        *objFourth = tmpFourth;
    }

    return true;
}

void ShowImageWidget::realBuriedPoint()
{
    //缩放图片-埋点
    kdk::kabase::BuriedPoint buriedPointZoomInandOut;
    if (buriedPointZoomInandOut.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerCurrentPointZoom)) {
        qCritical() << "buried point fail CurrentPointZoom!";
    }
}

void ShowImageWidget::qmlActiveFocusChanged()
{
    QQuickItem *focusItem = m_showImageQml->quickWindow()->activeFocusItem();
    if (focusItem && focusItem->inherits("QQuickTextEdit")) {
        disconnect(m_showImageQml->quickWindow(), &QQuickWindow::activeFocusItemChanged, this,
                   &ShowImageWidget::qmlActiveFocusChanged);
        clearFocus();
        focusItem->forceActiveFocus();
    }
}

void ShowImageWidget::qmlAndWidgetConnectNormal(QObject *obj)
{
    connect(this, SIGNAL(undoSign()), obj, SLOT(undoSignOperate()), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendScale(QVariant)), this, SLOT(setScale(QVariant)));
    connect(obj, SIGNAL(needShowNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)), this,
            SLOT(showNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)));
    connect(obj, SIGNAL(closeNavigator()), this, SLOT(needCloseNavigator()));
    connect(obj, SIGNAL(sendRectInfo(QVariant)), this, SLOT(rectPaint(QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendCircleInfo(QVariant, QVariant, QVariant, QVariant)), this,
            SLOT(circlePaint(QVariant, QVariant, QVariant, QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendLineInfo(QVariant, QVariant)), this, SLOT(linePaint(QVariant, QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendArrowInfo(QVariant, QVariant, QVariant, QVariant)), this,
            SLOT(arrowPaint(QVariant, QVariant, QVariant, QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendPencilInfo(QVariant)), this, SLOT(pencilPaint(QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendMarkInfo(QVariant, QVariant, QVariant)), this,
            SLOT(markPaint(QVariant, QVariant, QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendTextInfo(QVariant, QVariant, QVariant)), this,
            SLOT(textPaint(QVariant, QVariant, QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(sendBlurInfot(QVariant)), this, SLOT(blurPaint(QVariant)), Qt::UniqueConnection);
    connect(obj, SIGNAL(buriedPoint()), this, SLOT(zoomBuriedPoint()));
}

void ShowImageWidget::qmlAndWidgetDisconnectNormal()
{
    QObject *qmlImageRect;
    QObject *imageLoader;
    QObject *imageItemLower;
    QObject *targatImage;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower, &targatImage)) {
        return;
    }
    disconnect(this, SIGNAL(undoSign()), imageItemLower, SLOT(undoSignOperate()));
    disconnect(imageItemLower, SIGNAL(sendRectInfo(QVariant)), this, SLOT(rectPaint(QVariant)));
    disconnect(imageItemLower, SIGNAL(sendCircleInfo(QVariant, QVariant, QVariant, QVariant)), this,
               SLOT(circlePaint(QVariant, QVariant, QVariant, QVariant)));
    disconnect(imageItemLower, SIGNAL(sendLineInfo(QVariant, QVariant)), this, SLOT(linePaint(QVariant, QVariant)));
    disconnect(imageItemLower, SIGNAL(sendArrowInfo(QVariant, QVariant, QVariant, QVariant)), this,
               SLOT(arrowPaint(QVariant, QVariant, QVariant, QVariant)));
    disconnect(imageItemLower, SIGNAL(sendPencilInfo(QVariant)), this, SLOT(pencilPaint(QVariant)));
    disconnect(imageItemLower, SIGNAL(sendMarkInfo(QVariant, QVariant, QVariant)), this,
               SLOT(markPaint(QVariant, QVariant, QVariant)));
    disconnect(imageItemLower, SIGNAL(sendTextInfo(QVariant, QVariant, QVariant)), this,
               SLOT(textPaint(QVariant, QVariant, QVariant)));
    disconnect(imageItemLower, SIGNAL(sendBlurInfot(QVariant)), this, SLOT(blurPaint(QVariant)));
}

void ShowImageWidget::qmlAndWidgetConnectMovie(QObject *obj)
{
    //    if (m_hasConnectedMovie) {
    //        return;
    //    }
    //    m_hasConnectedMovie = true;
    connect(obj, SIGNAL(sendScale(QVariant)), this, SLOT(setScale(QVariant)));
    connect(obj, SIGNAL(needShowNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)), this,
            SLOT(showNavigator(QVariant, QVariant, QVariant, QVariant, QVariant)));
    connect(obj, SIGNAL(closeNavigator()), this, SLOT(needCloseNavigator()));
    connect(obj, SIGNAL(buriedPoint()), this, SLOT(zoomBuriedPoint()));
}

void ShowImageWidget::zoomBuriedPoint()
{
    if (m_timer->isActive()) {
        return;
    }
    m_timer->start(1000);
}
void ShowImageWidget::needExitApp()
{
    m_programeIsClosing = true;
}
void ShowImageWidget::needCloseNavigator()
{
    m_naviState = false;
    Q_EMIT toCloseNavigator(QPixmap());
}

void ShowImageWidget::needUpdateImagePosFromMainWid(bool needUpdate)
{
    m_needUpdateImagePosFromMainWid = needUpdate;
}

void ShowImageWidget::needChangeImage(QVariant needChangeImage)
{
    if (displayMode != DisplayMode::NormalMode) {
        return;
    }
    Variable::g_nextOrPreviousTiff = false;
    m_canSet = true;
    if (needChangeImage.toBool()) {
        operayteMode = OperayteMode::BackImage;
        Interaction::getInstance()->backImage();
    } else {
        operayteMode = OperayteMode::NextImage;
        Interaction::getInstance()->nextImage();
    }
}

void ShowImageWidget::noticeQmlAdjustImagePos(bool needRecord)
{

    QObject *qmlImageRect;
    if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect)) {
        return;
    }
    if (qmlImageRect->property(IMAGE_TYPE.toLatin1().data()) == 0) {

        QObject *imageLoader;
        QObject *imageItemLower;
        if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 0, &imageLoader, &imageItemLower)) {
            return;
        }

        QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_ADJUSTIMAGEPOS.toLatin1().data(),
                                  Q_ARG(QVariant, needRecord));
    } else {
        QObject *imageLoader;
        QObject *imageItemLower;
        if (!getQmlObjAndJudgeNullptr(m_qmlObj, &qmlImageRect, 1, &imageLoader, &imageItemLower)) {
            return;
        }
        QMetaObject::invokeMethod(imageItemLower, IMAGE_FUNC_ADJUSTIMAGEPOS.toLatin1().data(),
                                  Q_ARG(QVariant, needRecord));
    }
}

void ShowImageWidget::rectPaint(QVariant rect)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.rectInfo = rect.toRect();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::circlePaint(QVariant leftX, QVariant leftY, QVariant painterW, QVariant painterH)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();
    recordDate.circleLeftX = leftX.toDouble();
    recordDate.circleLeftY = leftY.toDouble();
    recordDate.paintWidth = painterW.toDouble();
    recordDate.paintHeight = painterH.toDouble();
    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::linePaint(QVariant startPos, QVariant endPos)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    recordDate.recordStartPos = startPos.toPoint();
    recordDate.recordEndPos = endPos.toPoint();
    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::arrowPaint(QVariant startPos, QVariant endPos, QVariant arrowStartPos, QVariant arrowEndPos)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    recordDate.recordStartPos = startPos.toPoint();
    recordDate.recordEndPos = endPos.toPoint();
    recordDate.arrowStartPos = arrowStartPos.toPoint();
    recordDate.arrowEndPos = arrowEndPos.toPoint();

    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::pencilPaint(QVariant pointList)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    QVector<QPoint> vp;
    QList<QVariant> lp = pointList.toList();
    if (lp.length() > 0) {
        for (int i = 0; i < lp.length(); i++) {
            vp.append(lp.at(i).toPoint());
        }
        recordDate.vp = vp;
        QVariant var;
        var.setValue<MarkPainterSet>(recordDate);
        Interaction::getInstance()->toMarkImage(var);
    } else {
        qDebug() << "绘图失败";
        return;
    }
}

void ShowImageWidget::markPaint(QVariant startPos, QVariant endPos, QVariant alpha)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    recordDate.recordStartPos = startPos.toPoint();
    recordDate.recordEndPos = endPos.toPoint();
    recordDate.opactity = alpha.toDouble();

    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::textPaint(QVariant textContent, QVariant startPos, QVariant type)
{
    if ("" == textContent.toString()) {
        qDebug() << "文字内容为空";
        return;
    }
    MarkPainterSet recordDate;
    recordDate.type = type.toInt();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    recordDate.m_textSet.textContent = textContent.toString();
    recordDate.recordStartPos = startPos.toPoint();
    recordDate.m_textSet.fontSize = InteractiveQml::getInstance()->getPainterTextFontSize();
    if ("" == InteractiveQml::getInstance()->getPainterTextFontType()) {
        QFont f;
        recordDate.m_textSet.fontType = f.family();
    } else {
        recordDate.m_textSet.fontType = InteractiveQml::getInstance()->getPainterTextFontType();
    }
    recordDate.m_textSet.isBold = InteractiveQml::getInstance()->getPainterTextBold();
    recordDate.m_textSet.isItalics = InteractiveQml::getInstance()->getPainterTextItalics();
    recordDate.m_textSet.isUnderLine = InteractiveQml::getInstance()->getPainterTextUnderLine();
    recordDate.m_textSet.isDeleteLine = InteractiveQml::getInstance()->getPainterTextDeleteLine();
    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::blurPaint(QVariant blurRect)
{
    MarkPainterSet recordDate;
    recordDate.type = InteractiveQml::getInstance()->getPainterType();
    recordDate.painterColor = InteractiveQml::getInstance()->getPainterColor();
    recordDate.thickness = InteractiveQml::getInstance()->getPainterThickness();

    recordDate.rectInfo = blurRect.toRect();
    QVariant var;
    var.setValue<MarkPainterSet>(recordDate);
    Interaction::getInstance()->toMarkImage(var);
}

void ShowImageWidget::showNavigator(QVariant scale, QVariant leftUpImageX, QVariant leftUpImageY, QVariant visibleWidth,
                                    QVariant visibleHeight)
{
    m_naviState = true;

    if (!m_needUpdateImagePosFromMainWid) {
        return;
    }
    if (m_movieImage && (m_paperFormat == "tif" || m_paperFormat == "tiff")) {
        Interaction::getInstance()->qmlTiffPageNavi(
            m_tiffPage, QPointF(leftUpImageX.toDouble(), leftUpImageY.toDouble()),
            QSizeF(visibleWidth.toDouble(), visibleHeight.toDouble()), scale.toDouble());
    } else {
        //给后端发送缩放比和鼠标位置左上角坐标
        Interaction::getInstance()->qmlStartNaviDisplay(QPointF(leftUpImageX.toDouble(), leftUpImageY.toDouble()),
                                                        QSizeF(visibleWidth.toDouble(), visibleHeight.toDouble()),
                                                        scale.toDouble());
    }

    Q_EMIT needGrabGestureOut(false);
}
void ShowImageWidget::navigatorState(bool state)
{
    m_navigatorState = state;
}

void ShowImageWidget::navigatorMove(bool isDrageImage)
{
    m_isDragImage = isDrageImage;
}
void ShowImageWidget::changeQmlcomponentColor(QColor color, double opacity)
{
    m_qmlObj->setProperty(IMAGE_COLOR.toLatin1().data(), color);
}

void ShowImageWidget::undoOperate()
{
    Q_EMIT undoSign();
}

void ShowImageWidget::exitSign()
{
    Variable::g_allowChangeImage = true;
    Variable::g_needImageSaveTip = false;
    displayMode = DisplayMode::NormalMode;
    operayteMode = OperayteMode::NormalMode;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //返回当前缩放比
    //暂时不需要，下一个阶段需要，目前阶段直接退出不保存即可
    QMetaObject::invokeMethod(m_qmlObj, IMAGE_FUNC_SIGN_EXITSIGN.toLatin1().data());
    //    Interaction::getInstance()->markClear();
    //    QVariant var;
    //    QMetaObject::invokeMethod(m_qmlObj, IMAGE_FUNC_SIGN_GETOPERATETIME.toLatin1().data(), Q_RETURN_ARG(QVariant,
    //    var)); if (var.toInt() != 0) {
    //        qDebug() << "弹出弹窗询问是否保存";
    //        cutQueryWid();
    //    }
}

void ShowImageWidget::finishSign()
{
    //具体需不需要弹窗提示，需要和设计讨论-当前默认备份保存
    QVariant var;
    QMetaObject::invokeMethod(m_qmlObj, IMAGE_FUNC_SIGN_GETOPERATETIME.toLatin1().data(), Q_RETURN_ARG(QVariant, var));
    if (var.toInt() != 0) {
        //发送给后端进行图片编辑
        //拿到图片操作队列
        Interaction::getInstance()->markSave(m_imagePath);
    } else {
        Q_EMIT exitSignComplete();
    }
    Variable::g_allowChangeImage = true;
    Variable::g_needImageSaveTip = false;
    displayMode = DisplayMode::NormalMode;
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    QMetaObject::invokeMethod(m_qmlObj, IMAGE_FUNC_SIGN_EXITSIGN.toLatin1().data());
}

void ShowImageWidget::setTiffToolbarPos(double posY)
{
    if (m_imagePageWid != nullptr) {
        m_imagePageWid->move(this->width() / 2 - m_imagePageWid->width() / 2, posY - m_imagePageWid->height());
    }
}

// bool ShowImageWidget::signSaveQuery()
//{
//    m_qmlObj->property("signListLength").toInt();
//}
//裁剪的工具栏窗口样式设置
void ShowImageWidget::cutBtnWidChange(QString themeTyep)
{
    if (KyView::mutual->m_locale == "kk_KZ" || KyView::mutual->m_locale =="ug_CN" || KyView::mutual->m_locale == "ky_KG") {
        m_ocrCancel->setIcon(QIcon::fromTheme("lingmo-end-symbolic"));
    } else {
        m_ocrCancel->setIcon(QIcon::fromTheme("lingmo-start-symbolic"));
    }
    if ("lingmo-dark" == themeTyep || "lingmo-black" == themeTyep) {
        m_btnWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.72);border-radius:6px;}");
        m_ocrBtnWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,0.72);border-radius:6px;}");
        //        m_ocrCancel->setIcon(QIcon(":/res/res/ocrCancel_B.png"));
    } else {
        m_btnWid->setStyleSheet("QWidget{background-color:rgba(255, 255, 255, 1);border-radius:6px;}");
        m_ocrBtnWid->setStyleSheet("QWidget{background-color:rgba(255, 255, 255, 1);border-radius:6px;}");
        //        m_ocrCancel->setIcon(QIcon(":/res/res/ocrCancel_W.png"));
    }
    m_ocrCancel->setIconSize(QSize(24, 24));
    m_rightPageBtn->themeStyle(themeTyep);
    m_leftPageBtn->themeStyle(themeTyep);
    m_imagePageWid->themeStyle(themeTyep);
}
//备用-目前不需要-以后可能会用到
int ShowImageWidget::cutQueryWid()
{
    return m_queryMsg->warning(this, tr("Warning"), tr("是否保存对此图片的更改？"), tr("覆盖保存"), tr("备份保存"),
                               tr("不保存"));
}

void ShowImageWidget::cutFinishUi()
{
    if (m_cutWid != nullptr) {
        m_cutWid->cutFinish();
    }
}
