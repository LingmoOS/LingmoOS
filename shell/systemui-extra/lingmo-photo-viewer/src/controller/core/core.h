#ifndef CORE_H
#define CORE_H

#include "base.h"

#define SET_BACKGROUND_PICTURE_GSETTINGS_PATH "org.mate.background"
#define SET_BACKGROUND_PICTURE_GSETTINGS_NAME "pictureFilename"

class MyStandardItem : public QStandardItem
{
public:
    void setPath(const QString &path);
    QString getPath();

private:
    QString m_path;
};

class Core : public QObject, public NavigationStatus
{
    Q_OBJECT

Q_SIGNALS:
    void needOpenImage(QString path);                  //启动时打开图片
    void openFinish(QVariant var);                     //打开完成
    void showNavigation(QPixmap pix);                  //操作导航器
    void coreProgremExit();                            //结束进程
    void changeAlbumHighLight(QModelIndex modelIndex); //在相册中选中到所切换的图片
    void delayShow(bool isLoading);                    //处理图片加载缓慢问题
    void openFromAlbum();                              //从相册打开
    void renameResult(int code, QFileInfo newPath);    //返回报错信息和新路径
    void canDelImage(bool canDelOrnot);                //返回是否可删除信号
    void updateSideNum(int sideBength);                //更新相册长度
    void fullScreen();                                 //处理dbus信号-全屏
    void releaseRegion(QPoint start, QPoint end);
    void cutSaveFinish();           //裁剪结束
    void returnMouseType(int type); //回传鼠标形状
    void isCutNotSave();            //关闭时-正在裁剪，给前端发信号询问是否保存更改
    void sendHightlightPos(QPoint startPos, QPoint endPos); //给前端发送导航器高亮区域范围
    void getTextResult(QVector<QString>);
    void exitOcrModeSign();
    void startOcr(QString path);

    void getLeftUpPosAccordingImage(QPointF leftUpPos); //返回已经显示的部分相对在图片上的位置

    void returnReplaceAndOrigImage(QImage replaceImage, QImage origImage); //返回背景替换之后的图片和原图
    void returnFlipAndOrigImage(QImage filpImage, QImage origImage);       //返回翻转图片
    void returnScanAndOrigImage(QImage scanImage, QImage origImage);       //返回扫描之后的图片和原图
    void returnScannerFlipAndOrigImage(QImage filpImage, QImage origImage);//返回翻转图片
    void returnScannerSaveResult(bool result);                             //返回保存结果

public:
    Core();
    ~Core();
    QString initDbus(const QStringList &arguments);    //初始化Dbus模块
    void findAllImageFromDir(QString fullPath);        //寻找目录下所有支持的图片
    void openImage(QString fullPath);                  //打开图片
    void changeImage(const int &type);                 //切换图片
    void changeImageFromClick(QModelIndex modelIndex); //切换图片
    void changeWidgetSize(const QSize &size);          //切换窗口大小
    void changeImageShowSize(ImageShowStatus::ChangeShowSizeType type,
                             QPoint focus = QPoint()); //图片显示状态（放大缩小）
    void clickNavigation(const QPoint &point = QPoint(-1, -1), bool isDragImage = true); //导航器点击
    void flipImage(const Processing::FlipWay &way);                                      //翻转处理
    void deleteImage();                                                                  //删除图片
    void setAsBackground();                                                              //设置为背景图
    void openInfile();                                                                   //文件夹中打开
    void changeOpenIcon(QString theme);        //主题改变时切换相册打开按钮
    void saveMovieFinish(const QString &path); //异步处理完成
    void tiffOperateFinish(const QString &path);
    void close();                                            //关闭进程
    QStandardItemModel *getAlbumModel();                     //获取相册model指针
    void albumLoadFinish(QVariant var);                      //预览加载完成
    void albumLoadRealFinish(QVariant var);                  //预览加载图片完成
    void toCoreChangeName(QString oldName, QString newName); //接收重命名的处理
    void toPrintImage(QPrinter *printer);                    //打印
    bool openFail(QString fullPath);                         //打开失败，判断是否为拖拽打开失败
    void cutImage();                                         //开始裁剪，处理显示的图片
    void cutRegion(const QPoint &point);                     //裁剪区域
    void startChange(const QPoint &point);                   //起始点-裁剪
    void realsePos(const QPoint &point);                     //裁剪-释放鼠标
    void releaseCutChange(int type, const QPoint &point);    //裁剪-释放鼠标后再次改变尺寸
    void exitCut();                                          //退出裁剪
    void needSaveCutImage(int proportion);                   //保存裁剪图片
    void saveCutImage(int proportion, const QPoint &startP, const QPoint &endP);  //保存裁剪图片
    void needSaveAsImage(QString path);                      //另存为
    void reloadImage(QModelIndex modelIndex);                //判断是否需要加载
    void startSignApp();                                     //调起画图
    void getTextFromImage();                                 //获取当前打开图片中文字



