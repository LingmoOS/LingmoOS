#include "coreinteraction.h"

Interaction *Interaction::m_interaction(nullptr);

Interaction *Interaction::getInstance()
{
    if (m_interaction == nullptr) {
        m_interaction = new CoreInteraction;
    }
    return m_interaction;
}


CoreInteraction::CoreInteraction()
{
    m_canProcess = new QTimer(this);
    m_canProcess->setSingleShot(true);
}

CoreInteraction::~CoreInteraction()
{
    if(m_core)
        m_core->deleteLater();
    if(m_thread)
        m_thread->quit();
}

void CoreInteraction::creatCore(const QStringList &list)
{
    if (m_isCoreInit) {
        return; //防止多次初始化核心
    }
    //开辟线程
    m_core = new Core();
    m_needStartWithOpenImagePath = m_core->initDbus(list);
    m_thread = new QThread();
    //绑定信号和槽
    initConnect();
    //放到新线程中
    m_core->moveToThread(m_thread);
    m_thread->start();
    m_isCoreInit = true; //防止多次初始化核心
}

void CoreInteraction::initConnect()
{
    connect(this, &CoreInteraction::coreOpenImage, m_core, &Core::findAllImageFromDir); //生成缩略图列表
    connect(this, &CoreInteraction::coreGetAlbumModel, m_core, &Core::getAlbumModel,
            Qt::BlockingQueuedConnection);                                  //获取相册model指针
    connect(m_core, &Core::openFinish, this, &CoreInteraction::openFinish); //图片打开完成，将数据返回给UI层
    connect(this, &CoreInteraction::coreChangeImage, m_core, &Core::changeImage);           //切换图片
    connect(this, &CoreInteraction::coreChangeWidgetSize, m_core, &Core::changeWidgetSize); //改变窗口大小
    connect(this, &CoreInteraction::coreChangeImageShowSize, m_core,
            &Core::changeImageShowSize); //图片显示状态（放大缩小）
    connect(m_core, &Core::showNavigation, this, &CoreInteraction::showNavigation);       //显示导航器
    connect(this, &CoreInteraction::coreClickNavigation, m_core, &Core::clickNavigation); //导航器点击
    connect(this, &CoreInteraction::coreFlip, m_core, &Core::flipImage);                  //翻转
    connect(this, &CoreInteraction::coreDeleteImage, m_core, &Core::deleteImage);         //删除图片
    connect(this, &CoreInteraction::coreSetAsBackground, m_core, &Core::setAsBackground); //设置为背景图
    connect(this, &CoreInteraction::coreOpenInfile, m_core, &Core::openInfile);           //文件夹中打开
    connect(this, &CoreInteraction::coreChangeOpenIcon, m_core, &Core::changeOpenIcon); //更改主题，切换相册打开按钮图标
    connect(this, &CoreInteraction::coreClose, m_core, &Core::close);                   //关闭事件
    connect(m_core, &Core::coreProgremExit, this, &CoreInteraction::progremExit); //发送信号让主界面结束进程
    connect(this, &CoreInteraction::changeImageFromClick, m_core, &Core::changeImageFromClick); //点击相册事件
    connect(m_core, &Core::changeAlbumHighLight, this,
            &CoreInteraction::changeAlbumHighLight);                      //在相册中选中到所切换的图片
    connect(m_core, &Core::delayShow, this, &CoreInteraction::delayShow); //图片加载过慢，发信号给前端
    connect(m_core, &Core::openFromAlbum, this, &CoreInteraction::openFromAlbum);    //点击相册按钮打开图片
    connect(this, &CoreInteraction::coreRename, m_core, &Core::toCoreChangeName);    //重命名
    connect(m_core, &Core::renameResult, this, &CoreInteraction::sendRenameResult);  //返回重命名结果
    connect(this, &CoreInteraction::corePrint, m_core, &Core::toPrintImage);         //打印
    connect(m_core, &Core::canDelImage, this, &CoreInteraction::changeDelState);     //检查是否可被删除
    connect(m_core, &Core::updateSideNum, this, &CoreInteraction::updateSideLength); //更新相册长度
    connect(m_core, &Core::fullScreen, this, &CoreInteraction::fullScreen);
    connect(this, &CoreInteraction::coreCut, m_core, &Core::cutImage); //开始裁剪，处理需要显示的图片
    connect(this, &CoreInteraction::coreCutRegion, m_core, &Core::cutRegion);     //裁剪区域
    connect(this, &CoreInteraction::coreStartChange, m_core, &Core::startChange); //起始点-裁剪
    connect(this, &CoreInteraction::coreRealsePos, m_core, &Core::realsePos);     //释放时-重绘裁剪框边界
    connect(m_core, &Core::releaseRegion, this, &CoreInteraction::releaseRegion); //释放时-裁剪框的起点和终点
    connect(this, &CoreInteraction::coreReleaseCutChange, m_core,
            &Core::releaseCutChange); //释放后-裁剪框需要再次改变尺寸的情况
    connect(this, &CoreInteraction::coreExitCut, m_core, &Core::exitCut);                   //退出裁剪
    connect(this, &CoreInteraction::coreNeedSaveCutImage, m_core, &Core::needSaveCutImage); //保存裁剪图片
    connect(this, &CoreInteraction::coreSaveCutImage, m_core, &Core::saveCutImage); //保存裁剪图片
    connect(m_core, &Core::cutSaveFinish, this, &CoreInteraction::cutSaveFinish);           //裁剪结束
    connect(this, &CoreInteraction::coreNeedSaveas, m_core, &Core::needSaveAsImage);        //图片另存为
    connect(m_core, &Core::returnMouseType, this, &CoreInteraction::returnMouseShape);      //回传鼠标形状
    connect(m_core, &Core::isCutNotSave, this, &CoreInteraction::isCutNotSave);   //正在裁剪还未保存就要关闭
    connect(this, &CoreInteraction::coreReloadImage, m_core, &Core::reloadImage); //将当前页的item传给后端
    connect(this, &CoreInteraction::coreGetText, m_core, &Core::getTextFromImage);        //获取图片中文字
    connect(m_core, &Core::getTextResult, this, &CoreInteraction::getTextFinish);         //图片中文字识别结果
    connect(m_core, &Core::sendHightlightPos, this, &CoreInteraction::sendHightlightPos); //回传导航栏高亮区域位置
    connect(this, &CoreInteraction::coreStartSignImageApp, m_core, &Core::startSignApp); //调起画图
    connect(m_core, &Core::exitOcrModeSign, this, &CoreInteraction::exitOcrModeSign);    //退出ocr模式

    // qml-------------------------------------
    connect(this, &CoreInteraction::coreQmlStartCutDisplay, m_core, &Core::qmlCutImageDisplay);
    connect(this, &CoreInteraction::coreQmlStartNaviDisplay, m_core, &Core::qmlNavigation);
    connect(this, &CoreInteraction::coreQmlClickNavigation, m_core, &Core::qmlClickNavigation);
    connect(m_core, &Core::getLeftUpPosAccordingImage, this, &CoreInteraction::getLeftUpPosAccordingImage);
    connect(this, &CoreInteraction::coreQmlTiffPageNavi, m_core, &Core::qmlTiffPage);
    connect(this, &CoreInteraction::coreQmlTiffImagePage, m_core, &Core::qmlTiffImagePage);
    //裁剪-显示区域全图
    connect(this, &CoreInteraction::coreCutFullViableImage, m_core, &Core::cutFullViableImage);
    //标注
    connect(this, &CoreInteraction::coreMarkImage, m_core, &Core::markImage); //按下标注
    connect(this, &CoreInteraction::coreStartMark, m_core, &Core::startMark); //进入标注
    connect(this, &CoreInteraction::coreMarkClear, m_core, &Core::markClear); //清除
    connect(this, &CoreInteraction::coreMarkSave, m_core, &Core::markSave);   //保存
    connect(this, &CoreInteraction::coreMarkUndo, m_core, &Core::markUndo);   //撤销

    connect(m_canProcess, &QTimer::timeout, this, &CoreInteraction::dealEndPointNaviReact);
    //背景替换
    connect(this, &CoreInteraction::corePicMatting, m_core, &Core::picMatting);
    connect(this, &CoreInteraction::coreReplaceBackground, m_core, &Core::replaceBackground);
    connect(m_core, &Core::returnReplaceAndOrigImage, this, &CoreInteraction::returnReplaceAndOrigImage);
    connect(this, &CoreInteraction::coreNeedSaveReplaceImage, m_core, &Core::needSaveReplaceImage);
    connect(this, &CoreInteraction::coreMattingFlip, m_core, &Core::mattingFlip);
    connect(m_core, &Core::returnFlipAndOrigImage, this, &CoreInteraction::returnFlipAndOrigImage);

    //扫描黑白件
    connect(this, &CoreInteraction::coreScanImage, m_core, &Core::scanImage);
    connect(m_core, &Core::returnScanAndOrigImage, this, &CoreInteraction::returnScanAndOrigImage);
    connect(m_core, &Core::returnScannerFlipAndOrigImage, this, &CoreInteraction::returnScannerFlipAndOrigImage);
    connect(m_core, &Core::returnScannerSaveResult, this, &CoreInteraction::returnScannerSaveResult);
    connect(this, &CoreInteraction::coreNeedSaveScanImage, m_core, &Core::needSaveScanImage);
    connect(this, &CoreInteraction::corePrintScanImage, m_core, &Core::printScanImage);
    connect(this, &CoreInteraction::coreScannerSavePDF, m_core, &Core::scannerSavePDF);
    connect(this, &CoreInteraction::coreScannerAddWatermark, m_core, &Core::scannerAddWatermark);
    connect(this, &CoreInteraction::coreScannerFlip, m_core, &Core::scannerFlip);
    connect(this, &CoreInteraction::coreScannerSaveCutImage, m_core, &Core::scannerSaveCutImage);
}

