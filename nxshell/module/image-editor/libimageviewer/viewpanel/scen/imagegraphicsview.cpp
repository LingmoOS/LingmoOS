// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagegraphicsview.h"
#include "unionimage/pluginbaseutils.h"

#include <QDebug>
#include <QFile>
#include <QOpenGLWidget>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMovie>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QPaintEvent>
#include <QtConcurrent>
#include <QHBoxLayout>
#include <qmath.h>
#include <QScrollBar>
#include <QGestureEvent>
#include <QSvgRenderer>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QShortcut>
#include <QApplication>
#include <DSpinner>

#include "graphicsitem.h"
#include "imagesvgitem.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "accessibility/ac-desktop-define.h"
#include "../contents/morepicfloatwidget.h"
#include "imageengine.h"
#include "service/mtpfileproxy.h"
#include "service/aimodelservice.h"
#include "service/permissionconfig.h"

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifndef QT_NO_OPENGL
//#include <QGLWidget>
#endif

#include <sys/inotify.h>
#include "service/commonservice.h"

DWIDGET_USE_NAMESPACE


namespace {

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");
const QSize SPINNER_SIZE = QSize(40, 40);
#ifndef tablet_PC
const qreal MAX_SCALE_FACTOR = 20.0;
const qreal MIN_SCALE_FACTOR = 0.02;
#else
const qreal MAX_SCALE_FACTOR = 2.0;
qreal MIN_SCALE_FACTOR = 0.0;
#endif

QVariantList cachePixmap(const QString &path)
{
    QImage tImg;
    QString errMsg;
    QSize size;
    Q_UNUSED(size);
//    UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, size, errMsg);
    LibUnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg);
    QPixmap p = QPixmap::fromImage(tImg);
    if (QFileInfo(path).exists() && p.isNull()) {
        //判定为损坏图片
//        p = utils::image::getDamagePixmap(DApplicationHelper::instance()->themeType() == DApplicationHelper::LightType);
        if(path.contains("ftp:host=")) {//前面失败后再处理ftp图片
            QFileInfo info(path);
            if(info.size() <=  1024*1024*1024) {//小于1Mftp文件做下载本地处理
                QNetworkAccessManager manager;
                QEventLoop loop;

                QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply *reply){
                    QByteArray imageData = reply->readAll();
                    tImg.loadFromData(imageData);
                    p = QPixmap::fromImage(tImg);
                    loop.quit();
                });
                int nIdex = path.indexOf("ftp:host=");
                QString urlAddr = path.mid(nIdex).replace("ftp:host=", "ftp://");
                QNetworkRequest request(urlAddr);
                manager.get(request);
                loop.exec();
            }
        }
        qDebug() << errMsg;
    }
    QVariantList vl;
    vl << QVariant(path) << QVariant(p);
    return vl;
}

}  // namespace
LibImageGraphicsView::LibImageGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
    , m_pool(new QThreadPool(this))
//    , m_svgItem(nullptr)
    , m_movieItem(nullptr)
    , m_pixmapItem(nullptr)
{
    this->setObjectName("ImageView");
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    setScene(new QGraphicsScene(this));
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);
    setAcceptDrops(false);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::Shape::NoFrame);

    viewport()->setCursor(Qt::ArrowCursor);

    //鼠标事件,以及不在通过键盘移动视图
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);

    connect(&m_watcher, &QFutureWatcherBase::finished, this, &LibImageGraphicsView::onCacheFinish);
//    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this, &ImageView::onThemeChanged);
    m_pool->setMaxThreadCount(1);
    m_loadTimer = new QTimer(this);
    m_loadTimer->setSingleShot(true);
    m_loadTimer->setInterval(300);

    connect(m_loadTimer, &QTimer::timeout, this, &LibImageGraphicsView::onLoadTimerTimeout);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &LibImageGraphicsView::onThemeTypeChanged);
    //初始化主题
    onThemeTypeChanged();
    m_imgFileWatcher = new QFileSystemWatcher(this);
    connect(m_imgFileWatcher, &QFileSystemWatcher::fileChanged, this, &LibImageGraphicsView::onImgFileChanged);
    m_isChangedTimer = new QTimer(this);
    QObject::connect(m_isChangedTimer, &QTimer::timeout, this, &LibImageGraphicsView::onIsChangedTimerTimeout);

    // MTP文件加载完成通知, 使用 QueuedConnection 确保不在 setImage 时触发，保证先后顺序。
    QObject::connect(MtpFileProxy::instance(), &MtpFileProxy::createProxyFileFinished, this, [ this ](const QString &proxyFile, bool){
        if (proxyFile == m_loadPath) {
            // SVG/TIF/GIF 需重新加载
            imageViewerSpace::ImageType Type = LibUnionImage_NameSpace::getImageType(m_loadPath);
            if (imageViewerSpace::ImageTypeDynamic == Type
                    || imageViewerSpace::ImageTypeSvg == Type
                    || imageViewerSpace::ImageTypeMulti == Type) {
                setImage(m_loadPath);
            } else {
                this->onLoadTimerTimeout();
            }

            m_imgFileWatcher->addPath(m_path);
            m_imgFileWatcher->addPath(MtpFileProxy::instance()->mapToOriginFile(m_path));
        }
    }, Qt::QueuedConnection);

    //让默认的快捷键失效，默认会滑动窗口
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this), &QShortcut::activated, this, &LibImageGraphicsView::slotsUp);
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this), &QShortcut::activated, this, &LibImageGraphicsView::slotsDown);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this);

    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Down), this);

    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down), this);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down), this);

    new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Down), this);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Down), this);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_Left), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_Right), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_Up), this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_Down), this);
}

int LibImageGraphicsView::getcurrentImgCount()
{
    int ret = 0;
    if (m_imageReader) {
        ret = m_imageReader->imageCount();
    }
    return ret;
}

void LibImageGraphicsView::setWindowIsFullScreen(bool bRet)
{
    if (bRet) {
        TITLEBAR_HEIGHT = 0;
    } else {
        TITLEBAR_HEIGHT = 50;
    }
}

