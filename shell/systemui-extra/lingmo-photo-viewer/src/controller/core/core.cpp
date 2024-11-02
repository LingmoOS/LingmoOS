#include "core.h"

Core::Core()
{
    initCore();
}

Core::~Core()
{
    delete m_fileWatcher;
    m_fileWatcher = nullptr;

    m_loadedImageCache.clear();
}

void Core::initCore()
{
    connect(LingmoImageCodec::getSignalObj(), &LingmoImageCodecSignals::saveMovieFinish, this, &Core::saveMovieFinish);

    m_matList = nullptr;

    m_albumModel = new QStandardItemModel;

    qRegisterMetaType<ImageShowStatus::ChangeShowSizeType>("ImageShowStatus::ChangeShowSizeType");
    qRegisterMetaType<QFileInfo>("QFileInfo");
    qRegisterMetaType<Processing::FlipWay>("Processing::FlipWay");

    m_clickBeforeStartPosition = QPoint(-1, -1);
    m_clickBeforePosition = QPoint(-1, -1);

    m_ocrThread = new QThread(this);
    m_ocr.moveToThread(m_ocrThread);
    m_ocrThread->start();
    connect(&m_ocr, &OCR::recResult, this, &Core::sendOcrResult);
    connect(this, &Core::startOcr, &m_ocr, &OCR::getText);

    m_playMovieTimer = new QTimer(this);
    connect(m_playMovieTimer, &QTimer::timeout, this, &Core::playMovie);

    m_canProcess = new QTimer(this);
    m_canProcess->setSingleShot(true);

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardError, this, &Core::processLog);

    m_fileWatcher = new QFileSystemWatcher();
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &Core::fileChangedNeedUpdated);

    connect(&m_markTool, &Mark::MarkTool::saveFinish, this, &Core::openSaveImage);
}

QString Core::initDbus(const QStringList &arguments)
{
    m_dbus = new Dbus;

    //获取注册Dbus服务是否成功
    bool connectSeccess = m_dbus->getConnectSeccess();

    //如果注册Dbus服务成功，接收命令
    if (connectSeccess) {
        connect(m_dbus, &Dbus::processingCommand, this, &Core::processingCommand);
        connect(m_dbus, &Dbus::processingFullScreen, this, &Core::processingFullscreen);
    }

    //没有参数不处理
    if (arguments.length() < 2) {
        return "";
    }

    //帮助命令则打印
    if (arguments[1] == "-help" || arguments[1] == "--help" || arguments[1] == "-h") {
        for (QString &key : Variable::SUPPORT_CMD.keys()) {
            QString str = key + "   " + Variable::SUPPORT_CMD.value(key);
            QStringList list = str.split("\n");
            for (QString &tmpStr : list) {
                qInfo() << tmpStr;
            }
        }
        exit(0);
        return "";
    }

    //如果是地址且文件存在则打开
    QString path = arguments[1];
    if (path.startsWith("file:///")) {
        path.replace("file:///", "/");
    }
    if (QFileInfo::exists(path)) {
        //不是支持的文件格式忽略
        QString format = path;
        format = format.split(".").last();
        if (!Variable::SUPPORT_FORMATS.contains(format.toLower())) {
            Variable::g_hasIllegalSuffix = true;
        } else {
            Variable::g_hasIllegalSuffix = false;
        }

        if (arguments.length() == 3) {
            if (Variable::startAppNameList.contains(arguments[2])) {
                Variable::startAppName = arguments[2];
            }
        }
        return path;
    }

    if (arguments[1] == "-api") {
        return processingApi(arguments);
    }

    //如果是命令
    if (Variable::SUPPORT_CMD.keys().contains(arguments[1])) {
        if (connectSeccess) { //如果为首个实例不响应
            return "";
        }
        m_dbus->argumentsCommand(arguments);
        return "";
    }

    qDebug() << "参数异常";
    return "";
}

void Core::processingCommand(const QStringList &cmd)
{
    if (coreOperateTooOften()) {
        return;
    }
    qDebug() << "响应外部命令" << cmd;

    if (cmd[0] == "-next") {
        changeImage(NEXT_IMAGE);
        return;
    }
    if (cmd[0] == "-back") {
        changeImage(BACK_IMAGE);
        return;
    }
    if (cmd[0] == "-big") {
        changeImageShowSize(ImageShowStatus::BIG);
        return;
    }
    if (cmd[0] == "-small") {
        changeImageShowSize(ImageShowStatus::SMALL);
        return;
    }
    if (cmd[0] == "-origin") {
        changeImageShowSize(ImageShowStatus::ORIGIN);
        return;
    }
    if (cmd[0] == "-auto") {
        changeImageShowSize(ImageShowStatus::AUTO);
        return;
    }
    if (cmd[0] == "-rotate") {
        flipImage(Processing::FlipWay::clockwise);
        return;
    }
}

QString Core::processingApi(const QStringList &cmd)
{
    if (cmd.length() < 4) {
        return "";
    }

    if (cmd[2] == "-flip") {
        if (cmd.length() < 5) {
            qInfo() << "参数不合规，使用-h查看帮助";
            return "";
        }
        m_apiCmd.append(cmd[2]);
        if (!QFileInfo::exists(cmd[3])) {
            qInfo() << "文件不存在";
            return "";
        }
        QString format = cmd[3];
        format = format.split(".").last();
        if (!Variable::SUPPORT_FORMATS.contains(format)) {
            qInfo() << "不支持的格式";
            return "";
        }
        m_apiCmd.append(cmd[3]);
        QString flipCmd = cmd[4];
        int hNum = flipCmd.count('h');
        hNum = hNum % 2; //保留有效操作
        int vNum = flipCmd.count('v');
        vNum = vNum % 2;
        int cNum = flipCmd.count('c');
        cNum = cNum % 4;
        QString flipCmdResult;
        for (int i = 0; i < hNum; i++) {
            flipCmdResult += "h";
        }
        for (int i = 0; i < vNum; i++) {
            flipCmdResult += "v";
        }
        for (int i = 0; i < cNum; i++) {
            flipCmdResult += "c";
        }
        m_apiCmd.append(flipCmdResult);

        if (cmd.length() > 5) {
            QString saveWay = cmd[5];
            if (saveWay.indexOf('r') > 0) {
                m_apiReplaceFile = true;
            }
        }

        m_isApi = true;
        return Variable::API_TYPE;
    }
    return "";
}

void Core::processingFullscreen()
{
    Q_EMIT fullScreen();
}

void Core::setHighLight(const QString &path)
{
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == path) {
            Q_EMIT changeAlbumHighLight(item->index());
            return;
        }
    }
}

void Core::openImage(QString fullPath)
{
    // 1.文件路径为空；2.从外部删除文件夹下的所有图片时，传空图片列表，不展示图片，回到开始界面
    if (fullPath.isEmpty()) {
        showImage(QPixmap());
        return;
    }
    if (m_isclose == true) {
        return;
    }
    Variable::g_hasRotation = false;
    m_rotateRecord.clear();
    m_tiffPage = 1;
    //判断图片状态
    QFlags<QFileDevice::Permission> power = QFile::permissions(fullPath);
    //判断文件所有者是否可写和是否可删除
    if (power.testFlag(QFile::WriteOwner) && File::canDel(fullPath)) {
        m_isCanDel = true;
        //发送信号给前端--可删除
        Q_EMIT canDelImage(true);
    } else {
        m_isCanDel = false;
        //发送信号给前端--不可删除
        Q_EMIT canDelImage(false);
    }

    m_willProcess.clear();
    if (!Variable::g_needEnterSign) {
        needSave();
    }
    //    m_nowRealFormat = getRealFormat(File::realFormat(fullPath));
    m_nowRealFormat = File::realFormat(fullPath);
    QFileInfo fileInfo(fullPath);
    if (fileInfo.suffix().toLower() == m_nowRealFormat) {
        Variable::g_hasIllegalSuffix = false;
    } else {
        Variable::g_hasIllegalSuffix = true;
    }

    QHash<QString, QPair<MatAndFileinfo, QPixmap>>::iterator iter = m_loadedImageCache.find(fullPath);
    MatAndFileinfo maf;
    if (iter != m_loadedImageCache.end())
    {
        QPair<MatAndFileinfo, QPixmap> matPix = iter.value();
        maf = matPix.first;
        m_nowImage = matPix.second;
    }
    else
    {
        maf = File::loadImage(fullPath, m_nowRealFormat);
        m_nowImage = Processing::converFormat(maf.mat);

        QPair<MatAndFileinfo, QPixmap> matPix;
        matPix.first = maf;
        matPix.second = m_nowImage;
        m_loadedImageCache.insert(fullPath, matPix);
    }

    m_openSuccess = maf.openSuccess; //判断是否打开失败-损坏
    if (!m_openSuccess) {
        //更新侧栏图标
        changeOpenIcon(Variable::g_themeStyle);
        if (m_loadedImage.contains(fullPath)) {
            m_loadedImage.removeAt(m_loadedImage.indexOf(fullPath));
        }
    }
    setHighLight(fullPath); //设置相册选中
    m_origPix = m_nowImage;

    //记录状态
    changeMat(maf.mat);
    m_info = maf.info;
    m_maxFrame = maf.maxFrame;
    m_matList = maf.matList;
    m_fps = maf.fps;
    m_nowpath = fullPath;
    //首次打开，m_backpath为空，造成相册切换
    if (m_backpath == nullptr) {
        m_backpath = m_nowpath;
    }
    //如果正在保存，则显示等待
    if (File::isSaving(fullPath) || (!m_tiffHasOperatedFinish && m_tiffProcessPath == m_nowpath)) {
        //如果此图正在保存
        m_thisImageIsSaving = true;
        showImage(QPixmap());
        return;
    }
    m_imageNeedUpdate = true;
    creatImage(-1, true);
    return;
}

void Core::showImage(const QPixmap &pix)
{
    //对于动图在静止无操作时，不多次进行触发
    if (!pix.isNull()) {
        if (m_matList != nullptr && (m_info.suffix().toLower() == "gif" || m_info.suffix().toLower() == "apng")) {
            if (m_backImagePath == m_nowpath) {
                if (operayteMode == OperayteMode::NormalMode || operayteMode == OperayteMode::NoOperate
                    || operayteMode == OperayteMode::ChangeImage) {
                    return;
                }
            }
        }
    }
    if (!pix.isNull()) {
        m_backImagePath = m_nowpath;
    }
    m_origCutImage = pix.toImage().copy();
    m_origMattingImage = pix.toImage().copy();

    ImageAndInfo package;
    package.proportion = m_proportion;
    package.imageNumber = m_albumModel->rowCount();
    package.info = m_info;
    package.image = pix;
    package.realFormat = m_nowRealFormat;
    m_paperFormat = m_nowRealFormat;
    package.openSuccess = m_openSuccess;
    if (!m_openSuccess) {
        package.imageSize = "-";
        package.colorSpace = "-";
    } else {
        package.imageSize = m_imageSize;
        package.colorSpace = m_colorSpace;
    }
    if (m_matList != nullptr) {
        package.movieImage = true;
    } else {
        package.movieImage = false;
    }
    package.originImage = m_nowImage;
    //    Variable::g_tempImage = m_nowImage.toImage();
    package.displaySizeOfPicture = m_tmpSize;
    package.imageShowWay = m_imageShowWay;
    if (displayMode == DisplayMode::OCRMode) {
        m_imageNeedUpdate = false;
    }
    package.imageNeedUpdate = m_imageNeedUpdate;
    package.zoomFocus = m_zoomFocus;
    package.leftUpPos = m_leftUpPos;
    package.actualClickPos = m_actualClickPos;
    package.actualPos = m_actualClickPos;
    package.allImagePathList = m_allImageList;
    package.imageDelPath = m_imageDelPath;
    package.imageTypeList = m_imageListEveryAddType;
    m_imageDelPath = "";

    if (m_allImageList.contains(m_nowpath)) {
        package.currentImageIndex = m_allImageList.indexOf(m_nowpath);
        package.loadImageList = this->needLoadImagePathList();
        package.showImageIndex = this->needUpdateImageIndex(package.loadImageList);
        package.imageTypeMovieOrNormal = this->needUpdateImageType(package.loadImageList);
    } else {
        package.currentImageIndex = 0;
    }

    if (m_info.suffix().toLower() == "tiff" || m_info.suffix().toLower() == "tif") {
        if (m_matList != nullptr && m_matList->length() > 1) {
            package.imageList = m_tiffImageList;
        }
    }
    QVariant var;
    var.setValue<ImageAndInfo>(package);
    Q_EMIT openFinish(var);
}