    // qml------------------------------------------
    void qmlCutImageDisplay(QPointF startPos, QSizeF displaySize, double imageScale); //处理裁剪

    void qmlNavigation(QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale);      //导航器
    void qmlClickNavigation(QPointF curMousePos = QPointF(-1, -1), bool needChangeDisplayImage = false); //点击导航器
    void qmlTiffPage(int pageNum, QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale);
    void qmlTiffImagePage(int pageNum);
    //裁剪-显示区域全图
    void cutFullViableImage();

    //标注
    void startMark();
    void markImage(QVariant var); //按下后，图片标注
    void markUndo();
    void markSave(QString savePath);
    void markClear();

    //背景替换
    Mat m_transparentMat;
    Mat m_origMattingMat;
    Mat m_replaceMat;
    Mat m_mattingCutMat;
    QImage m_origMattingImage;
    bool m_isSaveCutImage = false;
    void matToSamples();
    void picMatting(QImage pic);
    void replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter);
    void finishMatting(Mat mat);
    void needSaveReplaceImage(QString path);
    void mattingFlip(const Processing::FlipWay &way);

    //扫描黑白件
    bool m_isWatermark = false;             //是否添加水印
    Mat m_scanMat;                          //扫描的图片
    Mat m_watermarkMat;                     //添加水印的图片
    QImage m_origScanImage;                 //原图
    void scanImage();                                                           //扫描图片
    void needSaveScanImage(QString savePath);                                   //保存图片
    void printScanImage(QPrinter *printer);                                     //调用全局打印
    void scannerSavePDF(QPrinter *printer);                                     //另存为PDF
    void scannerAddWatermark(bool isWatermark, QString vmContent);              //添加水印
    void scannerFlip(const Processing::FlipWay &way);                           //旋转和翻转操作
    void scannerSaveCutImage(int proportion, const QPoint &startP, const QPoint &endP);