LibImageGraphicsView::~LibImageGraphicsView()
{
    if (m_imgFileWatcher) {
//        m_imgFileWatcher->clear();
//        m_imgFileWatcher->quit();
//        m_imgFileWatcher->terminate();
//        m_imgFileWatcher->wait();
        m_imgFileWatcher->deleteLater();
    }
    if (m_imageReader) {
        delete m_imageReader;
        m_imageReader = nullptr;
    }
    if (m_morePicFloatWidget) {
        m_morePicFloatWidget->deleteLater();
        m_morePicFloatWidget = nullptr;
    }
    if (m_movieItem) {
        delete m_movieItem;
        m_movieItem = nullptr;
    }
    if (m_pixmapItem) {
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    if (m_imgSvgItem) {
        delete m_imgSvgItem;
        m_imgSvgItem = nullptr;
    }
    //保存旋转状态
    slotRotatePixCurrent();
}

void LibImageGraphicsView::clear()
{
    if (m_pixmapItem != nullptr) {
        delete m_pixmapItem;
        m_pixmapItem = nullptr;
    }
    m_movieItem = nullptr;
    scene()->clear();
}

void LibImageGraphicsView::setImage(const QString &path, const QImage &image)
{
    // m_spinner 生命周期由 scene() 管理
    hideSpinner();

    //默认多页图的按钮显示为false
    if (m_morePicFloatWidget) {
        m_morePicFloatWidget->setVisible(false);
        m_morePicFloatWidget->getButtonDown()->clearFocus();
    }
    if (m_imageReader) {
        delete m_imageReader;
        m_imageReader = nullptr;
    }

    // 判断是否需要等待 MTP 代理文件加载完成，失败同样无需等待加载
    MtpFileProxy::FileState state = MtpFileProxy::instance()->state(path);
    bool needProxyLoad = MtpFileProxy::instance()->contains(path) && (MtpFileProxy::Loading == state);

    bool delayLoad = needProxyLoad;
    // 判断是否为AI模型处理图片，需要延迟加载
    auto enhanceState = AIModelService::instance()->enhanceState(path);
    bool imageEnhance = (AIModelService::None != enhanceState);
    delayLoad |= (AIModelService::Loading == enhanceState);
    QString sourceFile = AIModelService::instance()->sourceFilePath(path);

    //检测数据缓存,如果存在,则使用缓存
    imageViewerSpace::ItemInfo info;
    if (needProxyLoad) {
        // 不获取数据
    } else if (imageEnhance) {
        info = LibCommonService::instance()->getImgInfoByPath(sourceFile);
    } else {
        info = LibCommonService::instance()->getImgInfoByPath(path);
    }

    m_bRoate = ImageEngine::instance()->isRotatable(path); //是否可旋转

    m_loadPath = path;
    // Empty path will cause crash in release-build mode
    if (path.isEmpty()) {
        return;
    }

    // 同时移除 MTP 代理文件的观察
    m_imgFileWatcher->removePath(MtpFileProxy::instance()->mapToOriginFile(m_path));
    m_imgFileWatcher->removePath(m_path);
    if (imageEnhance) {
        // 获取之前的原文件
        m_imgFileWatcher->removePath(AIModelService::instance()->sourceFilePath(m_path));
    }

    m_path = path;
    if (!needProxyLoad) {
        // MTP 代理文件等待创建完成后再添加观察
        m_imgFileWatcher->addPath(m_path);
    }
    // 保留原始文件
    if (imageEnhance) {
        m_imgFileWatcher->addPath(sourceFile);
    }

    QString strfixL = QFileInfo(path).suffix().toUpper();
    QGraphicsScene *s = scene();
    QFileInfo fi(path);
    QStringList fList = LibUnionImage_NameSpace::supportMovieFormat(); //"gif","mng"
    //QMovie can't read frameCount of "mng" correctly,so change
    //the judge way to solve the problem

    imageViewerSpace::ImageType Type = info.imageType;
    if (delayLoad) {
        // 延迟加载，MTP代理文件默认为空，后续加载处理
        Type = imageViewerSpace::ImageTypeBlank;
    } else if (Type == imageViewerSpace::ImageTypeBlank) {
        Type = LibUnionImage_NameSpace::getImageType(path);
    }

    //ImageTypeDynamic
    if (Type == imageViewerSpace::ImageTypeDynamic) {
        m_pixmapItem = nullptr;
        m_movieItem = nullptr;
        m_imgSvgItem = nullptr;
        s->clear();
        resetTransform();
        m_movieItem = new LibGraphicsMovieItem(path, strfixL);
        //        m_movieItem->start();
        // Make sure item show in center of view after reload
        setSceneRect(m_movieItem->boundingRect());
        qDebug() << "m_movieItem->boundingRect() = " << m_movieItem->boundingRect();
        s->addItem(m_movieItem);
        emit imageChanged(path);
        QMetaObject::invokeMethod(this, [ = ]() {
            resetTransform();
            autoFit();
        }, Qt::QueuedConnection);
        m_newImageLoadPhase = FullFinish;
    } else if (Type == imageViewerSpace::ImageTypeSvg) {
        m_pixmapItem = nullptr;
        m_movieItem = nullptr;
        m_imgSvgItem = nullptr;
        s->clear();
        resetTransform();

        if (!m_svgRenderer) {
            m_svgRenderer = new QSvgRenderer(this);
        }
        m_svgRenderer->load(path);
        m_imgSvgItem = new LibImageSvgItem();
        m_imgSvgItem->setSharedRenderer(m_svgRenderer);
        //不会出现锯齿
        m_imgSvgItem->setCacheMode(QGraphicsItem::NoCache);
        setSceneRect(m_imgSvgItem->boundingRect());
        s->addItem(m_imgSvgItem);
        emit imageChanged(path);
        QMetaObject::invokeMethod(this, [ = ]() {
            resetTransform();
            autoFit();
        }, Qt::QueuedConnection);
        m_newImageLoadPhase = FullFinish;
    } else {
        QPixmap previousPix;
        if (imageEnhance && m_pixmapItem) {
            previousPix = m_pixmapItem->pixmap();
        }

        //当传入的image无效时，需要重新读取数据
        m_pixmapItem = nullptr;
        m_movieItem = nullptr;
        m_imgSvgItem = nullptr;
        scene()->clear();
        resetTransform();

        if (image.isNull()) {
            QPixmap pix ;
            if (!info.image.isNull()) {
                // 获取用于模糊的图片
                pix = getBlurPixmap(path, info, previousPix);
            }

            m_pixmapItem = new LibGraphicsPixmapItem(pix);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);

            if (delayLoad && imageEnhance) {
                // 图像增强使用 60% 透明度蒙版效果,不同主题，白色/黑色
                LibGraphicsMaskItem *maskItem = new LibGraphicsMaskItem(m_pixmapItem);
                maskItem->setRect(m_pixmapItem->boundingRect());
            } else {
                // 设置加载图片模糊效果
                // Make sure item show in center of view after reload
                if (!m_blurEffect) {
                    m_blurEffect = new QGraphicsBlurEffect(this);
                    m_blurEffect->setBlurRadius(5);
                    m_blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
                }
                m_pixmapItem->setGraphicsEffect(m_blurEffect);
            }

            //如果缩略图不为空,则区域变为m_pixmapItem
            if (!pix.isNull()) {
                setSceneRect(m_pixmapItem->boundingRect());
            }

            // 使用 MTP 代理文件，需等待代理文件创建完成 createProxyFileFinished() ，
            // 或其他AI模型处理等延迟处理，完成后调用 onLoadTimerTimeout()
            //第一次打开直接启动,不使用延时300ms
            if (!delayLoad) {
                if (m_isFistOpen) {
                    onLoadTimerTimeout();
                    m_isFistOpen = false;
                } else {
                    m_loadTimer->start();
                }
            }

            scene()->addItem(m_pixmapItem);

            // 没有可用的图片，设置选转加载图标
            // AI图像增强时，允许同时存在
            if (pix.isNull() || delayLoad) {
                addLoadSpinner(imageEnhance);
            }

            if (!imageEnhance) {
                emit imageChanged(path);
            }
            QMetaObject::invokeMethod(this, [ = ]() {
                resetTransform();
            }, Qt::QueuedConnection);
            m_newImageLoadPhase = ThumbnailFinish;
        } else {
            //当传入的image有效时，直接刷入图像，不再重复读取
            QPixmap pix = QPixmap::fromImage(image);
            pix.setDevicePixelRatio(devicePixelRatioF());
            m_pixmapItem = new LibGraphicsPixmapItem(pix);
            m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
            setSceneRect(m_pixmapItem->boundingRect());
            scene()->clear();
            scene()->addItem(m_pixmapItem);
            autoFit();
            emit imageChanged(path);
            this->update();
            emit hideNavigation();
            m_newImageLoadPhase = FullFinish;
        }

        if (Type == imageViewerSpace::ImageTypeMulti) {
            if (!m_morePicFloatWidget) {
                initMorePicWidget();
            }

            m_imageReader = new QImageReader(path);
            if (m_imageReader->imageCount() > 1) {
                //校验tiff的第一个图
//                m_imageReader->jumpToImage(0);
//                auto tifImg = m_imageReader->read();
//                if (tifImg.isNull()) {
//                    m_morePicFloatWidget->setVisible(false);
//                } else {
                //以免出现焦点在down的按钮下
                m_morePicFloatWidget->setFocus();
                m_morePicFloatWidget->setVisible(true);
                if (m_morePicFloatWidget->getButtonUp()) {
                    m_morePicFloatWidget->getButtonUp()->setEnabled(false);
                }
                if (m_morePicFloatWidget->getButtonDown()) {
                    m_morePicFloatWidget->getButtonDown()->setEnabled(true);
                }
                m_currentMoreImageNum = 0;
//                }
            } else {
                m_morePicFloatWidget->setVisible(false);
            }
            m_morePicFloatWidget->setLabelText(QString::number(m_imageReader->currentImageNumber() + 1) + "/" + QString::number(m_imageReader->imageCount()));
            //由于最小化窗口尺寸过小，会遮盖掉切换栏，抬高30px
            m_morePicFloatWidget->move(this->width() - 80, this->height() / 2 -80);
        }
    }
    m_firstset = true;
}