void Core::creatImage(const int &proportion, bool defaultOpen)
{
    m_leftUpPos = QPointF(0, 0);

    if (m_nowImage.isNull()) {
        return;
    }
    QPixmap tempPix = m_nowImage;
    if (m_matList != nullptr && (m_paperFormat == "tiff" || m_paperFormat == "tif")) {
        if (m_tiffImageList.length() > 0) {
            tempPix = QPixmap::fromImage(m_tiffImageList.at(m_tiffPage - 1));
        }
    }
    int defaultProportion = 100 * m_size.width() / tempPix.width();
    if (tempPix.height() * defaultProportion / 100 > m_size.height()) {
        defaultProportion = 100 * m_size.height() / tempPix.height();
    }

    if (defaultOpen == false) {
        m_isClickBtnChangeImageSize = true;
        operateImage(proportion, defaultProportion);
    } else {
        m_isClickBtnChangeImageSize = false;
        defaultImage(proportion, defaultProportion);
    }
}
//默认打开，旋转等，不进行放缩操作的方式查看图片
void Core::defaultImage(int proportion, int defaultProportion)
{
    //自适应窗口大小显示
    if (proportion <= 0) {
        if (defaultProportion >= 100) {
            m_proportion = 100;
        } else {
            m_proportion = defaultProportion;
        }
        m_lastTimeProportion = defaultProportion;
        m_tmpSize = m_nowImage.size() * m_proportion / 100;
        //        navigation(QPoint(-1, -1), true); //关闭导航器
        showImageOrMovie();
        return;
    }
}
//对图片尺寸进行了操作的方式查看图片
void Core::operateImage(int proportion, int defaultProportion)
{

    m_lastTimeProportion = m_proportion;
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
    m_tmpSize = m_nowImage.size() * m_proportion / 100;
    //极小的图缩放后尺寸低于1的不处理
    if (judgeSizeIsZero(m_tmpSize)) {
        return;
    }
    showImageOrMovie();
}

void Core::showImageOrMovie()
{
    if (m_tiffImageList.length() > 0) {
        m_tiffImageList.clear();
    }
    QFileInfo fileInfo(m_nowpath);
    //动画类格式循环播放
    if (m_matList != nullptr) {
        if (fileInfo.suffix().toLower() != "tiff" && fileInfo.suffix().toLower() != "tif") {
            if (m_matList->length() > 1) {
                playMovie(); //立即播放第一帧
                m_playMovieTimer->start(m_fps);
                return;
            }
        } else {
            if (m_matList->length() > 1) {
                for (int i = 0; i < m_matList->length(); i++) {
                    Mat mat = m_matList->at(i);
                    QPixmap nowIndexPix = Processing::converFormat(mat);
                    m_tiffImageList.append(nowIndexPix.toImage());
                }
            }
        }
    }
    //非动画类格式
    QPixmap pix = Processing::resizePix(m_nowImage, m_tmpSize);
    showImage(pix);
}

void Core::playMovie()
{

    if (m_matListIndex >= m_matList->length()) {
        m_matListIndex = 0;
    }

    Mat mat = m_matList->at(m_matListIndex);
    QPixmap nowIndexPix = Processing::converFormat(mat);
    m_matListIndex++;

    if (m_isNavigationShow) {

        QPixmap result = Processing::localAmplification(nowIndexPix, m_tmpSize, m_startShowPoint, m_size);
        showImage(result);
        return;
    }
    QPixmap pix = Processing::resizePix(nowIndexPix, m_tmpSize);
    showImage(pix);
}

void Core::navigation(const QPoint &point, bool isDragImage)
{
    //    m_clickBeforeStartPosition = QPoint(-1, -1);
    if (point.x() < 0 || point.y() < 0) { //关闭导航器
        m_clickBeforePosition = QPoint(-1, -1);
        m_isNavigationShow = false;
        Q_EMIT showNavigation(QPixmap());
        return;
    }
    m_isNavigationShow = true;
    creatNavigation();
    //记录上次放大位置
    clickNavigation(m_zoomFocus, isDragImage);
}

void Core::qmlNavigation(QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale)
{
    m_qmlProportion = imageScale;
    qmlCreatNavigation(displaySize);
    qmlClickNavigation(leftUpPosAccordingShowImage, false);
}

void Core::qmlCreatNavigation(QSizeF displaySize)
{
    if (m_matList != nullptr && (m_paperFormat == "tiff" || m_paperFormat == "tif")) {
        m_tiffNaviPix = QPixmap::fromImage(m_tiffImageList.at(m_tiffPage - 1));
    } else {
        m_tiffNaviPix = m_nowImage;
    }

    //导航栏背景
    QSize navigationSize = Variable::NAVIGATION_SIZE;
    if (m_naviBackPixPath != m_nowpath) {
        m_navigationImage = Processing::resizePix(m_tiffNaviPix, navigationSize).toImage();
        m_naviBackPixPath = m_nowpath;
    } else if (m_naviBackPixPath == m_nowpath
               && (m_matList != nullptr && (m_paperFormat == "tiff" || m_paperFormat == "tif"))) {
        m_navigationImage = Processing::resizePix(m_tiffNaviPix, navigationSize).toImage();
    } else {
        m_navigationImage = Processing::resizePix(m_tiffNaviPix, navigationSize).toImage();
    }
    //记录空白区域
    m_spaceWidth = (navigationSize.width() - m_navigationImage.width()) / 2;
    m_spaceHeight = (navigationSize.height() - m_navigationImage.height()) / 2;

    //待显示图
    m_tmpSize = m_tiffNaviPix.size() * m_qmlProportion;

    //    m_hightlightSize.setWidth(
    //        displaySize.width() / m_qmlProportion
    //        * ((static_cast<double>(m_navigationImage.width()) / static_cast<double>(m_nowImage.width()))) *
    //        m_size.width() / m_tmpSize.width());
    //    m_hightlightSize.setHeight(
    //        displaySize.height() / m_qmlProportion
    //        * ((static_cast<double>(m_navigationImage.height()) / static_cast<double>(m_nowImage.height())))
    //        * m_size.height() / m_tmpSize.height());

    m_hightlightSize.setWidth(m_navigationImage.width() * displaySize.width() / m_tmpSize.width());
    m_hightlightSize.setHeight(m_navigationImage.height() * displaySize.height() / m_tmpSize.height());

    if (m_hightlightSize.width() > m_navigationImage.width()) {
        m_hightlightSize.setWidth(m_navigationImage.width());
    }
    if (m_hightlightSize.height() > m_navigationImage.height()) {
        m_hightlightSize.setHeight(m_navigationImage.height());
    }
}

void Core::tiffOperate()
{
    m_processed = false;
    m_tiffHasOperatedFinish = false;
    MultiTiffOperate *thread = new MultiTiffOperate(m_nowpath, m_tiffProcess);
    connect(thread, &MultiTiffOperate::finished, thread, &MultiTiffOperate::deleteLater);
    connect(thread, &MultiTiffOperate::tiffOPerateFinish, this, &Core::tiffOperateFinish);
    thread->start();
}

QStringList Core::needLoadImagePathList()
{
    QStringList tempImageList = {};

    //判断是否包含当前path
    if (m_allImageList.contains(m_nowpath)) {
        //判断是否路径列表内有5个（包含5个）的路径
        if (m_allImageList.length() <= 5) {
            tempImageList = m_allImageList;
            return tempImageList;
        }
        int currentIndex = m_allImageList.indexOf(m_nowpath);

        if (currentIndex + 2 <= m_allImageList.length() - 1 && currentIndex - 2 >= 0) {
            tempImageList.insert(0, m_allImageList.at(currentIndex - 2));
            tempImageList.insert(1, m_allImageList.at(currentIndex - 1));
            tempImageList.insert(2, m_allImageList.at(currentIndex));
            tempImageList.insert(3, m_allImageList.at(currentIndex + 1));
            tempImageList.insert(4, m_allImageList.at(currentIndex + 2));
        } else if (currentIndex + 2 > m_allImageList.length() - 1) {
            tempImageList.insert(0, m_allImageList.at(currentIndex - 2));
            tempImageList.insert(1, m_allImageList.at(currentIndex - 1));
            tempImageList.insert(2, m_allImageList.at(currentIndex));
            if (currentIndex + 1 > m_allImageList.length() - 1) {
                tempImageList.insert(3, m_allImageList.at(0));
                tempImageList.insert(4, m_allImageList.at(1));
            } else {
                tempImageList.insert(3, m_allImageList.at(currentIndex + 1));
                tempImageList.insert(4, m_allImageList.at(0));
            }
        } else if (currentIndex - 2 < 0) {
            if (currentIndex - 1 < 0) {
                tempImageList.insert(0, m_allImageList.at(m_allImageList.length() - 2));
                tempImageList.insert(1, m_allImageList.at(m_allImageList.length() - 1));
            } else {
                tempImageList.insert(0, m_allImageList.at(m_allImageList.length() - 1));
                tempImageList.insert(1, m_allImageList.at(0));
            }
            tempImageList.insert(2, m_allImageList.at(currentIndex));
            tempImageList.insert(3, m_allImageList.at(currentIndex + 1));
            tempImageList.insert(4, m_allImageList.at(currentIndex + 2));
        }
        return tempImageList;
    }
}

QList<int> Core::needUpdateImageIndex(QStringList imageList)
{
    QList<int> tempIndex = {};
    m_showImageIndex.clear();
    for (int i = 0; i < imageList.length(); i++) {
        if (m_allImageList.contains(imageList.at(i))) {
            tempIndex.append(m_allImageList.indexOf(imageList.at(i)));
        }
    }
    if (tempIndex.length() != imageList.length()) {
        qDebug() << "图片index预加载失败!";
        m_showImageIndex.clear();
        tempIndex.clear();
        return tempIndex;
    }
    m_showImageIndex = tempIndex;
    return tempIndex;
}

QList<int> Core::needUpdateImageType(QStringList imageList)
{
    QList<int> tempTypeList = {};
    for (int i = 0; i < imageList.length(); i++) {
        if (m_allImageList.contains(imageList.at(i))) {
            if (!Variable::MOVIETYPE.contains(File::realFormat(imageList.at(i)).toLower())) {
                tempTypeList.append(0);
            } else {
                tempTypeList.append(1);
            }
        }
    }
    return tempTypeList;
}

void Core::needUpdateAddImageType(QStringList addImageList)
{
    if (addImageList.length() <= 0) {
        return;
    } else {
        for (int i = 0; i < addImageList.length(); i++) {
            QString tempType = File::realFormat(addImageList.at(i));
            if (tempType == "gif" || tempType == "apng") {
                m_imageListEveryAddType.insert(i, 1);
            } else {
                m_imageListEveryAddType.insert(i, 0);
            }
        }
    }
}