void CoreInteraction::picMatting(QImage pic)
{
    Q_EMIT corePicMatting(pic);
}

void CoreInteraction::replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter)
{
    Q_EMIT coreReplaceBackground(b, g, r, a, isFirstEnter);
}

void CoreInteraction::needSaveReplaceImage(QString path)
{
    Q_EMIT coreNeedSaveReplaceImage(path);
}

bool CoreInteraction::coreOperateTooOften()
{
    if (m_canProcess->isActive()) {
        return true;
    }
    m_canProcess->start(Variable::REFRESH_RATE); //刷新间隔
    return false;
}

void CoreInteraction::dealEndPointNaviReact()
{
    if (m_isNaviFromQml) {
        m_isNaviFromQml = false;
        if (m_imageScale != 0) {
            Q_EMIT coreQmlStartNaviDisplay(m_naviPointFromQml, m_displaySize, m_imageScale);
        }
    }
}

void CoreInteraction::initUiFinish()
{
    Q_EMIT startWithOpenImage(m_needStartWithOpenImagePath);
}

void CoreInteraction::openImage(const QString &path)
{
    Q_EMIT coreOpenImage(path);
    //打开图片-埋点
    kdk::kabase::BuriedPoint buriedPointOpenPicture;
    if (buriedPointOpenPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                   kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerOpenPicture)) {
        qDebug() << "buried point fail OpenPicture!";
    }
}