void LibImageGraphicsView::setScaleValue(qreal v)
{
    //预先计算需要的缩放比
    double temp = m_scal * v;
    double scaleFactor = -1.0;
    if (v < 1 && temp <= MIN_SCALE_FACTOR) {
        scaleFactor = MIN_SCALE_FACTOR / m_scal;
    } else if (v > 1 && temp >= MAX_SCALE_FACTOR) {
        scaleFactor = MAX_SCALE_FACTOR / m_scal;
    } else {
        scaleFactor = v;
        m_isFitImage = false;
        m_isFitWindow = false;
    }

    //执行缩放
    m_scal *= scaleFactor;
    scale(scaleFactor, scaleFactor);
    qDebug() << m_scal;

    //1:1高亮按钮
    if (m_scal - 1 > -0.01 && m_scal - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }

    //缩放通知信号
    emit scaled(m_scal * 100);
    emit showScaleLabel();
    emit transformChanged();
    titleBarControl();
}

void LibImageGraphicsView::autoFit()
{
    //确认场景加载出来后，才能调用场景内的item
//    if (!scene()->isActive())
//        return;
    if (image().isNull())
        return;
    QSize image_size = image().size();
    if ((image_size.width() >= width() ||
            image_size.height() >= height() - TITLEBAR_HEIGHT * 2) &&
            width() > 0 && height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }
}

const QImage LibImageGraphicsView::image()
{
    QImage img;
    if (m_movieItem) {           // bit-map
        img = m_movieItem->pixmap().toImage();
    } else if (m_pixmapItem) {
        //FIXME: access to m_pixmapItem will crash
        if (nullptr == m_pixmapItem) {  //add to slove crash by shui
            img = QImage();
        } else {
            img = m_pixmapItem->pixmap().toImage();
        }

    } else if (m_imgSvgItem) { // 新增svg的image
        QImage image(m_imgSvgItem->renderer()->defaultSize(), QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor(0, 0, 0, 0));
        QPainter imagePainter(&image);
        m_imgSvgItem->renderer()->render(&imagePainter);
        imagePainter.end();
        img = image;
    } else {
        img = QImage();
    }
    if (img.isNull() && m_morePicFloatWidget) {
        m_morePicFloatWidget->setVisible(false);
    } else if (m_imageReader && (m_imageReader->imageCount() > 1) && m_morePicFloatWidget) {
        m_morePicFloatWidget->setVisible(true);
    }
    return img;
}

void LibImageGraphicsView::fitWindow()
{
    qreal wrs = windowRelativeScale();
    qDebug()<<wrs;
    resetTransform();
    //可能存在比例超过20.0的情况，设置为20.0
    if (wrs > 20.0) {
        wrs = 20.0;
    }
    m_scal = wrs; //注意，这个东西的初始化要和附近的scale函数同步
    scale(wrs, wrs);
    emit checkAdaptScreenBtn();
    if (wrs - 1 > -0.01 &&
            wrs - 1 < 0.01) {
        emit checkAdaptImageBtn();
    } else {
        emit disCheckAdaptImageBtn();
    }
    m_isFitImage = false;
    m_isFitWindow = true;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();

    titleBarControl();
}

void LibImageGraphicsView::fitImage()
{
    qreal wrs = windowRelativeScale();
    resetTransform();
    scale(1, 1);
    m_scal = 1; //注意，这个东西的初始化要和附近的scale函数同步
    emit checkAdaptImageBtn();
    if (wrs - 1 > -0.01 &&
            wrs - 1 < 0.01) {
        emit checkAdaptScreenBtn();
    } else {
        emit disCheckAdaptScreenBtn();
    }
    m_isFitImage = true;
    m_isFitWindow = false;
    scaled(imageRelativeScale() * 100);
    emit transformChanged();

    titleBarControl();
}

void LibImageGraphicsView::rotateClockWise()
{
    QString errMsg;
    QImage rotateResult;
    if (!LibUnionImage_NameSpace::rotateImageFIleWithImage(90, rotateResult, m_path, errMsg)) {
        qDebug() << errMsg;
        return;
    }
//    dApp->m_imageloader->updateImageLoader(QStringList(m_path), QList<QImage>({rotateResult}));
    setImage(m_path, rotateResult);
}

void LibImageGraphicsView::rotateCounterclockwise()
{
    QString errMsg;
    QImage rotateResult;
    if (!LibUnionImage_NameSpace::rotateImageFIleWithImage(-90, rotateResult, m_path, errMsg)) {
        qDebug() << errMsg;
        return;
    }
//    dApp->m_imageloader->updateImageLoader(QStringList(m_path), QList<QImage>({rotateResult}));
    setImage(m_path, rotateResult);
}

void LibImageGraphicsView::centerOn(qreal x, qreal y)
{
    QGraphicsView::centerOn(x, y);
    emit transformChanged();
}


qreal LibImageGraphicsView::imageRelativeScale() const
{
    // vertical scale factor are equal to the horizontal one
    return transform().m11();
}

qreal LibImageGraphicsView::windowRelativeScale() const
{
    QRectF bf = sceneRect();

    //新需求,默认适应窗口顶格到标题栏
    if (1.0 * width() / (height() - TITLEBAR_HEIGHT * 2) > 1.0 * bf.width() / (bf.height())) {
//    if (1.0 * width() / (height() - TITLEBAR_HEIGHT * 2) > 1.0 * bf.width() / (bf.height() - TITLEBAR_HEIGHT * 2)) {
        return 1.0 * (height() - TITLEBAR_HEIGHT * 2) / bf.height();
    } else {
        return 1.0 * width() / bf.width();
    }
}

const QString LibImageGraphicsView::path() const
{
    return m_path;
}