void Core::qmlClickNavigation(QPointF curMousePos, bool needChangeDisplayImage)
{
    bool hasArg = true;
    //无参输入则使用上次的位置
    if (curMousePos == QPoint(-1, -1)) {
        hasArg = false;
    }
    //有参则记录，无参使用上一次的点
    if (hasArg) {
        m_clickBeforePosition = curMousePos;
    }
    //两种情况：1.点击导航栏区域或拖拽导航栏高亮区域；2.除1外的其他情况
    int naviClickStartPointX = m_clickBeforePosition.x() - m_hightlightSize.width() / 2 - m_spaceWidth;
    int naviClickStartPointY = m_clickBeforePosition.y() - m_hightlightSize.height() / 2 - m_spaceHeight;
    double otherStartPointX =
        static_cast<double>(m_clickBeforePosition.x()) / m_qmlProportion
        * ((static_cast<double>(m_navigationImage.width()) / static_cast<double>(m_tiffNaviPix.width())));
    double otherStartPointY =
        static_cast<double>(m_clickBeforePosition.y()) / m_qmlProportion
        * ((static_cast<double>(m_navigationImage.height()) / static_cast<double>(m_tiffNaviPix.height())));
    QPoint startPoint;
    //计算点击区域
    if (needChangeDisplayImage) {
        startPoint.setX(naviClickStartPointX);
        startPoint.setY(naviClickStartPointY);
    } else {
        startPoint.setX(otherStartPointX);
        startPoint.setY(otherStartPointY);
    }

    int right = m_navigationImage.width() - m_hightlightSize.width();    //右侧边缘
    int bottom = m_navigationImage.height() - m_hightlightSize.height(); //下侧边缘

    //过滤无效区域
    if (startPoint.x() < 0) {
        startPoint.setX(0);
    }
    if (startPoint.y() < 0) {
        startPoint.setY(0);
    }
    if (startPoint.x() > right) {
        startPoint.setX(right);
    }
    if (startPoint.y() > bottom) {
        startPoint.setY(bottom);
    }
    m_clickBeforeStartPosition = startPoint;
    //处理导航器图片
    QImage image = Processing::pictureDeepen(m_navigationImage, m_hightlightSize, startPoint);

    //计算实际显示的图片的部分在当前缩放比下相对于图片上的位置
    //已知：导航栏图片，高亮区域的大小，起始点位置，缩放比
    if (needChangeDisplayImage) {

        double displayDiffX =
            static_cast<double>(startPoint.x() - m_backHightLight.x())
            / (static_cast<double>(m_navigationImage.width()) / static_cast<double>(m_tiffNaviPix.width()))
            * m_qmlProportion;
        double displayDiffY =
            static_cast<double>(startPoint.y() - m_backHightLight.y())
            / (static_cast<double>(m_navigationImage.height()) / static_cast<double>(m_tiffNaviPix.height()))
            * m_qmlProportion;

        QPointF actualPos(displayDiffX, displayDiffY);
        Q_EMIT getLeftUpPosAccordingImage(actualPos);
    }
    //发送到导航器设置
    Q_EMIT sendHightlightPos(QPoint(startPoint.x() + m_spaceWidth, startPoint.y() + m_spaceHeight),
                             QPoint(startPoint.x() + m_hightlightSize.width() + m_spaceWidth,
                                    startPoint.y() + m_hightlightSize.height() + m_spaceHeight));
    //发送到导航器
    Q_EMIT showNavigation(QPixmap::fromImage(image));
    m_backHightLight = startPoint;
}

void Core::qmlTiffPage(int pageNum, QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale)
{
    m_tiffPage = pageNum;
    qmlNavigation(leftUpPosAccordingShowImage, displaySize, imageScale);
}

void Core::qmlTiffImagePage(int pageNum)
{
    m_tiffPage = pageNum;
}

bool Core::qmlNaviCheckBoundary(QSizeF visibleSize, QPointF curVisiblePartLeftUpPos, double imageScale)
{
    if (visibleSize.width() > m_nowImage.width() * imageScale
        || visibleSize.height() > m_nowImage.height() * imageScale) {
        return false;
    }
    if (curVisiblePartLeftUpPos.x() > m_nowImage.width() * imageScale
        || curVisiblePartLeftUpPos.y() > m_nowImage.height() * imageScale) {
        return false;
    }
    return true;
}
void Core::clickNavigation(const QPoint &point, bool isDragImage)
{
    bool hasArg = true;
    //无参输入则使用上次的位置
    if (point == QPoint(-1, -1)) {
        hasArg = false;
    }

    //有参则记录，无参使用上一次的点
    if (hasArg) {
        m_clickBeforePosition = point;
    }

    //计算点击区域——鼠标要在高亮区域中央，且要减去导航栏窗口与图片边缘的距离
    QPoint startPoint;
    startPoint.setX(m_clickBeforePosition.x() - m_hightlightSize.width() / 2 - m_spaceWidth);
    startPoint.setY(m_clickBeforePosition.y() - m_hightlightSize.height() / 2 - m_spaceHeight);

    int right = m_navigationImage.width() - m_hightlightSize.width();    //右侧边缘
    int bottom = m_navigationImage.height() - m_hightlightSize.height(); //下侧边缘

    //过滤无效区域
    if (startPoint.x() < 0) {
        startPoint.setX(0);
    }
    if (startPoint.y() < 0) {
        startPoint.setY(0);
    }
    if (startPoint.x() > right) {
        startPoint.setX(right);
    }
    if (startPoint.y() > bottom) {
        startPoint.setY(bottom);
    }

    //    //有参情况下，和上次点击的有效区域一致则不处理,同时区分
    //    if ((startPoint == m_clickBeforeStartPosition) && hasArg && !m_isClickBtnChangeImageSize) {
    //        return;
    //    }
    m_clickBeforeStartPosition = startPoint;

    //处理导航器图片
    QImage image = Processing::pictureDeepen(m_navigationImage, m_hightlightSize, startPoint);

    //发送到导航器
    Q_EMIT showNavigation(QPixmap::fromImage(image));
    //发送到导航器设置
    Q_EMIT sendHightlightPos(QPoint(startPoint.x() + m_spaceWidth, startPoint.y() + m_spaceHeight),
                             QPoint(startPoint.x() + m_hightlightSize.width() + m_spaceWidth,
                                    startPoint.y() + m_hightlightSize.height() + m_spaceHeight));

    //处理待显示区域
    m_startShowPoint = startPoint * m_tmpSize.width() / m_navigationImage.width();
    //拿左上角实际坐标
    //    m_leftUpPos = -m_startShowPoint + m_clickBeforeStartPosition;
    m_leftUpPos = -m_startShowPoint;
    //如果是动图，则交给动图显示事件去处理，避免闪屏
    if (m_playMovieTimer->isActive()) {
        return;
    }
    //    m_imageNeedUpdate = isDragImage;
    QPixmap result = Processing::localAmplification(m_nowImage, m_tmpSize, m_startShowPoint, m_size);
    showImage(result);
}

void Core::saveMovieFinish(const QString &path)
{
    //如果收到关闭信号，保存完再退出
    if (m_shouldClose) {
        //双层判断，避免多次确认状态，提升效率
        if (File::allSaveFinish()) {
            progremExit();
        }
        return;
    }

    //如果当前播放的不是这张图
    if (m_nowpath != path) {
        return;
    }
    m_thisImageIsSaving = false;
    openImage(path);
}

void Core::tiffOperateFinish(const QString &path)
{
    //如果收到关闭信号，保存完再退出
    if (m_shouldClose) {
        progremExit();
        return;
    }
    m_tiffProcess.clear();
    m_thisImageIsSaving = false;
    m_tiffHasOperatedFinish = true;
    //如果当前播放的不是这张图
    if (m_nowpath != path) {
        return;
    }
    openImage(path);
}
//处理上一次图片没处理完，进行下一次操作的时候，更新对齐动图每一帧的操作状态
void Core::processNewLoadImage()
{
    // m_willProcess为空，说明上一次处理完毕，不需要进行对齐更新
    if (m_willProcess.isEmpty()) {
        return;
    }
    //从未处理处开始进行对齐，m_willProcessNum为上一次处理到第几帧
    for (int i = m_willProcessNum; i < m_matList->length(); i++) {
        //遍历队列，对剩下帧进行同样的操作，保持所有帧一致
        for (Processing::FlipWay &tmpWay : m_willProcess) {
            Mat mat = Processing::processingImage(Processing::flip, m_matList->at(i), QVariant(tmpWay));
            m_matList->replace(i, mat);
        }
    }
}

void Core::flipImage(const Processing::FlipWay &way)
{
    //如果此图正在保存
    if (m_thisImageIsSaving) {
        return;
    }
    m_rotateRecord.append(way);
    m_processed = true;

    processNewLoadImage();

    //如果是动图，则批量处理
    if (m_playMovieTimer->isActive()) {
        for (int i = 0; i < m_matList->length(); i++) {
            Mat mat = Processing::processingImage(Processing::flip, m_matList->at(i), QVariant(way));
            m_matList->replace(i, mat);
        }
        //判断此次操作是否未加载完全，如是，记录此时的加载帧数和操作方式
        if (m_maxFrame > m_matList->length()) {
            m_willProcessNum = m_matList->length();
            m_willProcess.append(way);
        } else {
            m_willProcess.clear();
        }
        m_imageNeedUpdate = false;
        //刷新导航栏
        m_nowImage = Processing::converFormat(m_matList->first());
        creatImage(-1, true);
        return;
    }
    Mat mat;
    if (m_matList != nullptr && (m_paperFormat == "tiff" || m_paperFormat == "tif")) {
        m_tiffProcess.append(way);
        m_tiffProcessPath = m_nowpath;
        for (int i = 0; i < m_matList->length(); i++) {
            Mat tempmat = Processing::processingImage(Processing::flip, m_matList->at(i), QVariant(way));
            m_matList->replace(i, tempmat);
        }
        for (int j = 0; j < m_matList->length(); j++) {
            Mat tempmat1 = m_matList->at(j);
            QPixmap nowIndexPix = Processing::converFormat(tempmat1);
            m_tiffImageList.replace(j, nowIndexPix.toImage());
        }
        mat = m_matList->at(m_tiffPage - 1);
    } else {
        mat = Processing::processingImage(Processing::flip, m_nowMat, QVariant(way));
    }
    if (!mat.data) {
        return;
    }
    m_imageNeedUpdate = true;
    mat = changeMat(mat);
    m_nowImage = Processing::converFormat(mat);
    creatImage(-1, true);
}

void Core::deleteImage()
{
    //如果此图正在保存
    if (m_thisImageIsSaving) {
        return;
    }

    m_processed = false; //重置是否操作过的状态,都已经删除了，就没必要存了

    if (!m_isCanDel) {
        return;
    }
    QFileInfo fileInfo(m_nowpath);
    if (fileInfo.suffix().toLower() == m_nowRealFormat || m_loadedImage.contains(m_nowpath)) {
        m_needDel = true;
    } else {
        m_needDel = false;
    }
    qDebug() << "deleteImage:" << m_needDel;
    if (m_allImageList.contains(m_nowpath)) {
        m_allImageList.removeAll(m_nowpath);
        m_imageDelPath = m_nowpath;
    }
    //切换到下一张
    changeImage(NEXT_IMAGE);

    setHighLight(m_nowpath);

    //删除后队列中无图片，返回状态
    if (m_albumModel->rowCount() == 1) {
        //        navigation(QPoint(-1, -1), true); //关闭导航器
        showImage(QPixmap()); //发送状态
        return;
    }
}

void Core::setAsBackground()
{
    //设置为背景图
    if (QGSettings::isSchemaInstalled(SET_BACKGROUND_PICTURE_GSETTINGS_PATH)) {
        QGSettings *background = new QGSettings(SET_BACKGROUND_PICTURE_GSETTINGS_PATH);
        QStringList keyList = background->keys();
        if (keyList.contains(SET_BACKGROUND_PICTURE_GSETTINGS_NAME)) {
            QString nowPath = background->get(SET_BACKGROUND_PICTURE_GSETTINGS_NAME).toString();
            if (nowPath == m_nowpath) {
                background->set(SET_BACKGROUND_PICTURE_GSETTINGS_NAME, QString(""));
            }
            background->set(SET_BACKGROUND_PICTURE_GSETTINGS_NAME, m_nowpath);
        }
        background->deleteLater();
    }
}

void Core::openInfile()
{
    QDBusInterface interface("org.freedesktop.FileManager1", "/org/freedesktop/FileManager1",
                             "org.freedesktop.FileManager1", QDBusConnection::sessionBus());
    QStringList items;
    items.push_back(m_nowpath);
    QString startUpId = "";
    interface.call("ShowItems", items, startUpId);
}

void Core::changeOpenIcon(QString theme)
{
    Variable::g_themeStyle = theme;
    m_theme = theme;
    //将第一张图设置为打开按钮
    if (m_item0 == nullptr) {
        m_item0 = new MyStandardItem;
        m_item0->setDragEnabled(false);
        m_item0->setSizeHint(Variable::ALBUM_IMAGE_SIZE);
        m_item0->setToolTip(tr("Add"));
        m_albumModel->insertRow(0, m_item0); //插入model
    }
    if ("lingmo-dark" == theme || "lingmo-black" == theme) {
        if (m_item0 != nullptr) {
            m_item0->setIcon(m_item0Black); //用来保存地址路径
            m_openImage = QPixmap(":/res/res/damaged_imgB.png");
            m_item0->setBackground(QBrush(Qt::black));
        }
    } else {
        if (m_item0 != nullptr) {
            m_item0->setIcon(m_item0White); //用来保存地址路径
            m_openImage = QPixmap(":/res/res/damaged_img.png");
            m_item0->setBackground(QBrush(Qt::white));
        }
    }
    if (!m_openSuccess) {
        showImage(m_openImage);
    }
    for (int i = 0; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item == nullptr) {
            continue;
        }
        if (Variable::g_damagedImage.contains(item->getPath())) {
            item->setIcon(m_openImage);
        }
        if ("lingmo-dark" == theme || "lingmo-black" == theme) {
            item->setBackground(QBrush(Qt::black));

        } else {
            item->setBackground(QBrush(Qt::white));
        }
    }
}