private:
    void initCore();                    //初始化核心
    Dbus *m_dbus = nullptr;             // DBus模块对象
    void showImage(const QPixmap &pix); //显示图片
    void showImageOrMovie();            //显示图片或动图
    void creatImage(const int &proportion = -1,
                    bool defaultOpen = true); //生成图像，defaultOpen区分是否是打开图片，无其他操作
    void defaultImage(int proportion, int defaultProportion); //默认显示图片原则
    void operateImage(int proportion, int defaultProportion); //进行操作显示图片原则
    void processingCommand(const QStringList &cmd);           //处理终端命令
    QString processingApi(const QStringList &cmd);            //处理外部命令
    void processingFullscreen();
    void creatModule(const QString &path, QStringList list);      //创建module
    void loadAlbum(QStringList list, int start = 1, int end = 1); //加载相册
    void loadImgCache(QModelIndex modelIndex); //加载图片缓存

    void navigation(const QPoint &point = QPoint(-1, -1), bool isDragImage = true); //导航器
    void playMovie();                                                               //播放动图的槽函数
    inline void changeImageType(QString path = "");                                 //修改图片标签
    Mat changeMat(Mat mat);                                                         //更改当前图片
    void creatNavigation();                    //创建导航器图片等数据，用于节省算力
    void deleteAlbumItem(const QString &path); //删除相册中的项
    ChamgeImageType nextOrBack(const QString &oldPath, const QString &newPath);
    RenameState successOrFail(const QString &oldPath, const QString &newPath); //返回重命名结果
    QString nextImagePath(const QString &oldPath);
    QString backImagePath(const QString &oldPath);
    void setHighLight(const QString &path);
    bool coreOperateTooOften();              //操作过于频繁
    void progremExit();                      //结束进程
    bool apiFunction();                      //处理api函数
    bool isSamePath(QString path);           //判断打开的是不是相同路径
    QStringList diffPath(QStringList image); //判断打开的同一个路径下的图片文件夹内是否还有其他图片
    void needSave();                         //判断是否需要保存图片
    void processNewLoadImage(); //处理--上一次图片没处理完就进行下一次操作时，更新对齐动图每一帧的操作状态
    bool m_isclose = false;
    MyStandardItem *m_item0 = nullptr;
    bool m_shouldClose = false;     //可以关闭
    bool m_isApi = false;           //作为API接口运行
    bool m_apiReplaceFile = false;  // api替换原图
    QStringList m_apiCmd;           //保存操作命令
    QTimer *m_canProcess = nullptr; //限制操作频率，降低cpu占用
    QThread *m_ocrThread = nullptr;

    //裁剪部分
    QPoint boundingSet(QPoint point, int xMax, int yMax); //计算边界值
    void cutFinishReset();                                //裁剪完毕，状态重置
    void openSaveImage();                                 //针对于裁剪和另存为，打开保存的图片
    void loadOnChange(QString path);                      //切换时加载--拿QModelIndex

    OCR m_ocr;
    bool m_isCanDel = false;                          //判断是否可删除，默认为true
    QImage m_origCutImage;                            //待继续处理和显示的裁剪的图，带灰色蒙层的
    QImage m_origCutImageBefore;                      //待显示的裁剪的背景图-原图
    bool m_isCutting = false;                         //正在裁剪
    QPoint m_cutBeforeStartPosition = QPoint(-1, -1); //记录起始点
    QPoint m_cutStartPostionSave = QPoint(-1, -1);    //保存前一个裁剪的起始点
    QSize m_cuthightlightSize;                        //高亮区域大小;
    QSize m_cuthightlightSizeBefore = QSize(0, 0);    //高亮区域大小-上一次的
    int m_cutspaceWidth = 0;                          //导航栏窗口与缩略图左边缘距离
    int m_cutspaceHeight = 0;                         //导航栏窗口与缩略图上边缘距离
    QImage m_cutImage;                                //记录鼠标释放后的图像
    bool m_hasCutBox = false;                         //有裁剪框存在的情况
    QPoint m_startP = QPoint(-1, -1);
    QPoint m_endP = QPoint(-1, -1);
    QPoint m_backPointRelease = QPoint(-1, -1);
    bool m_isReleasePaint = false; //正在进行释放后又绘制的动作--默认为false，没有进行动作
    int m_backType = -1;           //标记上一个动作的鼠标类型
    bool m_isCutSaved = false;     //记录裁剪是否保存--默认为false--已完成或未开始
    QString m_paperFormat;
    QStringList m_loadedImage;  //已经加载了的图片
    QHash<QString, QPair<MatAndFileinfo, QPixmap>> m_loadedImageCache;  //预加载图片
    QStringList m_allImageList; //所有图片的路径
    int m_currentPage = 0;      //存当前页，处理某些特殊情况

    bool judgeSizeIsZero(QSize size); //判断尺寸为中宽高有等于0的
    CutRegion cutRegionSelection(QPointF startPos, QSize cutSize, QPixmap nowImage);
    CutRegion calcCutRegionSelection(QPointF startPos, QSize cutSize, QPixmap nowImage);
    bool m_isClickBtnChangeImageSize = false; //判断是通过点击缩放按钮来改变的图片尺寸，默认不是
    int m_imageShowWay = 3;                   //图片显示方式--默认自适应
    bool m_imageNeedUpdate = true;            //图片是需要更新--默认需要
    bool m_isDragUpdateImage = true; //是否为拖动导航栏显示图片-需要更新图片，默认为拖动，更新图片
    //调起画图，文件监听
    void processLog();
    QProcess *m_process = nullptr;
    QFileSystemWatcher *m_fileWatcher = nullptr;
    void fileChangedNeedUpdated();
    QString m_monitoredFiles = "";
    void sendOcrResult(QString path, QVector<QString> result); // ocr识别完成后，发送出去给前端

    QString m_backImagePath = "";

    QPoint m_imageLeftPos = QPoint(-1, -1);


    QPointF qmlBoundaryPos(QPointF pointf, int xMax, int yMax);
    QSizeF qmlBoundarySize(QPointF pointf, QSizeF displaySize);
    bool qmlNaviCheckBoundary(QSizeF visibleSize, QPointF curVisiblePartLeftUpPos, double imageScale);
    void qmlCreatNavigation(QSizeF displaySize);
    double m_qmlProportion = 0;
    QPointF m_backHightLight = QPointF(-1, -1);
    QList<QImage> m_tiffImageList;
    void tiffOperate();
    QList<FIBITMAP *> m_fibilist;
    int m_tiffPage = 1;
    int m_backPage = -1;
    QPixmap m_tiffNaviPix = QPixmap();
    bool m_tiffHasOperatedFinish = true;
    QString m_nowRealFormat = "";
    Mark::MarkTool m_markTool;
    QPixmap m_origPix;
    QList<Processing::FlipWay> m_rotateRecord;
    QString m_naviBackPixPath = "";
    //    QTimer *m_time = nullptr;
    QStringList m_swipeImageList; //存储需要显示的图片,让当前需要显示的图片一直处于中间的位置

    // story 19807
    int m_imageTotalNum = -1;                               //图片增加的数量
    QString m_imageDelPath = "";                            //需要删掉的图片路径
    QStringList needLoadImagePathList();                    //更新每次需要加载的5张图片的路径
    QList<int> m_showImageIndex;                            //当前路径上显示的5张图片的index
    QList<int> needUpdateImageIndex(QStringList imageList); //更新每次需要加载的5张图片的index
    void needUpdateAddImageType(QStringList addImagePath); //更新每次打开文件夹的时候需要增加的图片的类型
    QList<int> needUpdateImageType(QStringList imageList); //更新需要加载的5张图片的类型
    QStringList m_imageListEveryAdd;                       //每次新增的图片
    QList<int> m_imageListEveryAddType;                    //每次新增的图片的类型
    // story 19807
    QString m_theme = "lingmo-default";

    bool m_needDel = false;
};

#endif // CORE_H