QPoint LibImageGraphicsView::mapToImage(const QPoint &p) const
{
    return viewportTransform().inverted().map(p);
}

QRect LibImageGraphicsView::mapToImage(const QRect &r) const
{
    return viewportTransform().inverted().mapRect(r);
}

QRect LibImageGraphicsView::visibleImageRect() const
{
    return mapToImage(rect()) & QRect(0, 0, static_cast<int>(sceneRect().width()), static_cast<int>(sceneRect().height()));
}

bool LibImageGraphicsView::isWholeImageVisible() const
{
    const QRect &r = visibleImageRect();
    const QRectF &sr = sceneRect();

    //社区版qt5.15.1存在偏差0-1
    return r.width() >= (sr.width() - 1) && r.height() >= (sr.height() - 1);
}

bool LibImageGraphicsView::isFitImage() const
{
    return m_isFitImage;
}

bool LibImageGraphicsView::isFitWindow() const
{
    return m_isFitWindow;
}

void LibImageGraphicsView::initMorePicWidget()
{
    m_morePicFloatWidget = new MorePicFloatWidget(this);
    m_morePicFloatWidget->initUI();

    connect(m_morePicFloatWidget->getButtonUp(), &DIconButton::clicked, this, &LibImageGraphicsView::slotsUp);
    connect(m_morePicFloatWidget->getButtonDown(), &DIconButton::clicked, this, &LibImageGraphicsView::slotsDown);

    m_morePicFloatWidget->setFixedWidth(70);
    m_morePicFloatWidget->setFixedHeight(140);
    m_morePicFloatWidget->show();
}

void LibImageGraphicsView::titleBarControl()
{
    qreal realHeight = 0.0;
    //简化image()的使用
    QImage img = image();
//    if (m_movieItem /*|| m_imgSvgItem*/) {
//        realHeight = img.size().height() * imageRelativeScale() * devicePixelRatioF();

//    } else {
    realHeight = img.size().height() * imageRelativeScale() / devicePixelRatioF();
//    }

    if (realHeight > height() - TITLEBAR_HEIGHT * 2 + 1) {
        emit sigImageOutTitleBar(true);
    } else {
        emit sigImageOutTitleBar(false);
    }
}

void LibImageGraphicsView::slotSavePic()
{
    //保存旋转的图片
    slotRotatePixCurrent();
}

void LibImageGraphicsView::onImgFileChanged(const QString &ddfFile)
{
    // 判断是否为MTP原始路径文件，若为则同步更新代理文件状态
    MtpFileProxy::instance()->triggerOriginFileChanged(ddfFile);

    m_isChangedTimer->start(200);

    // 判断是否为当前处理图片
    if (AIModelService::instance()->isValid()) {
        QString lastProcImage = AIModelService::instance()->lastProcOutput();
        QString lastSource = AIModelService::instance()->sourceFilePath(lastProcImage);
        if (lastSource == ddfFile) {
            AIModelService::instance()->cancelProcess(lastProcImage);
        }
    }
}

void LibImageGraphicsView::onLoadTimerTimeout()
{
    QFuture<QVariantList> f = QtConcurrent::run(m_pool, cachePixmap, m_loadPath);
    if (m_watcher.isRunning()) {
        m_watcher.cancel();
        m_watcher.waitForFinished();
    }
    m_watcher.setFuture(f);
    emit hideNavigation();

    //重新刷新缓存
    //ImageEngine::instance()->makeImgThumbnail(LibCommonService::instance()->getImgSavePath(), QStringList(m_loadPath), 1, true);
}

void LibImageGraphicsView::onThemeTypeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_backgroundColor = Libutils::common::DARK_BACKGROUND_COLOR;
    } else {
        m_backgroundColor = Libutils::common::LIGHT_BACKGROUND_COLOR;
    }
    update();
}

void LibImageGraphicsView::onIsChangedTimerTimeout()
{
    QFileInfo file(m_path);
    if (file.exists()) {
//        dApp->m_imageloader->updateImageLoader(QStringList(m_path));
        setImage(m_path);
    }
    //刷新文件信息
    emit sigFIleDelete();
    m_isChangedTimer->stop();
}

void LibImageGraphicsView::slotsUp()
{
    if (m_morePicFloatWidget) {
        if (m_morePicFloatWidget->getButtonUp()) {
            m_morePicFloatWidget->getButtonUp()->setEnabled(true);
        }
        if (m_morePicFloatWidget->getButtonDown()) {
            m_morePicFloatWidget->getButtonDown()->setEnabled(true);
        }

        if (m_pixmapItem && m_imageReader && m_imageReader->imageCount() > 1) {
            if ((0 == m_imageReader->currentImageNumber()) && (0 == m_currentMoreImageNum)) {
                //改为与现在相同按钮点击逻辑相同修改bug62227，第一张图片时候，向上按钮置灰
                m_morePicFloatWidget->getButtonUp()->setEnabled(false);
                m_currentMoreImageNum = 0;
            } else if ((1 == m_imageReader->currentImageNumber()) || (1 == m_currentMoreImageNum)) {
                m_imageReader->jumpToImage(0);
                m_currentMoreImageNum = 0;
                if (m_morePicFloatWidget->getButtonUp()) {
                    m_morePicFloatWidget->getButtonUp()->setEnabled(false);
                }

            } else {
                m_currentMoreImageNum--;
                if (0 == m_imageReader->currentImageNumber()) {
                    m_imageReader->jumpToImage(m_currentMoreImageNum);
                } else {
                    m_imageReader->jumpToImage(m_imageReader->currentImageNumber() - 1);
                }
            }
            m_pixmapItem = nullptr;
            m_pixmapItem = nullptr;
            m_imgSvgItem = nullptr;
            scene()->clear();

            resetTransform();
            QPixmap pixmap = QPixmap::fromImage(m_imageReader->read());
            pixmap.setDevicePixelRatio(devicePixelRatioF());
            m_pixmapItem = new LibGraphicsPixmapItem(pixmap);
            scene()->addItem(m_pixmapItem);
            QRectF rect = m_pixmapItem->boundingRect();
            setSceneRect(rect);
            autoFit();
            if (m_currentMoreImageNum != m_imageReader->currentImageNumber()) {
                m_morePicFloatWidget->setLabelText(QString::number(m_currentMoreImageNum + 1) + "/" + QString::number(m_imageReader->imageCount()));
            } else {
                m_morePicFloatWidget->setLabelText(QString::number(m_imageReader->currentImageNumber() + 1) + "/" + QString::number(m_imageReader->imageCount()));
            }
            //todo ,更新导航栏
            emit UpdateNavImg();
        }
    }
}