void Core::close()
{

    //如果已经触发过关闭事件不响应
    if (m_shouldClose) {
        return;
    }
    //如果正在裁剪,且还未保存--暂时逻辑，标题栏已隐藏，暂时不用处理
    //    if (m_isCutting && m_isCutSaved) {
    //        Q_EMIT isCutNotSave();
    //    }
    //如果没有操作且有正在保存的动图直接退出
    if (!m_processed) {
        if (!File::allSaveFinish()) {
            if (m_playMovieTimer->isActive()) {
                m_playMovieTimer->stop();
            }

            m_shouldClose = true;
            showImage(QPixmap());
            return;
        }
        progremExit();
        return;
    }
    if (m_tiffProcess.length() > 0) {
        this->tiffOperate();
        m_shouldClose = true;
        showImage(QPixmap());
        return;
    }
    //如果不是作为外部API调用则默认覆盖保存
    if (!m_isApi) {
        m_apiReplaceFile = true;
    }
    m_isclose = true;
    Variable::g_isClosing = m_isclose;
    //如果正在播放动图，则停止
    if (m_playMovieTimer->isActive()) {
        m_playMovieTimer->stop();
        showImage(QPixmap());
        //判断没有加载完全部时，进行阻塞
        while (m_maxFrame > m_matList->length()) {
            QThread::usleep(10);
        }
        processNewLoadImage();
        //保存动图
        File::saveImage(m_matList, m_fps, m_nowpath, m_nowRealFormat, m_apiReplaceFile);
        m_shouldClose = true;
    } else {
        //保存图片
        File::saveImage(m_nowMat, m_nowpath, m_nowRealFormat, m_apiReplaceFile);
        QString suffix = QFileInfo(m_nowpath).suffix().toLower();
        if (suffix == "apng" || suffix == "png" || suffix == "gif") {
            if (m_matList != nullptr) {
                if (m_matList->length() > 1) {
                    m_shouldClose = true;
                    return;
                }
            }
            //解决git静态图无法保存的问题，已测试，界面可正常关掉，png和apng可正常保存
            if (suffix == "gif") {
                m_shouldClose = true;
                return;
            }
        }
        progremExit();
    }
}

QStandardItemModel *Core::getAlbumModel()
{
    return m_albumModel;
}

void Core::changeImage(const int &type)
{
    //    operayteMode = OperayteMode::ChangeImage;
    //如果错误不处理
    if (type == ERROR_IMAGE) {
        Q_EMIT updateSideNum(m_albumModel->rowCount());
        return;
    }

    //如果图片队列小于1，不处理。小于2时，相册文件清空时，会一直显示当前图片
    if (m_albumModel->rowCount() < 2) {
        m_backpath = m_nowpath;
        return;
    }
    if (m_isclose == true) {
        return;
    }
    needSave();

    if (type == NEXT_IMAGE) {
        QString key = nextImagePath(m_nowpath);
        changeImageType(key);

        if(m_needDel) {
            //从队列中去除
            deleteAlbumItem(m_backpath);
            //更新相册列表
            Q_EMIT updateSideNum(m_albumModel->rowCount());
            //清空相册直接返回
            if (m_albumModel->rowCount() < 2) {
                m_backpath = m_nowpath;
                return;
            }
            m_needDel = false;
        }

        openImage(m_nowpath);
        loadOnChange(m_nowpath);
        return;
    }
    if (type == BACK_IMAGE) {
        QString key = backImagePath(m_nowpath);
        changeImageType(key);
        openImage(m_nowpath);
        loadOnChange(m_nowpath);
        return;
    }

    //点击相册切换图片
    MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(type));
    changeImageType(item->getPath());
    openImage(m_nowpath);
    reloadImage(m_albumModel->indexFromItem(item));
    loadImgCache(m_albumModel->indexFromItem(item));
}

void Core::changeImageFromClick(QModelIndex modelIndex)
{
    MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->itemFromIndex(modelIndex));
    if (item == nullptr) {
        changeImage(modelIndex.row());
        //切换图片-埋点
        kdk::kabase::BuriedPoint buriedPointSwitchPicture;
        if (buriedPointSwitchPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                         kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSwitchPicture)) {
            qCritical() << "Error : buried point fail SwitchPicture!";
        }
        return;
    }
    if (item->getPath() == m_nowpath) {
        return;
    }
    //判断是按钮点击，则打开图片
    if (modelIndex.row() == 0) {
        Q_EMIT openFromAlbum();
        return;
    }
    //切换图片-埋点
    kdk::kabase::BuriedPoint buriedPointSwitchPicture;
    if (buriedPointSwitchPicture.functionBuriedPoint(kdk::kabase::AppName::LingmoPhotoViewer,
                                                     kdk::kabase::BuriedPoint::PT::LingmoPhotoViewerSwitchPicture)) {
        qCritical() << "Error : buried point fail SwitchPicture!";
    }
    m_tiffPage = 1;
    changeImage(modelIndex.row());
    //    openImage(item->getPath());
}

void Core::changeWidgetSize(const QSize &size)
{
    m_size = size;
    creatImage(-1, true);
}
void Core::changeImageShowSize(ImageShowStatus::ChangeShowSizeType type, QPoint focus)
{
    //如果此图正在保存
    if (m_thisImageIsSaving) {
        return;
    }
    m_imageNeedUpdate = false;
    m_zoomFocus = focus;
    int resizeKey;
    resizeKey = Variable::RESIZE_KEY;
    int tmpProportion = 0;
    switch (type) {

    ///----------------------------放大图片----------------------------
    case ImageShowStatus::BIG:
        if (m_proportion == Variable::RESIZE_KEY_MAX) { //等于临界值
            return;
        }
        if (m_proportion + resizeKey > Variable::RESIZE_KEY_MAX) { //将要超出临界值
            tmpProportion = Variable::RESIZE_KEY_MAX;
            creatImage(tmpProportion, false);
            return;
        }
        if (m_proportion % resizeKey != 0) { //不能整除
            tmpProportion = (m_proportion / resizeKey + 1) * resizeKey;
            creatImage(tmpProportion, false);
            return;
        }
        tmpProportion = m_proportion + resizeKey;
        creatImage(tmpProportion, false);
        return;

        ///----------------------------缩小图片----------------------------
    case ImageShowStatus::SMALL:
        if (m_proportion == Variable::RESIZE_KEY_MIN) { //等于临界值
            return;
        }
        if (m_proportion - resizeKey < Variable::RESIZE_KEY_MIN) { //将要超出临界值
            tmpProportion = Variable::RESIZE_KEY_MIN;
            creatImage(tmpProportion, false);
            return;
        }
        if (m_proportion % resizeKey != 0) { //不能整除
            tmpProportion = (m_proportion / resizeKey) * resizeKey;
            creatImage(tmpProportion, false);
            return;
        }
        tmpProportion = m_proportion - resizeKey;
        creatImage(tmpProportion, false);
        return;

        ///----------------------------查看原图----------------------------
    case ImageShowStatus::ORIGIN:
        creatImage(100, false);
        return;

        ///----------------------------查看自适应图----------------------------
    case ImageShowStatus::AUTO:
        creatImage(-1, false);
        return;

    default:
        qDebug() << "代码错误，请检查";
        break;
    }
}

void Core::changeImageType(QString path)
{
    m_matListIndex = 0;
    if (path == "") { //回滚
        deleteAlbumItem(m_nowpath);
        if (m_albumModel->rowCount() == 1) {
            m_backpath = m_nowpath;
            m_nowpath = "";
            return;
        }
        m_nowpath = m_backpath;
        return;
    }
    m_backpath = m_nowpath;
    m_nowpath = path;
}

Mat Core::changeMat(Mat mat)
{
    m_backMat = m_nowMat;
    m_nowMat = mat;
    m_imageSize = QString::number(m_nowImage.width()) + "x" + QString::number(m_nowImage.height());
    switch (m_nowMat.type()) {
    case CV_8UC4:
        m_colorSpace = "RGBA";
        break;
    case CV_8UC3:
        m_colorSpace = "RGB";
        break;
    case CV_8UC1:
        m_colorSpace = "GRAY";
        break;
    }
    // gif走的是apng流程，所以这里要单独判断
    if (QFileInfo(m_nowpath).suffix().toLower() == "gif") {
        m_colorSpace = "RGB";
    }
    return m_nowMat;
}

void Core::creatNavigation()
{
    //导航栏背景
    QSize navigationSize = Variable::NAVIGATION_SIZE;
    m_navigationImage = Processing::resizePix(m_nowImage, navigationSize).toImage();

    //记录空白区域
    m_spaceWidth = (navigationSize.width() - m_navigationImage.width()) / 2;
    m_spaceHeight = (navigationSize.height() - m_navigationImage.height()) / 2;

    //待显示图
    m_tmpSize = m_nowImage.size() * m_proportion / 100;

    //高亮区域大小
    m_hightlightSize.setWidth(m_navigationImage.width() * m_size.width() / m_tmpSize.width());
    m_hightlightSize.setHeight(m_navigationImage.height() * m_size.height() / m_tmpSize.height());
    if (m_hightlightSize.width() > m_navigationImage.width()) {
        m_hightlightSize.setWidth(m_navigationImage.width());
    }
    if (m_hightlightSize.height() > m_navigationImage.height()) {
        m_hightlightSize.setHeight(m_navigationImage.height());
    }
    //缩放焦点换算   思路：保持变化前后焦点坐标基于待显示图的位置比值不变
    //导航栏图片起始点在导航栏的位置
    QPointF navigationImageStartPos(m_spaceWidth, m_spaceHeight);
    //变化前高亮区起始点（左上角坐标）
    QPointF clickBeforeStartPosition = m_clickBeforeStartPosition;
    if (clickBeforeStartPosition.x() < 0 || clickBeforeStartPosition.y() < 0) {
        //初次显示导航栏赋空值
        clickBeforeStartPosition = QPointF();
    } else {
        // m_clickBeforeStartPosition为不考虑空白区域的实际高亮区，故需要将空白区域考虑进去
        clickBeforeStartPosition += navigationImageStartPos;
    }
    //变化前待显示图和导航栏缩略图比例
    double displayAndThumbnailScaleBefore = double(m_nowImage.size().width()) * double(m_lastTimeProportion)
                                            / double(m_navigationImage.size().width()) / 100.00;
    //变化前待显示完整图（包含白边）大小
    QSizeF fullImageSizeBefore = displayAndThumbnailScaleBefore * QSizeF(navigationSize);
    //变化前高亮区域左上角位置相对于待显示完整图的坐标
    QPointF hightLightTopLeftPointBefore = clickBeforeStartPosition * displayAndThumbnailScaleBefore;
    //变化前焦点在待显示完整图的坐标
    QPointF tmpZoomFocusBefore = hightLightTopLeftPointBefore + QPointF(m_zoomFocus);
    //变化前后待显示图大小比值
    double displayAndThumbnailScale = double(m_proportion) / double(m_lastTimeProportion);
    //变化后待显示完整图大小
    QSizeF fullImageSizeAfter = fullImageSizeBefore * displayAndThumbnailScale;
    //变化后焦点在待显示完整图的坐标
    QPointF tmpZoomFocusAfter = tmpZoomFocusBefore * displayAndThumbnailScale;
    //变化后高亮区域左上角坐标
    QPointF hightLightTopLeftPointAfter = tmpZoomFocusAfter - m_zoomFocus;
    //变化后在待显示完整图上的虚拟点击坐标
    QPointF tmpClickPoint(hightLightTopLeftPointAfter.x() + m_size.width() / 2,
                          hightLightTopLeftPointAfter.y() + m_size.height() / 2);
    //变化后待显示完整图和导航栏尺寸比值
    double displayAndThumbnailScaleAfter = double(fullImageSizeAfter.width()) / double(navigationSize.width());
    //以虚拟点坐标计算导航栏的实际点击坐标
    QPointF zoomFocus = QPointF(tmpClickPoint / displayAndThumbnailScaleAfter);
    //减小误差
    zoomFocus.setX(zoomFocus.x() - 0.5);
    zoomFocus.setY(zoomFocus.y() - 0.5);
    //得出实际点击坐标
    m_zoomFocus = zoomFocus.toPoint();
    m_actualClickPos = hightLightTopLeftPointAfter;
}