void CoreInteraction::changeImage(const int &type)
{
    if (coreOperateTooOften()) {
        return;
    }
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT coreChangeImage(type);
    //切换图片-埋点
    kdk::kabase::BuriedPoint buriedPointSwitchPicture;
    if (buriedPointSwitchPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                     kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSwitchPicture)) {
        qCritical() << "buried point fail SwitchPicture!";
    }
}

void CoreInteraction::nextImage()
{
    if (!Variable::g_allowChangeImage) {
        return;
    }
    if (coreOperateTooOften()) {
        return;
    }
    //    operayteMode = OperayteMode::NextImage;
    //    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT coreChangeImage(Enums::NEXT_IMAGE);
    //切换图片-埋点
    kdk::kabase::BuriedPoint buriedPointSwitchPicture;
    if (buriedPointSwitchPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                     kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSwitchPicture)) {
        qCritical() << "buried point fail SwitchPicture!";
    }
}

void CoreInteraction::backImage()
{
    if (!Variable::g_allowChangeImage) {
        return;
    }
    if (coreOperateTooOften()) {
        return;
    }
    //    operayteMode = OperayteMode::BackImage;
    //    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT coreChangeImage(Enums::BACK_IMAGE);
    //切换图片-埋点
    kdk::kabase::BuriedPoint buriedPointSwitchPicture;
    if (buriedPointSwitchPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                     kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSwitchPicture)) {
        qCritical() << "Error : buried point fail SwitchPicture!";
    }
}

void CoreInteraction::changeWidgetSize(const QSize &size)
{
    Q_EMIT coreChangeWidgetSize(size);
}

void CoreInteraction::watchBigImage(QPoint focus)

{
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::BIG, focus);
    //缩放图片-埋点
    kdk::kabase::BuriedPoint buriedPointZoomInandOut;
    if (buriedPointZoomInandOut.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerZoomInandOut)) {
        qCritical() << "buried point fail ZoomInandOut!";
    }
}

void CoreInteraction::watchSmallImage(QPoint focus)
{

    Q_EMIT coreChangeImageShowSize(ImageShowStatus::SMALL, focus);
    //缩放图片-埋点
    kdk::kabase::BuriedPoint buriedPointZoomInandOut;
    if (buriedPointZoomInandOut.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerZoomInandOut)) {
        qCritical() << "buried point fail ZoomInandOut!";
    }
}