void LibImageGraphicsView::slotsDown()
{
    if (m_morePicFloatWidget) {
        if (m_morePicFloatWidget->getButtonUp()) {
            m_morePicFloatWidget->getButtonUp()->setEnabled(true);
        }
        if (m_morePicFloatWidget->getButtonDown()) {
            m_morePicFloatWidget->getButtonDown()->setEnabled(true);
        }

        if (m_pixmapItem && m_imageReader && m_imageReader->imageCount() > 1) {
            if ((m_imageReader->currentImageNumber() == m_imageReader->imageCount() - 1) || m_currentMoreImageNum == m_imageReader->imageCount() - 1) {
                //改为与现在相同按钮点击逻辑相同修改bug62227，最后一张图片时候，向下按钮置灰
                m_morePicFloatWidget->getButtonDown()->setEnabled(false);
                m_currentMoreImageNum = m_imageReader->imageCount() - 1;
            } else if ((m_imageReader->currentImageNumber() == m_imageReader->imageCount() - 2) || (m_currentMoreImageNum == m_imageReader->imageCount() - 2)) {
                m_imageReader->jumpToImage(m_imageReader->imageCount() - 1);
                m_currentMoreImageNum = m_imageReader->imageCount() - 1;
                if (m_morePicFloatWidget->getButtonDown()) {
                    m_morePicFloatWidget->getButtonDown()->setEnabled(false);
                }

            } else {
                m_imageReader->jumpToNextImage();
                m_currentMoreImageNum++;
            }
            //修复bug69273,缩放存在问题
            m_pixmapItem = nullptr;
            m_pixmapItem = nullptr;
            m_imgSvgItem = nullptr;
            scene()->clear();
            resetTransform();
            QPixmap pixmap = QPixmap::fromImage(m_imageReader->read());
            pixmap.setDevicePixelRatio(devicePixelRatioF());
            m_pixmapItem = new LibGraphicsPixmapItem(pixmap);
            scene()->addItem(m_pixmapItem);
            QRectF rect = m_pixmapItem->boundingRect();
            setSceneRect(rect);
            autoFit();
            if (m_currentMoreImageNum != m_imageReader->currentImageNumber()) {
                m_morePicFloatWidget->setLabelText(QString::number(m_currentMoreImageNum + 1) + "/" + QString::number(m_imageReader->imageCount()));
            } else {
                m_morePicFloatWidget->setLabelText(QString::number(m_imageReader->currentImageNumber() + 1) + "/" + QString::number(m_imageReader->imageCount()));
            }

            //todo ,更新导航栏
            emit UpdateNavImg();
        }
    }
}

bool LibImageGraphicsView::slotRotatePixmap(int nAngel)
{
    if (!m_pixmapItem) return false;
    QPixmap pixmap = m_pixmapItem->pixmap();
    QTransform rotate;
    rotate.rotate(nAngel);

    pixmap = pixmap.transformed(rotate, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    scene()->clear();
    resetTransform();
    m_pixmapItem = new LibGraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();
    //            rect.setHeight(rect.height() + 50);
    setSceneRect(rect);
    //            setSceneRect(m_pixmapItem->boundingRect());
    scene()->addItem(m_pixmapItem);

    autoFit();
    m_rotateAngel += nAngel;

    QPixmap thumbnailPixmap;
    if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
        bool cache_exist = false;
        if (pixmap.height() != 200 && pixmap.width() != 200) {
            if (pixmap.height() >= pixmap.width()) {
                cache_exist = true;
                thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
            } else if (pixmap.height() <= pixmap.width()) {
                cache_exist = true;
                thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
            }
        }
        if (!cache_exist) {
            if (static_cast<float>(pixmap.height()) / static_cast<float>(pixmap.width()) > 3) {
                thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
            } else {
                thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
            }
        }
    }
    emit currentThumbnailChanged(thumbnailPixmap, pixmap.size());
    emit imageChanged(m_path);
    return true;
}

void LibImageGraphicsView::slotRotatePixCurrent()
{
    m_rotateAngel =  m_rotateAngel % 360;
    if (0 != m_rotateAngel) {
        //20211019修改：特殊位置不执行写入操作
        imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(m_path);

        if (pathType != imageViewerSpace::PathTypeMTP && pathType != imageViewerSpace::PathTypePTP && //安卓手机
                pathType != imageViewerSpace::PathTypeAPPLE && //苹果手机
                pathType != imageViewerSpace::PathTypeSAFEBOX && //保险箱
                pathType != imageViewerSpace::PathTypeRECYCLEBIN) { //回收站

            // 图片信息不一定同步，需做二次判断，根据权限配置是否回写文件
            if (!PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableEdit, m_path)) {
                m_rotateAngel = 0;
                return;
            }

            disconnect(m_imgFileWatcher, &QFileSystemWatcher::fileChanged, this, &LibImageGraphicsView::onImgFileChanged);
            Libutils::image::rotate(m_path, m_rotateAngel);

            // 如果是 MTP 文件，则将缓存文件的变更提交到 MTP 目录下
            MtpFileProxy::instance()->submitChangesToMTP(m_path);

            //如果是相册调用，则告知刷新
            if (LibCommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeAlbum) {
                emit ImageEngine::instance()->sigRotatePic(m_path);
            }

            QTimer::singleShot(1000, this, [ = ] {
                connect(m_imgFileWatcher, &QFileSystemWatcher::fileChanged, this, &LibImageGraphicsView::onImgFileChanged);
            });
            m_rotateAngel = 0;

            // 通知授权控制图片编辑完成
            PermissionConfig::instance()->triggerAction(PermissionConfig::TidEdit, m_path);
        }
    }
}

void LibImageGraphicsView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit doubleClicked();
    }

    QGraphicsView::mouseDoubleClickEvent(e);
}

void LibImageGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);

    viewport()->setCursor(Qt::ArrowCursor);
    if (e->source() == Qt::MouseEventSynthesizedByQt && m_maxTouchPoints == 1) {
        const QRect &r = visibleImageRect();
        //double left=r.width()+r.x();
        const QRectF &sr = sceneRect();
        int xpos = e->pos().x() - m_startpointx;
        qDebug() << r.width();
        qDebug() << r.height();
        qDebug() << sr.width();
        qDebug() << sr.height();
        //fix 42660 2020/08/14 单指时间在QEvent处理，双指手势通过手势处理。为了解决图片放大后单指滑动手势冲突的问题
        if ((r.width() >= (sr.width() - 1) && r.height() >= (sr.height() - 1))) {

            if (abs(xpos) > 200 && m_startpointx != 0) {
                if (xpos > 0) {
                    emit previousRequested();
                } else {
                    emit nextRequested();
                }
            }
        }
        if ((QDateTime::currentMSecsSinceEpoch() - m_clickTime) < 200 && abs(xpos) < 50) {
            m_clickTime = QDateTime::currentMSecsSinceEpoch();
            emit sigClicked();
        }
    }
    m_startpointx = 0;
    m_maxTouchPoints = 0;
#ifdef tablet_PC
    if (m_press) {
        emit clicked();
    }
#endif
}

void LibImageGraphicsView::mousePressEvent(QMouseEvent *e)
{
#ifdef tablet_PC
    m_press = true;
#endif
    QGraphicsView::mousePressEvent(e);

    viewport()->unsetCursor();
    viewport()->setCursor(Qt::ArrowCursor);

#ifndef tablet_PC
    emit clicked();
#endif
    m_clickTime = QDateTime::currentMSecsSinceEpoch();
    m_startpointx = e->pos().x();
}

void LibImageGraphicsView::mouseMoveEvent(QMouseEvent *e)
{

    m_press = false;
    if (!(e->buttons() | Qt::NoButton)) {
        viewport()->setCursor(Qt::ArrowCursor);
        emit mouseHoverMoved();
    } else {
        QGraphicsView::mouseMoveEvent(e);
        viewport()->setCursor(Qt::ClosedHandCursor);

        emit transformChanged();
    }
    emit sigMouseMove();
}