void Core::deleteAlbumItem(const QString &path)
{
    if (path == "") {
        return;
    }

    if (m_loadedImage.contains(path)) {
        m_loadedImage.removeAt(m_loadedImage.indexOf(path));
    }
    if (m_allImageList.contains(path)) {
        m_allImageList.removeAt(m_allImageList.indexOf(path));
    }
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == path) {
            if (m_nowpath == "" && m_backpath == "") {
                m_backpath = dynamic_cast<MyStandardItem *>(m_albumModel->item(i - 1))->getPath();
            }
            m_albumModel->removeRow(i);
            return;
        }
    }
}

Enums::ChamgeImageType Core::nextOrBack(const QString &oldPath, const QString &newPath)
{

    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == oldPath) {
            return NEXT_IMAGE;
        }
        if (item->getPath() == newPath) {
            return BACK_IMAGE;
        }
    }
    qDebug() << "判断切换状态错误";
    return ERROR_IMAGE;
}

Enums::RenameState Core::successOrFail(const QString &oldPath, const QString &newPath)
{
    QFile file(oldPath);
    bool ok = file.rename(oldPath, newPath);
    //改名
    if (ok) {
        QFileInfo fileToCore(newPath);
        QString fileNewName = fileToCore.absoluteFilePath();
        //更新相册，m_nowpath，m_info信息为重命名后的图片
        m_nowpath = fileNewName;
        m_info = fileToCore;
        for (int i = 1; i < m_albumModel->rowCount(); i++) {
            MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
            if (item->getPath() == oldPath) {
                item->setPath(fileNewName);
            }
        }
        return SUCCESS;
    } else {
        if (!file.exists()) {
            qDebug() << "文件不存在";
            return NOT_EXITS;
        }
        if (QFile::exists(newPath)) {
            qDebug() << "文件同名";
            return SAME_NAME;
        }
        return UNKNOWN_ERROR;
    }
}

QString Core::nextImagePath(const QString &oldPath)
{
    //相册中没有文件时，不进行处理
    if (m_albumModel->rowCount() < 2) {
        return "";
    }

    MyStandardItem *item = nullptr;
    //最后一张切下一张时，回到队列开头
    item = dynamic_cast<MyStandardItem *>(m_albumModel->item(m_albumModel->rowCount() - 1));
    if (item->getPath() == oldPath) {
        item = dynamic_cast<MyStandardItem *>(m_albumModel->item(1));
        return item->getPath();
    }

    bool tmp = false;
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (tmp) {
            return item->getPath();
        }
        if (item->getPath() == oldPath) {
            tmp = true;
        }
    }
    return "";
}

QString Core::backImagePath(const QString &oldPath)
{
    //相册中没有文件时，不进行处理
    if (m_albumModel->rowCount() < 2) {
        return "";
    }

    MyStandardItem *item = nullptr;
    //第一张切上一张时，回到队列结尾
    item = dynamic_cast<MyStandardItem *>(m_albumModel->item(1));
    if (item->getPath() == oldPath) {
        item = dynamic_cast<MyStandardItem *>(m_albumModel->item(m_albumModel->rowCount() - 1));
        return item->getPath();
    }

    QString tmp = "";
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        QString tmp2 = item->getPath();
        if (tmp2 == oldPath) {
            return tmp;
        }
        tmp = tmp2;
    }
    return "";
}

void Core::progremExit()
{
    Q_EMIT coreProgremExit();
}

void Core::albumLoadFinish(QVariant var)
{
    ImageAndInfo package = var.value<ImageAndInfo>();
    QString fileName = package.info.absoluteFilePath();
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == fileName) {
            if (package.image.isNull()) {
                m_albumModel->removeRow(i);
                return;
            }
            m_albumModel->item(i)->setIcon(package.image);
            return;
        }
    }
}

void Core::albumLoadRealFinish(QVariant var)
{
    QPair<MatAndFileinfo, QPixmap> matPix = var.value<QPair<MatAndFileinfo, QPixmap>>();
    MatAndFileinfo maf = matPix.first;
    QString fileName = maf.info.absoluteFilePath();
    if (m_loadedImageCache.find(fileName) != m_loadedImageCache.end())
    {
        return;
    }

    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == fileName) {
            QPixmap pix = matPix.second;
            if (pix.isNull()) {
                return;
            }

            QPair<MatAndFileinfo, QPixmap> matPix = var.value<QPair<MatAndFileinfo, QPixmap>>();
            m_loadedImageCache.insert(fileName, matPix);
            return;
        }
    }
}

//重命名
void Core::toCoreChangeName(QString oldName, QString newName)
{
    //存原图后缀
    QFileInfo file(oldName);
    QString oldSuffix = file.suffix();
    if ("" != file.suffix()) {
        oldSuffix = "." + oldSuffix;
    }
    //形成完整路径
    QString path = file.absolutePath();
    QString newPath = path + "/" + newName;
    newPath = newPath + oldSuffix;
    //判断是否重命名成功，并返回重命名结果
    RenameState type = successOrFail(oldName, newPath);
    //重命名返回结果---给前端发信号改名或错误提示
    if (type == SUCCESS) { //重命名成功
        m_allImageList.replace(m_allImageList.indexOf(oldName), newPath);
        QFileInfo fileToCore(newPath);
        Q_EMIT renameResult(0, fileToCore);
    } else if (type == NOT_EXITS) { //文件不存在
        Q_EMIT renameResult(-1, file);
    } else if (type == SAME_NAME) { //文件同名
        Q_EMIT renameResult(-2, file);
    } else if (type == NO_PEIMISSION) { //没有权限----暂时算为其他未知错误中，待以后解决
        Q_EMIT renameResult(-3, file);
    } else { //其他未知错误
        Q_EMIT renameResult(-4, file);
    }
}

void Core::toPrintImage(QPrinter *printer)
{
    if (m_paperFormat != "tiff" && m_paperFormat != "tif") {
        //图片使用原图进行打印
        QImage img = m_nowImage.toImage();
        QPainter painter(printer);
        QRect rect = painter.viewport();
        QSize size = img.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(img.rect());
        painter.drawImage(0, 0, img);
    } else {
        // 把mat读出来，放到matlist里面
        MatAndFileinfo maf = File::loadImage(m_nowpath);
        QList<Mat> matlist;
        if (maf.matList == nullptr) {
            matlist.append(maf.mat);
        } else {
            for (int i = 0; i < maf.matList->size(); i++) {
                matlist.append(maf.matList->at(i));
            }
        }

        // 设置开始结束页码
        int fromPage;
        int toPage;
        switch (printer->printRange()) {
        case QPrinter::PageRange:
            fromPage = max(printer->fromPage() - 1, 0);
            toPage = min(printer->toPage() - 1, matlist.size() - 1);
            break;
        default:
            fromPage = 0;
            toPage = matlist.size() - 1;
            break;
        }

        // 把页面画到painter里面
        int cnt = 0;
        QPainter painter(printer);
        QRect rect = painter.viewport();
        for (int i = fromPage; i <= toPage; i++) {
            Mat mat = matlist.at(i);
            if (cnt != 0) {
                printer->newPage();
            }
            QPixmap pix = Processing::converFormat(mat);
            QImage img = pix.toImage();
            QSize size = img.size();
            size.scale(rect.size(), Qt::KeepAspectRatio);
            painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            painter.setWindow(img.rect());
            painter.drawImage(0, 0, img);
            cnt++;
        }
    }
}
//打开失败，判断是否为拖拽打开失败--是的话，不进行切换
bool Core::openFail(QString fullPath)
{
    //注--此函数只有在打开失败的时候才进来，不影响正常的图片
    QStringList m_pathList;
    //遍历整个相册
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        m_pathList.append(item->getPath());
    }
    //如果不包含当前路径，则说明是外部拖入
    if (!m_pathList.contains(fullPath)) {
        return true;
    } else {
        return false;
    }
}
//裁剪，处理显示的图片
void Core::cutImage()
{
    m_isCutting = true;
    QImage image = Processing::pictureCutDeepen(m_origCutImage, m_origCutImage.size());
    showImage(QPixmap::fromImage(image));
}
//裁剪-显示区域-全图裁剪
void Core::cutFullViableImage()
{
    m_cutBeforeStartPosition = QPoint(0, 0);
    cutRegion(QPoint(m_origCutImage.width(), m_origCutImage.height()));
    realsePos(QPoint(0, 0));
}

void Core::startMark()
{
    m_markTool.setPixmap(m_origPix);
}

void Core::markImage(QVariant var)
{
    MarkPainterSet markImage = var.value<MarkPainterSet>();
    if (7 == markImage.type) {
        auto *mob = new Mark::MarkOperatioBlur(markImage.painterColor, markImage.rectInfo, 1, markImage.thickness);
        m_markTool.setMarkOperatio(mob);
    } else if (6 == markImage.type) {
        auto *mot = new Mark::MarkOperatioText(
            markImage.painterColor, markImage.m_textSet.textContent, markImage.recordStartPos,
            markImage.m_textSet.fontSize, 1, markImage.m_textSet.fontType, markImage.m_textSet.isBold,
            markImage.m_textSet.isItalics, markImage.m_textSet.isUnderLine, markImage.m_textSet.isDeleteLine);
        m_markTool.setMarkOperatio(mot);
    } else if (5 == markImage.type) {
        auto *mom = new Mark::MarkOperatioMarker(markImage.painterColor, markImage.recordStartPos,
                                                 markImage.recordEndPos, 1, markImage.thickness, markImage.opactity);
        m_markTool.setMarkOperatio(mom);
    } else if (4 == markImage.type) {
        auto *mop = new Mark::MarkOperatioPencil(markImage.painterColor, markImage.vp, 1, markImage.thickness);
        m_markTool.setMarkOperatio(mop);
    } else if (3 == markImage.type) {
        auto *moa =
            new Mark::MarkOperatioArrow(markImage.painterColor, markImage.recordStartPos, markImage.recordEndPos, 1,
                                        markImage.thickness, markImage.arrowStartPos, markImage.arrowEndPos);
        m_markTool.setMarkOperatio(moa);
    } else if (2 == markImage.type) {
        auto *mol = new Mark::MarkOperatioLine(markImage.painterColor, markImage.recordStartPos, markImage.recordEndPos,
                                               1, markImage.thickness);
        m_markTool.setMarkOperatio(mol);
    } else if (1 == markImage.type) {
        auto *moc = new Mark::MarkOperatioCircle(markImage.painterColor, markImage.circleLeftX, markImage.circleLeftY,
                                                 markImage.paintWidth, markImage.paintHeight, 1, markImage.thickness);
        m_markTool.setMarkOperatio(moc);
    } else if (0 == markImage.type) {
        auto *mor = new Mark::MarkOperatioRectangle(markImage.painterColor, markImage.rectInfo, 1, markImage.thickness);
        m_markTool.setMarkOperatio(mor);
    } else {
        qDebug() << "标注类型错误";
    }
    //    m_markTool.undo();
    //    m_markTool.save("/home/zou/桌面/test.png");
}

void Core::markUndo()
{
    m_markTool.undo();
}

void Core::markSave(QString savePath)
{
    m_markTool.save(savePath, m_rotateRecord, false);
}

void Core::markClear()
{
    m_markTool.clear();
}

//裁剪区域
void Core::cutRegion(const QPoint &point)
{
    //起始点变化
    if (m_cutBeforeStartPosition != m_cutStartPostionSave && m_cutStartPostionSave.x() != -1
        && m_cutStartPostionSave.y() != -1) {
        //多个起点
        return;
    }
    //判断是否是边界
    QPoint changePoint = boundingSet(point, m_origCutImageBefore.width(), m_origCutImageBefore.height());
    if (m_backPointRelease.x() != -1 && m_backPointRelease.y() != -1) {

        m_backPointRelease =
            boundingSet(m_backPointRelease, m_origCutImageBefore.width(), m_origCutImageBefore.height());
    }
    //判断只是点击一下，不进行绘制
    if (changePoint == m_cutBeforeStartPosition) {
        return;
    }
    //高亮区域
    m_cuthightlightSize =
        QSize(abs(point.x() - m_cutBeforeStartPosition.x()), abs(point.y() - m_cutBeforeStartPosition.y()));
    //处理待显示的图片
    QImage image = Processing::pictureCutImageDeepen(m_origCutImage, m_cuthightlightSize, m_cutBeforeStartPosition,
                                                     m_origCutImageBefore, m_cuthightlightSizeBefore, changePoint);
    //保存上一个区域
    m_cuthightlightSizeBefore = m_cuthightlightSize;
    //保存上一个起始点
    m_cutStartPostionSave = m_cutBeforeStartPosition;
    m_isReleasePaint = false; //非释放鼠标的时候要重置
    m_isCutSaved = true;      //开始裁剪当并未保存
    //重置上一个点
    m_backPointRelease.setX(-1);
    m_backPointRelease.setY(-1);
    showImage(QPixmap::fromImage(image));
    //将当前画完框的图像存起来
    m_cutImage = image;
}
//记录起始点
void Core::startChange(const QPoint &point)
{
    m_cutBeforeStartPosition = point;
}
//释放鼠标
void Core::realsePos(const QPoint &point)
{
    //判断裁剪框已经画好了，就不需要再次重绘
    if (m_hasCutBox) {
        return;
    }
    m_isReleasePaint = false; //释放鼠标的时候要重置
    //重置上一个点
    m_backPointRelease.setX(-1);
    m_backPointRelease.setY(-1);
    QImage image = Processing::pictureRelease(m_cutImage, m_origCutImageBefore);
    showImage(QPixmap::fromImage(image));
    m_startP = Processing::releaseStart();
    m_endP = Processing::releaseEnd();
    Q_EMIT releaseRegion(m_startP, m_endP);
}