void CoreInteraction::watchOriginalImage()
{
    //    if (coreOperateTooOften()) {
    //        return;
    //    }
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::ORIGIN, QPoint());
}

void CoreInteraction::watchAutoImage()
{
    //    if (coreOperateTooOften()) {
    //        return;
    //    }
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::AUTO, QPoint());
}

void CoreInteraction::clickNavigation(const QPoint &point, bool isDragImage)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreClickNavigation(point, isDragImage);
}

void CoreInteraction::rotate(const bool &clockwise)
{
    if (coreOperateTooOften()) {
        return;
    }
    if (clockwise) {
        Q_EMIT coreFlip(Processing::clockwise);
    } else {
        Q_EMIT coreFlip(Processing::counterclockwise);
    }
    //旋转-埋点
    kdk::kabase::BuriedPoint buriedPointRotate;
    if (buriedPointRotate.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                              kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerRotate)) {
        qCritical() << "buried point fail Rotate!";
    }
}

void CoreInteraction::flipH()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreFlip(Processing::horizontal);
    //翻转图片-埋点
    kdk::kabase::BuriedPoint buriedPointFlip;
    if (buriedPointFlip.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                            kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerFlip)) {
        qCritical() << "buried point fail Flip!";
    }
}

void CoreInteraction::flipV()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreFlip(Processing::vertical);
    //翻转图片-埋点
    kdk::kabase::BuriedPoint buriedPointFlip;
    if (buriedPointFlip.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                            kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerFlip)) {
        qCritical() << "buried point fail Flip!";
    }
}

void CoreInteraction::mattingRotate(const bool &clockwise)
{
    if (clockwise) {
        Q_EMIT coreMattingFlip(Processing::clockwise);
    } else {
        Q_EMIT coreMattingFlip(Processing::counterclockwise);
    }
}
void CoreInteraction::mattingFlipH()
{
    Q_EMIT coreMattingFlip(Processing::horizontal);
}
void CoreInteraction::mattingFlipV()
{
    Q_EMIT coreMattingFlip(Processing::vertical);
}

void CoreInteraction::scanImage()
{
    Q_EMIT coreScanImage();
}

void CoreInteraction::needSaveScanImage(QString savePath)
{
    Q_EMIT coreNeedSaveScanImage(savePath);
}

void CoreInteraction::printScanImage(QPrinter *printer)
{
    Q_EMIT corePrintScanImage(printer);
}

void CoreInteraction::scannerSavePDF(QPrinter *printer)
{
    Q_EMIT coreScannerSavePDF(printer);
}

void CoreInteraction::scannerAddWatermark(bool isWatermark, QString vmContent)
{
    Q_EMIT coreScannerAddWatermark(isWatermark, vmContent);
}

void CoreInteraction::scannerRotate(const bool &clockwise)
{
    if (clockwise) {
        Q_EMIT coreScannerFlip(Processing::clockwise);
    } else {
        Q_EMIT coreScannerFlip(Processing::counterclockwise);
    }
}

void CoreInteraction::scannerFlipH()
{
    Q_EMIT coreScannerFlip(Processing::horizontal);
}

void CoreInteraction::scannerFlipV()
{
    Q_EMIT coreScannerFlip(Processing::vertical);
}

void CoreInteraction::scannerSaveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint)
{
    if (coreOperateTooOften()) {
        return;
    }
	
    Q_EMIT coreScannerSaveCutImage(proportion, beginPoint, endPoint);
}

void CoreInteraction::deleteImage()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreDeleteImage();
    //删除-埋点
    kdk::kabase::BuriedPoint buriedPointDelete;
    if (buriedPointDelete.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                              kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerDelete)) {
        qCritical() << "buried point fail Delete!";
    }
}

void CoreInteraction::setAsBackground()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreSetAsBackground();
    //设置为背景-埋点
    kdk::kabase::BuriedPoint buriedPointSetAsDesktopWallpaper;
    if (buriedPointSetAsDesktopWallpaper.functionBuriedPoint(
            kdk::kabase::AppName::LingmoPhotoViewer,
            kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSetAsDesktopWallpaper)) {
        qCritical() << "buried point fail SetAsDesktopWallpaper!";
    }
}

void CoreInteraction::openImageInfile()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreOpenInfile();
    //在文件夹中打开-埋点
    kdk::kabase::BuriedPoint buriedPointShowInFolder;
    if (buriedPointShowInFolder.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                    kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerShowInFolder)) {
        qCritical() << "buried point fail ShowInFolder!";
    }
}