void LibImageGraphicsView::leaveEvent(QEvent *e)
{
//    dApp->getDAppNew()->restoreOverrideCursor();

    QGraphicsView::leaveEvent(e);
}

void LibImageGraphicsView::resizeEvent(QResizeEvent *event)
{
    qDebug() << "---" << __FUNCTION__ << "---" << event->size();
    //20201027曾在右侧浮动窗口，关于多图片
    if (m_morePicFloatWidget) {
        //由于最小化窗口尺寸过小，会遮盖掉切换栏，抬高30px
        m_morePicFloatWidget->move(this->width() - 80, this->height() / 2 - 80);
    }
    titleBarControl();
    if (!m_isFitWindow) {
        setScaleValue(1.0);
    }

    QGraphicsView::resizeEvent(event);
//    m_toast->move(width() / 2 - m_toast->width() / 2,
//                  height() - 80 - m_toast->height() / 2 - 11);
}

void LibImageGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void LibImageGraphicsView::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *mimeData = e->mimeData();
    if (!pluginUtils::base::checkMimeData(mimeData)) {
        return;
    }
    e->accept();
}

void LibImageGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
//    QPixmap pm(12, 12);
//    QPainter pmp(&pm);
//    //TODO: the transparent box
//    //should not be scaled with the image
//    pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//    pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//    pmp.end();

    painter->save();
    painter->fillRect(rect, m_backgroundColor);

//    QPixmap currentImage(m_path);
//    if (!currentImage.isNull())
//        painter->fillRect(currentImage.rect(), QBrush(pm));
    painter->restore();
}

int static count = 0;
bool LibImageGraphicsView::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate ||
            evType == QEvent::TouchEnd) {
        if (evType == QEvent::TouchBegin) {
            count = 0;
            m_maxTouchPoints = 1;
        }
        if (evType == QEvent::TouchUpdate) {
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if (touchPoints.size() > count) {
                count = touchPoints.size();
            }
        }
        if (evType == QEvent::TouchEnd) {
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

            if (touchPoints.size() == 1 && count <= 1) {
                //QPointF centerPointOffset = gesture->centerPoint();
                qreal offset = touchPoints.at(0).lastPos().x() - touchPoints.at(0).startPos().x();
                if (qAbs(offset) > 200) {
                    if (offset > 0) {
                        emit previousRequested();
                        qDebug() << "zy------ImageView::event previousRequested";
                    } else {
                        emit nextRequested();
                        qDebug() << "zy------ImageView::event nextRequested";
                    }
                }
            }
        }
        /*lmh0804*/
//        const QRect &r = visibleImageRect();
//        double left = r.width() + r.x();
//        const QRectF &sr = sceneRect();
//        if (r.x() <= 1) {
//            return true;
//        }
//        if (left - sr.width() >= -1 && left - sr.width() <= 1) {
//            return true;
//        }
//        if (r.width() >= sr.width()) {
//            return true;
//        }
    } else if (evType == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent *>(event));

    return QGraphicsView::event(event);
}

void LibImageGraphicsView::onCacheFinish()
{
    hideSpinner();

    QVariantList vl = m_watcher.result();
    if (vl.length() == 2) {
        const QString path = vl.first().toString();
        if (path == m_path) {
            if (!m_pixmapItem) {
                return;
            }
            QPixmap pixmap = vl.last().value<QPixmap>();
            QPixmap tmpPixmap = pixmap;
            tmpPixmap.setDevicePixelRatio(devicePixelRatioF());
            if (!tmpPixmap.isNull()) {
                pixmap = tmpPixmap;
            }
            if (m_newImageRotateAngle != 0) {
                QTransform rotate;
                rotate.rotate(m_newImageRotateAngle);
                pixmap = pixmap.transformed(rotate, Qt::SmoothTransformation);
                m_newImageRotateAngle = 0;
            }
            m_pixmapItem->setGraphicsEffect(nullptr);
            m_pixmapItem->setPixmap(pixmap);
            setSceneRect(m_pixmapItem->boundingRect());
            autoFit();
            emit imageChanged(path);
            this->update();
            m_newImageLoadPhase = FullFinish;

            // AI修图 图像增强屏蔽更新缩略图和图像信息，以准确取得原始图片信息
            bool currentImageEnhance = AIModelService::instance()->isTemporaryFile(path);

            //刷新缩略图
            if (!pixmap.isNull() && !currentImageEnhance) {
                QPixmap thumbnailPixmap;
                if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
                    bool cache_exist = false;
                    if (pixmap.height() != 200 && pixmap.width() != 200) {
                        if (pixmap.height() >= pixmap.width()) {
                            cache_exist = true;
                            thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
                        } else if (pixmap.height() <= pixmap.width()) {
                            cache_exist = true;
                            thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
                        }
                    }
                    if (!cache_exist) {
                        if (static_cast<float>(pixmap.height()) / static_cast<float>(pixmap.width()) > 3) {
                            thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
                        } else {
                            thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
                        }
                    }
                }
                else {
                    thumbnailPixmap = pixmap.scaled(200,200);
                }
                emit currentThumbnailChanged(thumbnailPixmap, pixmap.size());
            }

        }
    }
}

//void ImageView::onThemeChanged(ViewerThemeManager::AppTheme theme)
//{
//    if (theme == ViewerThemeManager::Dark) {
//        m_backgroundColor = utils::common::DARK_BACKGROUND_COLOR;
//        m_loadingIconPath = utils::view::DARK_LOADINGICON;
//    } else {
//        m_backgroundColor = utils::common::LIGHT_BACKGROUND_COLOR;
//        m_loadingIconPath = utils::view::LIGHT_LOADINGICON;
//    }
//    update();
//}

void LibImageGraphicsView::scaleAtPoint(QPoint pos, qreal factor)
{
    // Remember zoom anchor point.
    const QPointF targetPos = pos;
    const QPointF targetScenePos = mapToScene(targetPos.toPoint());

    // Do the scaling.
    setScaleValue(factor);

    // Restore the zoom anchor point.
    //
    // The Basic idea here is we don't care how the scene is scaled or transformed,
    // we just want to restore the anchor point to the target position we've
    // remembered, in the coordinate of the view/viewport.
    const QPointF curPos = mapFromScene(targetScenePos);
    const QPointF centerPos = QPointF(width() / 2.0, height() / 2.0) + (curPos - targetPos);
    const QPointF centerScenePos = mapToScene(centerPos.toPoint());
    centerOn(static_cast<int>(centerScenePos.x()), static_cast<int>(centerScenePos.y()));
}

void LibImageGraphicsView::handleGestureEvent(QGestureEvent *gesture)
{
    if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void LibImageGraphicsView::pinchTriggered(QPinchGesture *gesture)
{
    m_maxTouchPoints = 2;
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        QPoint pos = mapFromGlobal(gesture->centerPoint().toPoint());
        if (abs(gesture->scaleFactor() - 1) > 0.006) {
            scaleAtPoint(pos, gesture->scaleFactor());
        }
    }
#ifndef tablet_PC
    //二指旋转
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        if (!m_bRoate || m_maxTouchPoints > 2) return;
        //释放手指后旋转的位置未结束不能进行下次旋转
        if (!m_rotateflag) {
            qDebug() << "ratateflag" << gesture->lastRotationAngle();
            gesture->setRotationAngle(gesture->lastRotationAngle());
            return;
        }
        qreal rotationDelta = gesture->rotationAngle() - gesture->lastRotationAngle();
        //防止在旋转过程中触发切换到下一张
        if (abs(gesture->rotationAngle()) > 20) m_bnextflag = false;
        if (abs(rotationDelta) > 0.2) {
            m_rotateAngelTouch = gesture->rotationAngle();
            rotate(rotationDelta);
        }
    }