void Core::releaseCutChange(int type, const QPoint &point)
{
    //鼠标没有释放，状态不能改变，保证鼠标一直是当前这个状态
    if (m_isReleasePaint) {
        type = m_backType;
        //给前端发信号，设置鼠标状态
        Q_EMIT returnMouseType(type);
    }
    //边界判断
    QPoint changPoint = boundingSet(point, m_origCutImageBefore.width(), m_origCutImageBefore.height());

    if (m_backPointRelease.x() != -1 && m_backPointRelease.y() != -1) {

        m_backPointRelease =
            boundingSet(m_backPointRelease, m_origCutImageBefore.width(), m_origCutImageBefore.height());
    }
    QImage image = Processing::pictureCutChange(type, m_backType, m_cutImage, m_startP, m_endP, changPoint,
                                                m_backPointRelease, m_origCutImageBefore);
    m_isReleasePaint = true;
    showImage(QPixmap::fromImage(image));
    m_cutImage = image;
    m_backType = type;
    m_backPointRelease = point;
}

void Core::exitCut()
{
    showImage(QPixmap::fromImage(m_origCutImageBefore));
    m_origCutImage = m_origCutImageBefore;
    //状态重置
    cutFinishReset();
}
//裁剪后保存
void Core::needSaveCutImage(int proportion)
{
    QPointF startPosition =
        QPointF(static_cast<double>(m_startP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                static_cast<double>(m_startP.y()) / (Variable::PERCENTAGE * static_cast<double>(proportion)));
    QPointF endPosition =
        QPointF(static_cast<double>(m_endP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                static_cast<double>(m_endP.y()) / (static_cast<double>(proportion) * Variable::PERCENTAGE));

    int imageWidth = endPosition.x() - startPosition.x();
    int imageHeight = endPosition.y() - startPosition.y();
    // 确定最终裁剪区域
    CutRegion cutRegion = cutRegionSelection(startPosition, QSize(imageWidth, imageHeight), m_nowImage);
    cv::Mat tempMat =
        m_cutMat(cv::Rect(cutRegion.startPostion.x(), cutRegion.startPostion.y(), cutRegion.width, cutRegion.height))
            .clone();
    Q_EMIT cutSaveFinish();
    //状态重置
    cutFinishReset();
    if (File::saveImage(tempMat, m_nowpath, m_nowRealFormat, false)) {
        openSaveImage();
    } else {
        qDebug() << "裁剪失败";
    }
}

void Core::saveCutImage(int proportion, const QPoint &startP, const QPoint &endP)
{
    m_isSaveCutImage = true;
    QPointF startPosition =
        QPointF(static_cast<double>(startP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                static_cast<double>(startP.y()) / (static_cast<double>(proportion) * Variable::PERCENTAGE));
    QPointF endPosition =
        QPointF(static_cast<double>(endP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                static_cast<double>(endP.y()) / (static_cast<double>(proportion) * Variable::PERCENTAGE));

    int imageWidth = endPosition.x() - startPosition.x();
    int imageHeight = endPosition.y() - startPosition.y();
    // 确定最终裁剪区域
    CutRegion cutRegion = calcCutRegionSelection(startPosition, QSize(imageWidth, imageHeight),
                                             Processing::converFormat(m_replaceMat));
    m_mattingCutMat =
        m_replaceMat(cv::Rect(cutRegion.startPostion.x(), cutRegion.startPostion.y(),
                          cutRegion.width, cutRegion.height)).clone();
    if (proportion > 100) {
        cv::resize(m_mattingCutMat, m_mattingCutMat, cv::Size(endP.x()-startP.x(), endP.y()-startP.y()), cv::INTER_LINEAR);
    }
    m_replaceMat = m_mattingCutMat.clone();
    finishMatting(m_mattingCutMat);
}
//确定最终的裁剪区域，返回起始点和裁剪宽度信息
CutRegion Core::cutRegionSelection(QPointF startPos, QSize cutSize, QPixmap nowImage)
{
    CutRegion cutRegion;
    //确定起始点是否已经是图片内最大的点或超过最大的点，起始点不能是边界
    if (startPos.x() >= nowImage.width()) {
        //防止越界
        startPos.setX(nowImage.width() - 1);
    }
    if (startPos.y() >= nowImage.height()) {
        //防止越界
        startPos.setY(nowImage.height() - 1);
    }
    //判断当前裁剪区域是否超过图片区域
    if (startPos.x() + cutSize.width() > m_nowImage.width()) {
        cutSize.setWidth(m_nowImage.width() - startPos.x());
    }
    if (startPos.y() + cutSize.height() > m_nowImage.height()) {
        cutSize.setHeight(m_nowImage.height() - startPos.y());
    }
    //裁剪区域的宽度和高度不能小于等于0
    if (cutSize.width() <= 0) {
        cutSize.setWidth(CutMinValue::minValue);
    }
    if (cutSize.height() <= 0) {
        cutSize.setHeight(CutMinValue::minValue);
    }
    //存到结构体
    cutRegion.startPostion = startPos;
    cutRegion.width = cutSize.width();
    cutRegion.height = cutSize.height();
    return cutRegion;
}

CutRegion Core::calcCutRegionSelection(QPointF startPos, QSize cutSize, QPixmap nowImage)
{
    CutRegion cutRegion;
    //确定起始点是否已经是图片内最大的点或超过最大的点，起始点不能是边界
    if (startPos.x() >= nowImage.width()) {
        //防止越界
        startPos.setX(nowImage.width() - 1);
    }
    if (startPos.y() >= nowImage.height()) {
        //防止越界
        startPos.setY(nowImage.height() - 1);
    }
    //判断当前裁剪区域是否超过图片区域
    if (startPos.x() + cutSize.width() > nowImage.width()) {
        cutSize.setWidth(nowImage.width() - startPos.x());
    }
    if (startPos.y() + cutSize.height() > nowImage.height()) {
        cutSize.setHeight(nowImage.height() - startPos.y());
    }
    //裁剪区域的宽度和高度不能小于等于0
    if (cutSize.width() <= 0) {
        cutSize.setWidth(CutMinValue::minValue);
    }
    if (cutSize.height() <= 0) {
        cutSize.setHeight(CutMinValue::minValue);
    }
    //存到结构体
    cutRegion.startPostion = startPos;
    cutRegion.width = cutSize.width();
    cutRegion.height = cutSize.height();
    return cutRegion;
}

//另存为
void Core::needSaveAsImage(QString path)
{
    QFileInfo file(path);
    if (Variable::g_needSaveas && file.suffix().toLower() == "jpeg" || file.suffix().toLower() == "jpg"
        || file.suffix().toLower() == "jpe") {
        File::changeSaveSign(Variable::g_needSaveas);
    }
    Variable::g_needSaveas = false;
    if (File::saveImage(m_nowMat, QString(path), file.suffix().toLower())) {
        openSaveImage();
    } else {
        //另存为失败
        qDebug() << "另存为失败";
    }
}

//处理边界问题
QPoint Core::boundingSet(QPoint point, int xMax, int yMax)
{
    if (point.x() > xMax) {
        point.setX(xMax);
    }
    if (point.y() > yMax) {
        point.setY(yMax);
    }
    if (point.x() < 0) {
        point.setX(0);
    }
    if (point.y() < 0) {
        point.setY(0);
    }
    return point;
}

void Core::cutFinishReset()
{
    //状态重置
    m_isCutting = false;
    m_isCutSaved = false;
    m_isReleasePaint = false;
    m_hasCutBox = false;
    m_backPointRelease = QPoint(-1, -1);
    m_cutStartPostionSave = QPoint(-1, -1);  //保存前一个裁剪的起始点
    m_cuthightlightSizeBefore = QSize(0, 0); //高亮区域大小-上一次的
    m_cutspaceWidth = 0;                     //导航栏窗口与缩略图左边缘距离
    m_cutspaceHeight = 0;                    //导航栏窗口与缩略图上边缘距离
    m_imageNeedUpdate = false;
}

void Core::openSaveImage()
{
    //裁剪和另存为之后打开保存的图片
    QString savePath = File::savePath();
    QFileInfo file(savePath);
    if (file.exists()) {
        findAllImageFromDir(savePath);
    } else {
        //给前端发信号处理-后续处理
        qDebug() << "保存图片打开失败" << savePath;
    }
}
//切换图片时加载-拿QModelIndex
void Core::loadOnChange(QString path)
{
    for (int i = 1; i < m_albumModel->rowCount(); i++) {
        MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
        if (item->getPath() == path) {
            reloadImage(m_albumModel->indexFromItem(item));
            loadImgCache(m_albumModel->indexFromItem(item));
        }
    }
}

bool Core::judgeSizeIsZero(QSize size)
{
    if (size.width() * size.height() == 0) {
        return true;
    }
    return false;
}


bool Core::apiFunction()
{
    if (!m_isApi) {
        return false;
    }
    QString fullPath = m_apiCmd[1];
    openImage(fullPath);
    //翻转
    if (m_apiCmd[0] == "-flip") {
        QString cmd = m_apiCmd[2];
        for (QChar &ch : cmd) {
            if (ch == 'h') {
                flipImage(Processing::FlipWay::horizontal);
            } else if (ch == 'v') {
                flipImage(Processing::FlipWay::vertical);
            } else if (ch == 'c') {
                flipImage(Processing::FlipWay::clockwise);
            }
        }
        close();
    }
    return true;
}
//判断是不是一个文件夹路径
bool Core::isSamePath(QString path)
{
    QString pathDir = QFileInfo(path).absolutePath(); //文件夹
    QStringList pathlist;
    //此处不能-1，会造成只有两张图片时，无法寻找第一张图片的路径，重复加载此路径下的图片
    int num = m_albumModel->rowCount();
    if (num > 0) {
        for (int i = 0; i < num; i++) {
            MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
            QFileInfo info(item->getPath());
            pathlist.append(info.absolutePath());
        }
        if (pathlist.contains(pathDir)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
//判断统一文件夹下有不同图片时，需要在相册显示
QStringList Core::diffPath(QStringList image)
{
    QStringList pathList;   //存相册的图片路径
    QStringList diffImages; //存不同图片
    int num = m_albumModel->rowCount();
    if (num > 0) {
        for (int i = 0; i < num; i++) {
            MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->item(i));
            QFileInfo info(item->getPath());
            if (info.exists()) {
                pathList.append(info.fileName());
            }
        }
    } else {
        return diffImages;
    }
    //遍历image列表
    for (int j = 0; j < image.length(); j++) {
        //判断，相册文件列表中无此时文件路径，存入diffImages中
        if (!pathList.contains(image.at(j))) {
            diffImages.append(image.at(j));
        }
    }
    return diffImages;
}

void Core::needSave()
{
    //解决侧栏切换和点击按钮打开图片时，上一张操作过的图片无法保存的问题
    m_thisImageIsSaving = false;

    //如果正在播放动图，则停止
    if (m_playMovieTimer->isActive()) {
        m_playMovieTimer->stop();
        //保存动图
        if (m_processed) {
            //判断没有加载完全部时，进行阻塞
            while (m_maxFrame > m_matList->length()) {
                QThread::usleep(10);
            }
            processNewLoadImage();
            File::saveImage(m_matList, m_fps, m_nowpath, m_nowRealFormat);
        }
    } else {

        //保存图片
        if (m_processed) {
            QString suffix = QFileInfo(m_nowpath).suffix().toLower();
            if (m_matList == nullptr) {
                File::saveImage(m_nowMat, m_nowpath, m_nowRealFormat);
            } else {
                if (m_tiffProcess.length() > 0) {
                    this->tiffOperate();
                }
            }
            //            File::saveImage(m_nowMat, m_nowpath);
        }
    }
    m_tiffProcess.clear();
    m_processed = false; //重置是否操作过的状态
}


void Core::findAllImageFromDir(QString fullPath)
{
    if (apiFunction()) {
        return;
    }
    QFileInfo info(fullPath);
    QString path = info.absolutePath();         //转成绝对路径
    QString filepath = info.absoluteFilePath(); //转成绝对路径
    QDir dir(path);                             //实例化目录对象
    QStringList nameFilters;                    //格式过滤
    for (const QString &format : Variable::SUPPORT_FORMATS) {
        nameFilters << "*." + format; //构造格式过滤列表
    }
    QStringList images = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name); //获取所有支持的图片
    //    QStringList images = dir.entryList(nameFilters, QDir::Files|QDir::Writable|QDir::Hidden,
    //    QDir::Name);//获取所有支持的图片
    //判断是同一文件夹下的图片，过滤掉相同的图片，加载文件夹下其他图片
    if (isSamePath(fullPath)) {
        images = diffPath(images);
    }
    m_imageListEveryAdd.clear();
    QString tmpFullPath;
    //先把完整路径存起来，方便对比
    for (int i = 0; i < images.length(); i++) {
        tmpFullPath = path + "/" + images.at(i);
        m_allImageList.insert(i, tmpFullPath);
        m_imageListEveryAdd.insert(i, tmpFullPath);
    }
    if (!m_allImageList.contains(fullPath)) {
        m_allImageList.insert(0, fullPath);
        images.insert(0, info.fileName());
        m_imageListEveryAdd.insert(0, tmpFullPath);
    }
    //更新图片类型
    needUpdateAddImageType(m_imageListEveryAdd);
    creatModule(path, images);
    openImage(filepath);

    //相册显示-加载缩略图，如果路径列表为空，其实不需要加载缩略图
    loadOnChange(fullPath);
}
//创建相册model
void Core::creatModule(const QString &path, QStringList list)
{
    //将所有图片存入队列
    int i = 1;

    for (QString &filename : list) {
        QString tmpFullPath = path + "/" + filename;
        //构造item
        MyStandardItem *item = new MyStandardItem;
        item->setDragEnabled(false);
        item->setSizeHint(Variable::ALBUM_IMAGE_SIZE);
        item->setIcon(QIcon::fromTheme("lingmo-photo-viewer"));//改为系统默认主题图标

        item->setPath(tmpFullPath);       //用来保存地址路径
        m_albumModel->insertRow(i, item); //插入model
        i++;
    }
}
//加载相册缩略图
void Core::loadAlbum(QStringList list, int start, int end)
{
    for (unsigned int i = start; i <= end; i++) {
        QString tmpFullPath = list.at(i - 1);
        if (m_loadedImage.contains(tmpFullPath)) {
            continue;
        }
        //存完整路径
        QFileInfo file(tmpFullPath);
        //判断有没有被外部删除
        if (file.exists()) {
            if (!m_loadedImage.contains(tmpFullPath)) {
                m_loadedImage.append(tmpFullPath);
            }
            QString nowRealFormat = File::realFormat(tmpFullPath);
            AlbumThumbnail *thread = new AlbumThumbnail(tmpFullPath, nowRealFormat);
            connect(thread, &AlbumThumbnail::finished, thread, &AlbumThumbnail::deleteLater);
            connect(thread, &AlbumThumbnail::albumFinish, this, &Core::albumLoadFinish);
            thread->start();
        } else {
            //触发删除-外部删除文件的情况，加载的时候需要删掉对应的item
            //            deleteAlbumItem(tmpFullPath);
        }
    }
}

void Core::loadImgCache(QModelIndex modelIndex)
{
    MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->itemFromIndex(modelIndex));
    if (item == nullptr) {
        qDebug() << "loadImgCache: album load fail";
        return;
    }

    int start = item->row() - Variable::LOADIMAGE_NUM / 2;
    int end = item->row() + Variable::LOADIMAGE_NUM / 2;
    if (start <= 0) {
        start = 1;
    }
    if (end > m_allImageList.length()) {
        end = m_allImageList.length();
    }

    QStringList pathList;
    for (unsigned int i = start; i <= end; i++) {
        QString tmpFullPath = m_allImageList.at(i - 1);
        pathList.append(tmpFullPath);
    }

    QHash<QString, QPair<MatAndFileinfo, QPixmap>>::iterator iter = m_loadedImageCache.begin();
    while (iter != m_loadedImageCache.end())
    {
        QString path = iter.key();
        if ((m_nowpath != path) && !pathList.contains(path))
        {
            QPair<MatAndFileinfo, QPixmap> matPix = iter.value();
            MatAndFileinfo maf = matPix.first;
            if (maf.matList != nullptr)
            {
                maf.matList->clear();
            }
            iter = m_loadedImageCache.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    for (unsigned int i = start; i <= end; i++) {
        QString tmpFullPath = m_allImageList.at(i - 1);
        if (m_loadedImageCache.contains(tmpFullPath)) {
            continue;
        }
        //存完整路径
        QFileInfo file(tmpFullPath);
        //判断有没有被外部删除
        if (file.exists()) {
            QString nowRealFormat = File::realFormat(tmpFullPath);
            AlbumRealImg *thread = new AlbumRealImg(tmpFullPath, nowRealFormat);
            connect(thread, &AlbumRealImg::finished, thread, &AlbumRealImg::deleteLater);
            connect(thread, &AlbumRealImg::albumRealFinish, this, &Core::albumLoadRealFinish);
            thread->start();
        } else {
            //触发删除-外部删除文件的情况，加载的时候需要删掉对应的item
            //            deleteAlbumItem(tmpFullPath);
        }
    }
}

//再加载图片
void Core::reloadImage(QModelIndex modelIndex)
{
    MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_albumModel->itemFromIndex(modelIndex));
    if (item == nullptr) {
        qDebug() << "reloadImage: album load fail";
        return;
    }

    int startItem = item->row() - Variable::LOADIMAGE_NUM / 2 - 1;
    int endItem = item->row() + Variable::LOADIMAGE_NUM / 2 + 4;
    if (startItem <= 0) {
        startItem = 1;
    }
    if (endItem > m_allImageList.length()) {
        endItem = m_allImageList.length();
//        if (endItem == 0)
//              return;
    }
    loadAlbum(m_allImageList, startItem, endItem);
}
// 获取图片中文字
void Core::getTextFromImage()
{
    m_ocr.setEngineType(EngineType::KySDK);
    Q_EMIT startOcr(m_nowpath);
}

void Core::qmlCutImageDisplay(QPointF startPos, QSizeF displaySize, double imageScale)
{
    m_isCutting = true;
    //边界修正
    startPos = qmlBoundaryPos(startPos, m_nowImage.width(), m_nowImage.height());
    displaySize = qmlBoundarySize(startPos, displaySize);
    //裁剪图片
    m_cutMat = m_nowMat(cv::Rect(startPos.x(), startPos.y(), displaySize.width(), displaySize.height())).clone();
    //缩放到显示的大小
    QPixmap pixmap = Processing::converFormat(m_cutMat);
    QSizeF tmp(pixmap.size());
    tmp.setWidth(tmp.width() * imageScale);
    tmp.setHeight(tmp.height() * imageScale);
    pixmap = pixmap.scaled(tmp.toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    //给图片添加蒙层
    QImage image = Processing::pictureCutDeepen(pixmap.toImage(), pixmap.size());
    //显示图片
    showImage(QPixmap::fromImage(image));
    m_origCutImageBefore = pixmap.toImage();
}

void Core::startSignApp()
{
    if (m_monitoredFiles != "") {
        m_fileWatcher->removePath(m_monitoredFiles);
    }
    if (m_matList == nullptr) {
        m_fileWatcher->addPath(m_nowpath);
        m_monitoredFiles = m_nowpath;
    }
    if (m_process->startDetached(Variable::SIGN_APP_NAME + QString("\"%1\"").arg(m_nowpath))) {
    } else {
        qDebug() << "调起画图失败";
    }
}

void Core::matToSamples()
{
#if !defined(__loongarch64) && !defined(__riscv)
    if (m_isSaveCutImage) {
        m_origMattingMat = m_mattingCutMat.clone();
    } else {
        m_origMattingMat = m_nowMat.clone();
    }

    if (m_origMattingMat.empty()) {
        qDebug() << "Failed to load image.";
        return;
    }
    // 检查图像是否为四通道,四通道转为三通道
    if (m_origMattingMat.channels() != 3) {
        cv::cvtColor(m_origMattingMat, m_origMattingMat, cv::COLOR_BGRA2BGR);
    }
    if (m_origMattingMat.channels() != 3) {
        qDebug() << "Please input RGB image.";
        return;
    }
    // 调整图像大小为模型期望的尺寸
    int inputWidth = 512;
    int inputHeight = 512;
    const std::vector<float> mean = {0.5, 0.5, 0.5};
    const std::vector<float> std = {0.5, 0.5, 0.5};
    cv::Mat image;
    cv::cvtColor(m_origMattingMat, image, cv::COLOR_BGR2RGB);
    image.convertTo(image, CV_32FC3, 1.0f / 255.0f);
    cv::Mat mean_mat(image.size(), CV_32FC3, cv::Scalar(mean[0], mean[1], mean[2]));
    cv::subtract(image, mean_mat, image);
    cv::Mat std_mat(image.size(), CV_32FC3, cv::Scalar(std[0], std[1], std[2]));

    cv::divide(image, std_mat, image);
    cv::resize(image, image, cv::Size(inputWidth, inputHeight), cv::INTER_LINEAR);
    image = cv::dnn::blobFromImage(image);
    // 创建ORT环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "Test");
    // 创建会话选项
    Ort::SessionOptions sessionOptions;
    // 加载模型
    const char* model_path = "/usr/share/lingmo-photo-viewer/model/lingmo_photo_viewer_matting_model.onnx";
    Ort::Session session(env, model_path, sessionOptions);
    // 准备输入数据
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    size_t input_tensor_size  = inputWidth * inputHeight * 3;
    std::vector<int64_t> input_shape = {1, 3, inputHeight, inputWidth};
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, (float*)image.data, input_tensor_size, input_shape.data(), 4);
    std::vector<const char*> input_names = {nullptr};
    std::vector<const char*> output_names = {nullptr};
    const char* input_name = "input";
    const char* output_name = "output";
    input_names[0] = input_name;
    output_names[0] = output_name;

    std::vector<float> output(1 * 1 * inputHeight * inputWidth);
    const std::vector<int64_t> outputShape = {1, 1, inputHeight, inputWidth};
    Ort::Value outputTensor = Ort::Value::CreateTensor<float>(
        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault),
        output.data(), output.size(), outputShape.data(), outputShape.size());

    Ort::RunOptions runOpts(nullptr);
    session.Run(runOpts, input_names.data(), &input_tensor, input_names.size(),
                output_names.data(), &outputTensor, output_names.size());

    float* output_data = outputTensor.GetTensorMutableData<float>();
    cv::Mat matte_np(inputHeight, inputWidth, CV_32F, output_data);

    // 调整灰度图大小与原始图像大小相匹配
    cv::Mat matte_resized;
    cv::resize(matte_np, matte_resized, cv::Size(m_origMattingMat.cols, m_origMattingMat.rows), 0, 0, cv::INTER_LINEAR);
    // 将抠图结果中背景部分设置为透明色
    matte_resized.convertTo(m_transparentMat, CV_8U, 255); // 将灰度图像转换为 8 位图像
#endif
}

void Core::picMatting(QImage pic)
{
    if (m_isSaveCutImage) {
        matToSamples();
        m_isSaveCutImage = false;
    }
    Mat bgImage(pic.height(), pic.width(), CV_8UC4, (void*)pic.bits(), pic.bytesPerLine());
    if (bgImage.empty()) {
        qDebug() << "无法读取背景图片!";
        return;
    }
    if (bgImage.size() != m_origMattingMat.size()) {
        cv::resize(bgImage, bgImage, m_origMattingMat.size());
    }
    Mat result(m_origMattingMat.size(), CV_8UC4);
    for (int y = 0; y < m_origMattingMat.rows; ++y) {
        for (int x = 0; x < m_origMattingMat.cols; ++x) {
            if (m_transparentMat.at<uchar>(y, x) >= 200) {
                cv::Vec3b fg = m_origMattingMat.at<cv::Vec3b>(y, x);
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(fg[0], fg[1], fg[2], 255); //前景
            } else {
                cv::Vec4b bg = bgImage.at<cv::Vec4b>(y, x);
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(bg[0], bg[1], bg[2], 255); //背景
            }
        }
    }
    m_replaceMat = result.clone();
    finishMatting(result);
}

void Core::replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter)
{
    if (isFirstEnter || m_isSaveCutImage) {
        matToSamples();
        m_isSaveCutImage = false;
    }
    Mat result(m_origMattingMat.size(), CV_8UC4);
    for (int y = 0; y < m_origMattingMat.rows; ++y) {
        for (int x = 0; x < m_origMattingMat.cols; ++x) {
            if (m_transparentMat.at<uchar>(y, x) >= 200) {
                cv::Vec3b bgr = m_origMattingMat.at<cv::Vec3b>(y, x);
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(bgr[0], bgr[1], bgr[2], 255); //前景
            } else {
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(b, g, r, a);  //背景
            }
        }
    }
    m_replaceMat = result.clone();
    finishMatting(result);
}

void Core::finishMatting(Mat mat)
{
    QImage img = (Processing::converFormat(mat)).toImage();
    Q_EMIT returnReplaceAndOrigImage(img, m_nowImage.toImage());
}

void Core::needSaveReplaceImage(QString path)
{
    QFileInfo file(path);
    if (File::saveImage(m_replaceMat, QString(path), file.suffix().toLower())) {
        openSaveImage();
    } else {
        qDebug() << "保存替换背景失败";
    }
}

void Core::mattingFlip(const Processing::FlipWay &way)
{
    if (m_isSaveCutImage) {
        m_mattingCutMat = Processing::processingImage(Processing::flip, m_mattingCutMat, QVariant(way));
        if (!m_mattingCutMat.data) {
            return;
        }
    } else {
        m_origMattingMat = Processing::processingImage(Processing::flip, m_origMattingMat, QVariant(way));
        if (!m_origMattingMat.data) {
            return;
        }
        m_transparentMat = Processing::processingImage(Processing::flip, m_transparentMat, QVariant(way));
        if (!m_transparentMat.data) {
            return;
        }
    }

    Mat mat;
    mat = Processing::processingImage(Processing::flip, m_replaceMat, QVariant(way));
    if (!mat.data) {
        return;
    }

    m_replaceMat = mat;
    QImage img = (Processing::converFormat(mat)).toImage();
    Q_EMIT returnFlipAndOrigImage(img, m_origMattingImage);
}

void Core::scanImage()
{
    Mat opMat = m_nowMat.clone();
    //灰度
    Mat grayMat = opMat.clone();
    if(opMat.channels() != 1){
        cvtColor(opMat, grayMat, COLOR_BGR2GRAY);
    }
    //双边滤波-降噪
    Mat filtMat;
    bilateralFilter(grayMat, filtMat, 10, 10, 10); //src和dst需不同
    //黑白二值图像
    Mat binaryMat;
    double maxValue = 255;
    cv::adaptiveThreshold(filtMat, binaryMat, maxValue,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C, //自动阈值计算
                          cv::THRESH_BINARY,
                          25,  // blockSize 必须是奇数
                          9);  // -C 用于微调阈值

    m_scanMat = binaryMat.clone();
    QImage scanImage = Processing::converFormat(m_scanMat).toImage();
    m_origScanImage = Processing::converFormat(m_nowMat).toImage();
    Q_EMIT returnScanAndOrigImage(scanImage, m_origScanImage);
}

void Core::needSaveScanImage(QString savePath)
{
    QFileInfo file(savePath);
    Mat saveMat;
    if(m_isWatermark){
        saveMat = m_watermarkMat.clone();
    }else{
        saveMat = m_scanMat.clone();
    }
    bool result = File::saveImage(saveMat, savePath, file.suffix());
    Q_EMIT returnScannerSaveResult(result);
    if (result) {
        openSaveImage();
    } else {
        qDebug() << "水印图片保存失败";
    }
}

void Core::printScanImage(QPrinter *printer)
{
    QImage printImage;
    if(m_isWatermark){
        printImage = Processing::converFormat(m_watermarkMat).toImage();
    }else{
        printImage = Processing::converFormat(m_scanMat).toImage();
    }
    QPainter painter(printer);
    QRect rect = painter.viewport();
    QSize size = printImage.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.setWindow(printImage.rect());
    painter.drawImage(0, 0, printImage);
    painter.end();
}

void Core::scannerSavePDF(QPrinter *printer)
{
    QImage printImage;
    if(m_isWatermark){
        printImage = Processing::converFormat(m_watermarkMat).toImage();
    }else{
        printImage = Processing::converFormat(m_scanMat).toImage();
    }

    QPainter painter(printer);
    QSizeF pageSize = printer->pageRect().size();   //打印机页面的尺寸
    QSizeF imageSize = printImage.size();           //原图的尺寸
    imageSize.scale(pageSize, Qt::KeepAspectRatio); //缩放
    QPointF startPoint((pageSize.width() - imageSize.width()) / 2.0, (pageSize.height() - imageSize.height()) / 2.0); //居中
    //绘制图像
    painter.drawImage(QRectF(startPoint, imageSize), printImage);
    painter.end();

    bool result;
    if (printer->printerState() == QPrinter::Error) {
        result = false;
        qDebug()<<"打印过程中发生错误";
    } else {
        result = true;
    }
    Q_EMIT returnScannerSaveResult(result);
}

void Core::scannerAddWatermark(bool isWatermark, QString vmContent)
{
    m_isWatermark = isWatermark;
    if(m_isWatermark){
        //准备绘制水印的图片、文本和设置
        QPixmap pix = Processing::converFormat(m_scanMat);
        QFont font("Noto Sans CJK SC",15);
        QColor textColor(128, 128, 128, 51); //半透明水印文字
        //计算绘制文本字符串的宽和高
        QFontMetricsF fontMetrics(font);
        qreal font_w = fontMetrics.width(vmContent);
        qreal font_h = fontMetrics.height();
        //旋转角度
        qreal ang = 30.0;
        double rad = ang * M_PI / 180;

        //开始绘图
        QPainter painter(&pix);
        painter.setFont(font);
        painter.setPen(textColor);
        painter.translate(pix.width() / 2, pix.height() / 2);  //平移原点
        painter.rotate(ang);
        int project_Y = abs(pix.width() * sin(rad)) + abs(pix.height() * cos(rad));//原图像Y坐标在新坐标系Y轴上的投影长度
        int project_X = abs(pix.height() * sin(rad)) + abs(pix.width() * cos(rad));//原图像x坐标在新坐标系x轴上的投影长度
        //水印间的距离
        int x_step = 2 * font_w;
        int y_step = 3 * font_h ;
        //水印的行数与列数
        int rowCount = project_Y / y_step + 1;
        int colCount = project_X / x_step + 2;//水印的行数  因为旋转了，如果不加2会导致水印缺少一块
        for (int r = 0; r < rowCount; r++){
            for (int c = 0; c < colCount; c++){
                painter.drawText(-project_X / 2 + x_step * c, -project_Y / 2 + y_step * r,vmContent);//写水印
            }
        }
        //转换为Mat
        QImage watermarkImage = pix.toImage();
        if(watermarkImage.format() == QImage::Format_RGB32){
            cv::Mat mat(watermarkImage.height(), watermarkImage.width(), CV_8UC4, watermarkImage.bits(), watermarkImage.bytesPerLine());
            m_watermarkMat = mat.clone();
        };
        //返回水印
        Q_EMIT returnScanAndOrigImage(watermarkImage, m_origScanImage);

    }else{
        QImage image = Processing::converFormat(m_scanMat).toImage();
        Q_EMIT returnScanAndOrigImage(image, m_origScanImage);
    }
}

void Core::scannerFlip(const ProcessingBase::FlipWay &way)
{
    //若添加水印后，扫描与水印状态同步
    Mat flipMat, mat;
    flipMat = m_scanMat.clone();
    mat = Processing::processingImage(Processing::flip, flipMat, QVariant(way));
    if (!mat.data) {
        return;
    }
    m_scanMat = mat;

    if(m_isWatermark){
        flipMat = m_watermarkMat.clone();
        mat = Processing::processingImage(Processing::flip, flipMat, QVariant(way));
        if (!mat.data) {
            return;
        }
        m_watermarkMat = mat;
    }

    if(m_isWatermark){
        QImage img = (Processing::converFormat(m_watermarkMat)).toImage();
        Q_EMIT returnScannerFlipAndOrigImage(img, m_origScanImage);
    }else{
        QImage img = (Processing::converFormat(m_scanMat)).toImage();
        Q_EMIT returnScannerFlipAndOrigImage(img, m_origScanImage);
    }
}

void Core::scannerSaveCutImage(int proportion, const QPoint &startP, const QPoint &endP)
{
    QPointF startPosition =
            QPointF(static_cast<double>(startP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                    static_cast<double>(startP.y()) / (static_cast<double>(proportion) * Variable::PERCENTAGE));
    QPointF endPosition =
            QPointF(static_cast<double>(endP.x()) / (static_cast<double>(proportion) * Variable::PERCENTAGE),
                    static_cast<double>(endP.y()) / (static_cast<double>(proportion) * Variable::PERCENTAGE));

    int imageWidth = endPosition.x() - startPosition.x();
    int imageHeight = endPosition.y() - startPosition.y();

    //水印与扫描图片同步
    if(m_isWatermark){
        CutRegion cutRegion = calcCutRegionSelection(startPosition, QSize(imageWidth, imageHeight),
                                                        Processing::converFormat(m_watermarkMat));
        m_watermarkMat = m_watermarkMat(cv::Rect(cutRegion.startPostion.x(), cutRegion.startPostion.y(),
                               cutRegion.width, cutRegion.height)).clone();

        QImage img = (Processing::converFormat(m_watermarkMat)).toImage();
        Q_EMIT returnScanAndOrigImage(img, m_nowImage.toImage());
    }
    else
    {
        //确定最终裁剪区域
        CutRegion cutRegion = calcCutRegionSelection(startPosition, QSize(imageWidth, imageHeight),
                                                        Processing::converFormat(m_scanMat));
        //裁剪图片
        m_scanMat = m_scanMat(cv::Rect(cutRegion.startPostion.x(), cutRegion.startPostion.y(),
                               cutRegion.width, cutRegion.height)).clone();

        QImage img = (Processing::converFormat(m_scanMat)).toImage();
        Q_EMIT returnScanAndOrigImage(img, m_nowImage.toImage());
    }
}

void Core::processLog()
{
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (error == "") {
        return;
    }
    qDebug() << "processLog: error:" << error;
}

void Core::fileChangedNeedUpdated()
{
    QFile file(m_monitoredFiles);
    if (file.exists()) {
        openImage(m_monitoredFiles);
    }
}

void Core::sendOcrResult(QString path, QVector<QString> result)
{
    if (path == m_nowpath) {
        Q_EMIT getTextResult(result);
    } else {
        qWarning() << "Image paths do not match,get recognition result failed!";
    }
}

QPointF Core::qmlBoundaryPos(QPointF pointf, int xMax, int yMax)
{
    if (pointf.x() > xMax) {
        pointf.setX(xMax);
    }
    if (pointf.y() > yMax) {
        pointf.setY(yMax);
    }
    if (pointf.x() < 0) {
        pointf.setX(0);
    }
    if (pointf.y() < 0) {
        pointf.setY(0);
    }
    return pointf;
}

QSizeF Core::qmlBoundarySize(QPointF pointf, QSizeF displaySize)
{
    QSizeF tmpSize = displaySize;
    if (m_nowImage.width() - pointf.x() - tmpSize.width() < 0) {
        tmpSize.setWidth(m_nowImage.width() - pointf.x());
    }
    if (m_nowImage.height() - pointf.y() - tmpSize.height() < 0) {
        tmpSize.setHeight(m_nowImage.height() - pointf.y());
    }
    return tmpSize;
}

bool Core::coreOperateTooOften()
{
    if (m_canProcess->isActive()) {
        return true;
    }
    m_canProcess->start(Variable::REFRESH_RATE); //刷新间隔
    return false;
}

void MyStandardItem::setPath(const QString &path)
{
    m_path = path;
}

QString MyStandardItem::getPath()
{
    return m_path;
}