void CoreInteraction::changeOpenIcon(QString theme)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreChangeOpenIcon(theme);
}

void CoreInteraction::close()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreClose();
}

QStandardItemModel *CoreInteraction::getAlbumModel()
{
    return coreGetAlbumModel();
}

void CoreInteraction::reName(QString oldPath, QString newPath)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreRename(oldPath, newPath);
    //打印-埋点
    kdk::kabase::BuriedPoint buriedPointRename;
    if (buriedPointRename.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                              kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerRename)) {
        qCritical() << "buried point fail Rename!";
    }
}

void CoreInteraction::printImage(QPrinter *printer)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT corePrint(printer);
    //打印-埋点
    kdk::kabase::BuriedPoint buriedPointPrint;
    if (buriedPointPrint.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                             kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerPrint)) {
        qCritical() << "buried point fail Print!";
    }
}

void CoreInteraction::cutImage()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreCut();
    //裁剪-埋点
    kdk::kabase::BuriedPoint buriedPointCutting;
    if (buriedPointCutting.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                               kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerCutting)) {
        qCritical() << "buried point fail Cutting!";
    }
}
void CoreInteraction::getText()
{
    Q_EMIT coreGetText();
}


void CoreInteraction::cutRegion(const QPoint &point)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreCutRegion(point);
}

void CoreInteraction::startChange(const QPoint &point)
{

    Q_EMIT coreStartChange(point);
}

void CoreInteraction::releasePos(const QPoint &point)
{
    Q_EMIT coreRealsePos(point);
}

void CoreInteraction::releaseCutChange(int type, const QPoint &point)
{
    Q_EMIT coreReleaseCutChange(type, point);
}

void CoreInteraction::exitCut()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreExitCut();
}

void CoreInteraction::needSaveCutImage(int proportion)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreNeedSaveCutImage(proportion);
}

void CoreInteraction::saveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreSaveCutImage(proportion, beginPoint, endPoint);
}

void CoreInteraction::needSaveAs(QString path)
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreNeedSaveas(path);
    //另存为-埋点
    kdk::kabase::BuriedPoint buriedPointSaveAs;
    if (buriedPointSaveAs.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                              kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSaveAs)) {
        qCritical() << "buried point fail SaveAs!";
    }
}

void CoreInteraction::reloadImage(QModelIndex modelIndex)
{
    Q_EMIT coreReloadImage(modelIndex);
}

void CoreInteraction::startSignImageApp()
{
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreStartSignImageApp();
}

void CoreInteraction::qmlStartCutDisplay(QPointF startPos, QSizeF displaySize, double imageScale)
{
    Q_EMIT coreQmlStartCutDisplay(startPos, displaySize, imageScale);
}

void CoreInteraction::qmlStartNaviDisplay(QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale)
{
    m_isNaviFromQml = true;
    m_naviPointFromQml = leftUpPosAccordingShowImage;
    m_displaySize = displaySize;
    m_imageScale = imageScale;
    if (coreOperateTooOften()) {
        return;
    }
    Q_EMIT coreQmlStartNaviDisplay(leftUpPosAccordingShowImage, displaySize, imageScale);
}

void CoreInteraction::qmlClickNaviDisplay(QPointF curMousePos, bool needChangeDisplayImage)
{

    Q_EMIT coreQmlClickNavigation(curMousePos, needChangeDisplayImage);
}

void CoreInteraction::qmlTiffPageNavi(int pageNum, QPointF leftUpPosAccordingShowImage, QSizeF displaySize,
                                      double imageScale)
{
    Q_EMIT coreQmlTiffPageNavi(pageNum, leftUpPosAccordingShowImage, displaySize, imageScale);
}

void CoreInteraction::qmlTiffImagePage(int pageNum)
{
    Q_EMIT coreQmlTiffImagePage(pageNum);
}
void CoreInteraction::clickCutFullViableImage()
{
    Q_EMIT coreCutFullViableImage();
}

void CoreInteraction::startMark()
{
    Q_EMIT coreStartMark();
}

void CoreInteraction::toMarkImage(QVariant var)
{
    Q_EMIT coreMarkImage(var);
}

void CoreInteraction::markUndo()
{
    Q_EMIT coreMarkUndo();
}

void CoreInteraction::markSave(QString savePath)
{
    Q_EMIT coreMarkSave(savePath);
}

void CoreInteraction::markClear()
{
    Q_EMIT coreMarkClear();
}