//    if (gesture->state() == Qt::GestureFinished) {

//        QPointF centerPointOffset = gesture->centerPoint();
//        qreal offset = centerPointOffset.x() - m_centerPoint.x();
//        if (qAbs(offset) > 200) {
//            if (offset > 0) {
//                emit previousRequested();
//                qDebug() << "zy------ImageView::pinchTriggered previousRequested";
//            } else {
//                emit nextRequested();
//                qDebug() << "zy------ImageView::pinchTriggered nextRequested";
//            }
//        }
//        m_isFirstPinch = false;
//    }
#endif
    if (changeFlags & QPinchGesture::CenterPointChanged) {
        if (!m_isFirstPinch) {
            m_centerPoint = gesture->centerPoint();
            m_isFirstPinch = true;
        }
    }

    if (gesture->state() == Qt::GestureFinished) {
        m_isFirstPinch = false;
        gesture->setCenterPoint(m_centerPoint);
        //旋转松开手势操作
        // m_rotateAngelTouch = m_rotateAngelTouch % 360;
        //int abs(m_rotateAngelTouch);
        if (!m_bRoate) return;
        m_rotateflag = false;
        QPropertyAnimation *animation = new QPropertyAnimation(this, "rotation");
        animation->setDuration(200);
        if (m_rotateAngelTouch < 0) m_rotateAngelTouch += 360;
        qreal endvalue;
        if (abs(0 - abs(m_rotateAngelTouch)) <= 10) {
            endvalue = 0;
        } else if (abs(360 - abs(m_rotateAngelTouch)) <= 10) {
            endvalue = 0;
        } else if (abs(90 - abs(m_rotateAngelTouch)) <= 10) {
            endvalue = 90;
        } else if (abs(180 - abs(m_rotateAngelTouch)) <= 10) {
            endvalue = 180;
        } else if (abs(270 - abs(m_rotateAngelTouch)) <= 10) {
            endvalue = 270;
        } else {
            endvalue = 0;
        }
//         if(!m_bRoate) endvalue = 0;
        m_endvalue = endvalue;
        qreal startvalue;
        if (abs(m_rotateAngelTouch - endvalue) > 180) {
            startvalue = m_rotateAngelTouch - 360;
        } else {
            startvalue = m_rotateAngelTouch;
        }
        animation->setStartValue(startvalue);
        animation->setEndValue(endvalue);
        // qDebug()<<"angle finish" <<m_rotateAngelTouch << endvalue;
        connect(animation, &QVariantAnimation::valueChanged, [ = ](const QVariant & value) {
            qreal angle = value.toReal() - m_rotateAngelTouch;
            m_rotateAngelTouch = value.toReal();
            if (static_cast<int>(value.toReal()) != static_cast<int>(endvalue) /*|| m_imgSvgItem*/)
                this->rotate(angle);
            //setPixmap(pixmap.transformed(t));
        });
        // animation->setLoopCount(1); //旋转次数
        connect(animation, SIGNAL(finished()), this, SLOT(OnFinishPinchAnimal()));
        animation->start(QAbstractAnimation::KeepWhenStopped);
        qDebug() << "finish";

    }
}

void LibImageGraphicsView::OnFinishPinchAnimal()
{

    m_rotateflag = true;
    m_bnextflag = true;
    m_rotateAngelTouch = 0;
//    if(m_imgSvgItem)
//    {
//      //  reloadSvgPix(m_path,90,false);
//        m_rotateAngel += m_endvalue;
//        dApp->m_imageloader->updateImageLoader(QStringList(m_path), true,static_cast<int>(m_endvalue));
//        emit dApp->signalM->sigUpdateThunbnail(m_path);
//        return;
//    }
    if (!m_pixmapItem) return;
    //QStranform旋转到180度有问题，暂未解决，因此动画结束后旋转Pixmap到180
    QPixmap pixmap;
    pixmap = m_pixmapItem->pixmap();
    QTransform rotate;
    rotate.rotate(m_endvalue);

    pixmap = pixmap.transformed(rotate, Qt::FastTransformation);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    scene()->clear();
    resetTransform();
    m_pixmapItem = new LibGraphicsPixmapItem(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    // Make sure item show in center of view after reload
    QRectF rect = m_pixmapItem->boundingRect();
    //            rect.setHeight(rect.height() + 50);
    setSceneRect(rect);
    //            setSceneRect(m_pixmapItem->boundingRect());

    scene()->addItem(m_pixmapItem);
    scale(m_scal, m_scal);
    if (m_bRoate) {
        m_rotateAngel += m_endvalue;
        if (m_endvalue > 0) {
            emit gestureRotate(static_cast<int>(0));
            QPixmap thumbnailPixmap;
            if (0 != pixmap.height() && 0 != pixmap.width() && (pixmap.height() / pixmap.width()) < 10 && (pixmap.width() / pixmap.height()) < 10) {
                bool cache_exist = false;
                if (pixmap.height() != 200 && pixmap.width() != 200) {
                    if (pixmap.height() >= pixmap.width()) {
                        cache_exist = true;
                        thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
                    } else if (pixmap.height() <= pixmap.width()) {
                        cache_exist = true;
                        thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
                    }
                }
                if (!cache_exist) {
                    if (static_cast<float>(pixmap.height()) / static_cast<float>(pixmap.width()) > 3) {
                        thumbnailPixmap = pixmap.scaledToWidth(200,  Qt::FastTransformation);
                    } else {
                        thumbnailPixmap = pixmap.scaledToHeight(200,  Qt::FastTransformation);
                    }
                }
            }
            emit currentThumbnailChanged(thumbnailPixmap, pixmap.size());
            emit UpdateNavImg();
        }
    }
    qDebug() << m_endvalue;
    //结束需要更改标题栏状态
    titleBarControl();
}

/**
   @brief 取得用于加载 \a path 文件过程中的模糊图片，此图片通过缓存 \a info 中的缩略图放大取得。
        图片还未进行模糊，而是通过设置 QGraphicsBlurEffect 实现
 */
QPixmap LibImageGraphicsView::getBlurPixmap(const QString &path, const imageViewerSpace::ItemInfo &info, const QPixmap &previousPix)
{
    QPixmap pix;
    QImageReader imagreader(path);      //取原图的分辨率
    int w = imagreader.size().width();
    int h = imagreader.size().height();

    int wScale = 0;
    int hScale = 0;
    int wWindow = 0;
    int hWindow = 0;
    if (QApplication::activeWindow()) {
        wWindow = static_cast<int>(QApplication::activeWindow()->width() * devicePixelRatioF());
        hWindow = static_cast<int>((QApplication::activeWindow()->height() - TITLEBAR_HEIGHT * 2) * devicePixelRatioF());
    } else {
        wWindow = static_cast<int>(this->width() * devicePixelRatioF());
        hWindow = static_cast<int>((this->height() - TITLEBAR_HEIGHT * 2) * devicePixelRatioF());
    }

    if (w >= wWindow) {
        wScale = wWindow;
        hScale = wScale * h / w;
        if (hScale > hWindow) {
            hScale = hWindow;
            wScale = hScale * w / h;
        }
    } else if (h >= hWindow) {
        hScale = hWindow;
        wScale = hScale * w / h;
        if (wScale >= wWindow) {
            wScale = wWindow;
            hScale = wScale * h / w;
        }
    } else {
        wScale = w;
        hScale = h;
    }
    if (wScale == 0 || wScale == -1) { //进入这个地方说明QImageReader未识别出图片
        if (info.imgOriginalWidth > wWindow || info.imgOriginalHeight > hWindow) {
            wScale = wWindow;
            hScale = hWindow;
        } else {
            wScale = info.imgOriginalWidth;
            hScale = info.imgOriginalHeight;
        }
    }

    if (previousPix.isNull()) {
        pix = QPixmap::fromImage(info.image).scaled(wScale, hScale, Qt::KeepAspectRatio);
    } else {
        pix = previousPix.scaled(wScale, hScale, Qt::KeepAspectRatio);
    }

    // 存在缩放比问题需要setDevicePixelRatio
    pix.setDevicePixelRatio(devicePixelRatioF());
    return pix;
}

/**
   @brief 设置图片旋转加载图标，当图片无缩略图，无法使用模糊加载效果时，使用此加载器显示加载效果。
        \a enhanceImage 用于 AI 图像增强时使用，显示不同文案
   @note 加载控件由QVBoxLayout布局管理，而不是scene管理，当重新进入 setImage() 时，会自动隐藏
 */
void LibImageGraphicsView::addLoadSpinner(bool enhanceImage)
{
    if (!m_spinner) {
        m_spinner = new DSpinner(this);
        m_spinner->setFixedSize(SPINNER_SIZE);

        m_spinnerCtx = new QWidget(this);
        m_spinnerCtx->setFixedSize(SPINNER_SIZE);
        QVBoxLayout *hLayout = new QVBoxLayout;
        hLayout->setMargin(0);
        hLayout->setSpacing(0);
        hLayout->addWidget(m_spinner, 0, Qt::AlignCenter);

        // 图像增强增加文案，默认隐藏
        m_spinnerCtx->setFixedWidth(300);
        m_spinnerCtx->setFixedHeight(70);
        m_spinnerLabel = new QLabel(m_spinnerCtx);
        m_spinnerLabel->setText(tr("AI retouching in progress, please wait..."));
        m_spinnerLabel->setVisible(false);
        hLayout->addWidget(m_spinnerLabel, 1, Qt::AlignBottom | Qt::AlignHCenter);

        m_spinnerCtx->setLayout(hLayout);

        if (!this->layout()) {
            QVBoxLayout *lay = new QVBoxLayout;
            lay->setAlignment(Qt::AlignCenter);
            this->setLayout(lay);
        }
        this->layout()->addWidget(m_spinnerCtx);
    }

    m_spinner->start();

    m_spinnerCtx->setVisible(true);
    m_spinner->setVisible(true);
    m_spinnerLabel->setVisible(enhanceImage);
}

/**
   @brief 隐藏加载图标和提示
 */
void LibImageGraphicsView::hideSpinner()
{
    if (m_spinnerCtx) {
        m_spinnerCtx->hide();
    }

    if (m_spinner) {
        m_spinner->stop();
        m_spinner->hide();
    }

    if (m_spinnerLabel) {
        m_spinnerLabel->hide();
    }
}

void LibImageGraphicsView::wheelEvent(QWheelEvent *event)
{
    // 加载过程不可缩放
    if (m_spinner && m_spinner->isVisible()) {
        return;
    }

    if ((event->modifiers() == Qt::ControlModifier)) {
        if (event->delta() > 0) {
            emit previousRequested();
        } else if (event->delta() < 0) {
            emit nextRequested();
        }

    } else {
        QFileInfo file(m_path);
        if (!file.exists()) {
            event->accept();
        } else {

            qreal factor = qPow(1.2, event->delta() / 240.0);
            qDebug() << factor;
            scaleAtPoint(event->pos(), factor);

            event->accept();
        }
    }
//    qDebug() << "---" << __FUNCTION__ << "---" << this->sceneRect();
}

//CFileWatcher::CFileWatcher(QObject *parent): QThread(parent)
//{
//    _handleId = inotify_init();
//}

//CFileWatcher::~CFileWatcher()
//{
//    clear();
//}

//bool CFileWatcher::isVaild()
//{
//    return (_handleId != -1);
//}

//void CFileWatcher::addWather(const QString &path)
//{
//    QMutexLocker loker(&_mutex);
//    if (!isVaild())
//        return;
//    QFileInfo info(path);
//    if (!info.exists() || !info.isFile()) {
//        return;
//    }
//    if (watchedFiles.find(path) != watchedFiles.end()) {
//        return;
//    }
//    std::string sfile = path.toStdString();
//    int fileId = inotify_add_watch(_handleId, sfile.c_str(), IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
//    watchedFiles.insert(path, fileId);
//    watchedFilesId.insert(fileId, path);
//    if (!_running) {
//        _running = true;
//        start();
//    }
//}

//void CFileWatcher::removePath(const QString &path)
//{
//    QMutexLocker loker(&_mutex);
//    if (!isVaild())
//        return;
//    auto itf = watchedFiles.find(path);
//    if (itf != watchedFiles.end()) {
//        inotify_rm_watch(_handleId, itf.value());
//        watchedFilesId.remove(itf.value());
//        watchedFiles.erase(itf);
//    }
//}

//void CFileWatcher::clear()
//{
//    QMutexLocker loker(&_mutex);
//    for (auto it : watchedFiles) {
//        inotify_rm_watch(_handleId, it);
//    }
//    watchedFilesId.clear();
//    watchedFiles.clear();
//}

//void CFileWatcher::run()
//{
//    doRun();
//}

//void CFileWatcher::doRun()
//{
//    if (!isVaild())
//        return;

//    char name[1024];
//    auto freadsome = [ = ](void *dest, size_t remain, FILE * file) {
//        char *offset = reinterpret_cast<char *>(dest);
//        while (remain) {
//            if (file == nullptr)
//                return -1;
//            size_t n = fread(offset, 1, remain, file);
//            if (n == 0) {
//                return -1;
//            }

//            remain -= n;
//            offset += n;
//        }
//        return 0;
//    };

//    FILE *watcher_file = fdopen(_handleId, "r");

//    while (true) {
//        inotify_event event;
//        if (-1 == freadsome(&event, sizeof(event), watcher_file)) {
//            qWarning() << "------------- freadsome error !!!!!---------- ";
//            break;
//        }
//        if (event.len) {
//            freadsome(name, event.len, watcher_file);
//        } else {
//            QMutexLocker loker(&_mutex);
//            auto itf = watchedFilesId.find(event.wd);
//            if (itf != watchedFilesId.end()) {
//                //qDebug() << "file = " << itf.value() << " event.wd = " << event.wd << "event.mask = " << event.mask;

//                if (event.mask & IN_MODIFY) {
//                    emit fileChanged(itf.value(), EFileModified);
//                } else if (event.mask & IN_MOVE_SELF) {
//                    emit fileChanged(itf.value(), EFileMoved);
//                } else if (event.mask & IN_DELETE_SELF) {
//                    emit fileChanged(itf.value(), EFileMoved);
//                }
//            }
//        }
//    }
//}
