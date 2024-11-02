#ifndef INTERACTION_H
#define INTERACTION_H

#include <QObject>
#include <QPixmap>
#include <QPrinter>
#include <QVariant>
#include <QFileInfo>
#include <QStandardItemModel>
class coreinteraction;

class Interaction : public QObject
{
    Q_OBJECT


Q_SIGNALS:

    /*
     * 接口功能：通知UI层启动时需要打开图片
     * 接口场景：双击图片调起等带图片参数启动本程序时
     * 接口类型：信号，需要绑定
     * 参数类型：QString
     * 参数描述：图片路径
     * 备注：如果路径值为Variable::API_TYPE,则不显示主界面
     */
    void startWithOpenImage(QString);

    /*
     * 接口功能：图片处理完成，通知UI层显示图片，以及把图片相关信息发给前端
     * 接口场景：任何图片操作完成需要UI层显示的时候
     * 接口类型：信号，需要绑定
     * 参数类型：QVariant -> ImageAndInfo 例：ImageAndInfo package=var.value<ImageAndInfo>();
     * 参数描述：结构体，图片及相关信息信息，详情请参见variable.h文件中的ImageAndInfo结构体
     * 备注1：当type值为0时，说明所有图片都被删掉，显示添加图片界面
     * 备注2：当QPixmap::isnull()为true时，要显示正在加载动画，放大、缩小、旋转、删除等会全部禁用
     */
    void openFinish(QVariant var);

    /*
     * 接口功能：显示或隐藏导航栏
     * 接口场景：更改图片显示状态（放大缩小、显示原图等）或更改窗口尺寸时
     * 接口类型：信号，需要绑定
     * 参数类型：QPixmap
     * 参数描述：导航栏要显示的预览图，如果为空，则关闭导航栏
     */
    void showNavigation(QPixmap pix);

    /*
     * 接口功能：结束进程
     * 接口场景：任务队列中所以任务处理完毕，可以退出时
     * 接口类型：信号，需要绑定
     * 备注：绑定MainWindow的close方法
     */
    void progremExit();

    /*
     * 接口功能：点击相册切换图片
     * 接口场景：点击相册中的图片时
     * 接口类型：信号，需要绑定QListView的click信号
     */
    void changeImageFromClick(QModelIndex modelIndex); //切换到所选图片

    /*
     * 接口功能：切换图片时，相册设置高亮
     * 接口场景：切换图片时
     * 接口类型：信号，需要绑定QListView的setCurrentIndex槽
     */
    void changeAlbumHighLight(QModelIndex modelIndex); //在相册中选中到所切换的图片
    /*
     * 接口功能：切换图片时，图片加载缓慢需要前端对应做处理
     * 接口场景：切换图片时
     * 接口类型：信号，需要绑定前端图片未加载时的界面显示函数
     */
    void delayShow(bool isLoading); //处理图片加载缓慢
    /*
     * 接口功能：打开图片
     * 接口场景：从相册点击打开按钮
     * 接口类型：信号，需要绑定
     * 参数类型：无
     */
    void openFromAlbum();
    /*
     * 接口功能：重命名
     * 接口场景：给前端信息
     * 接口类型：信号，需要绑定
     * 参数类型：无
     */
    void sendRenameResult(int code, QFileInfo newFile); //返回重命名信息
    /*
     * 接口功能：显示文件是否可被删除
     * 接口场景：打开图片时，判断文件是否可被删除
     * 接口类型：信号，需要绑定
     * 参数类型：bool
     * 参数描述：如果为false，则不能被删除，按钮需要置灰
     */
    void changeDelState(bool canDelete);
    /*
     * 接口功能：更新相册长度
     * 接口场景：删除图片时，更新相册长度
     * 接口类型：信号，需要绑定
     * 参数类型：int
     * 参数描述：实时更新相册长度
     */
    void updateSideLength(int sideLength);
    /*
     * 接口功能：全屏
     * 接口场景：全屏
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void fullScreen();
    /*
     * 接口功能：返回裁剪框起点和终点
     * 接口场景：鼠标释放时，返回两个点
     * 接口类型：信号，需要绑定
     * 参数类型：QPoint,QPoint
     * 参数描述：用于告知界面裁剪区域
     */
    void releaseRegion(QPoint start, QPoint end);
    /*
     * 接口功能：裁剪完成，告知前端界面裁剪完成
     * 接口场景：按保存按钮或者enter键，保存完成后发出
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void cutSaveFinish();
    /*
     * 接口功能：返回识别结果
     * 接口场景：用户点击文字识别，程序识别结束时触发
     * 接口类型：信号，需要绑定
     * 参数类型：QVector<QString>
     * 参数描述：识别结果
     */
    void getTextFinish(QVector<QString>);
    /*
     * 接口功能：返回鼠标形状
     * 接口场景：鼠标没有释放，状态不能改变，保证鼠标一直是当前这个状态
     * 接口类型：信号，需要绑定
     * 参数类型：int
     * 参数描述：用于告知界面当前鼠标的类型
     */
    void returnMouseShape(int type);
    /*
     * 接口功能：给前端弹窗询问
     * 接口场景：关闭时正在裁剪，还未保存
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void isCutNotSave(); //关闭时，正在裁剪
    /*
     * 接口功能：给前端高亮区域位置信息
     * 接口场景：鼠标悬停在导航栏时，需要便装抓手，回传位置
     * 接口类型：信号，需要绑定
     * 参数类型：QPoint，QPoint
     * 参数描述：传起始位置和终点位置
     */
    void sendHightlightPos(QPoint startPos, QPoint endPos);
    /*
     * 接口功能：退出ocr
     * 接口场景：鼠标悬停在导航栏时，需要便装抓手，回传位置
     * 接口类型：信号，需要绑定
     * 参数类型：QPoint，QPoint
     * 参数描述：传起始位置和终点位置
     */
    void exitOcrModeSign();
    /*
     * 接口功能：导航栏操作更改图片位置
     * 接口场景：鼠标悬停在导航栏时，需要便装抓手，回传位置
     * 接口类型：信号，需要绑定
     * 参数类型：QPointF
     * 参数描述：传图片显示的区域在图片上的位置
     */
    void getLeftUpPosAccordingImage(QPointF leftUpPos);

    void returnReplaceAndOrigImage(QImage replaceImage, QImage origImage);//返回背景替换之后的图片和原图
    void returnFlipAndOrigImage(QImage filpImage, QImage origImage);      //返回翻转图片
    void returnScanAndOrigImage(QImage scanImage, QImage origImage);           //返回扫描之后的图片和原图
    void returnScannerFlipAndOrigImage(QImage filpImage, QImage origImage);    //返回翻转图片
    void returnScannerSaveResult(bool result);

public:
    /*
     * 接口功能：单例取指针，供UI层实例化接口对象
     * 接口场景：程序启动时，首次调用要放到主界面构造函数的第一行
     * 接口类型：函数，直接调用
     * 返回类型：Interaction
     * 返回描述：接口对象
     */
    static Interaction *getInstance();

    /*
     * 接口功能：初始化核心模块
     * 接口场景：程序启动时，放在主界面构造函数的第二行，只需调用这一次，多次调用无效果
     * 接口类型：函数，直接调用
     * 参数类型：QStringList
     * 参数描述：程序启动时传入的参数，一般来自QApplication::arguments()
     */
    virtual void creatCore(const QStringList &list) = 0;

    /*
     * 接口功能：通知后端UI初始化完成
     * 接口场景：主界面构造完成时，放在主界面构造函数的最后一行，只需调用这一次，多次调用无效果
     * 接口类型：函数，直接调用
     */
    virtual void initUiFinish() = 0;

    /*
     * 接口功能：打开图片
     * 接口场景：拖拽或点击按钮添加图片时或收到startWithOpenImage(QString)信号时
     * 接口类型：函数，直接调用
     * 参数类型：QString
     * 参数描述：包含图片路径的文件名
     * 返回类型：QVariant -> QList<int> 例：QList<int> list=var.value<QList<int>>();
     * 返回描述：图片唯一标识的队列，用来实例化相册
     */
    virtual void openImage(const QString &path) = 0;

    /*
     * 接口功能：切换图片
     * 接口场景：点击切换图片按钮或识别到对应手势时
     * 接口类型：函数，直接调用
     */
    virtual void nextImage() = 0; //切换到下一张图片
    virtual void backImage() = 0; //切换到上一张图片

    /*
     * 接口功能：将显示图片的窗口的尺寸发送给后端
     * 接口场景：切换窗口大小或最大化/窗口化时
     * 接口类型：函数，直接调用
     * 参数类型：QSize
     * 参数描述：显示图片的窗口的尺寸
     */
    virtual void changeWidgetSize(const QSize &size) = 0;

    /*
     * 接口功能：切换图片显示状态
     * 接口场景：点击工具栏对应按钮或识别到对应手势时
     * 接口类型：函数，直接调用
     */
    virtual void watchBigImage(QPoint focus = QPoint()) = 0;   //图片放大
    virtual void watchSmallImage(QPoint focus = QPoint()) = 0; //图片缩小
    virtual void watchOriginalImage() = 0;                     //查看原图
    virtual void watchAutoImage() = 0;                         //自适应窗口大小

    /*
     * 接口功能：将鼠标在导航栏的坐标发送给后端，用于显示图片不同的部分
     * 接口场景：在导航栏上点击/拖动鼠标时
     * 接口类型：函数，直接调用
     * 参数类型：QPoint
     * 参数描述：鼠标在导航栏的坐标
     */
    virtual void clickNavigation(const QPoint &point, bool isDragImage) = 0; //导航器点击

    /*
     * 接口功能：旋转图片
     * 接口场景：点击工具栏对应按钮或识别到对应手势时
     * 接口类型：函数，直接调用
     * 参数类型：bool
     * 参数描述：无参/true - 顺时针   false - 逆时针
     */
    virtual void rotate(const bool &clockwise = true) = 0; //旋转

    /*
     * 接口功能：操作图片
     * 接口场景：点击工具栏对应按钮时
     * 接口类型：函数，直接调用
     */
    virtual void flipH() = 0;       //水平翻转
    virtual void flipV() = 0;       //垂直翻转
    virtual void deleteImage() = 0; //删除图片

    /*
     * 接口功能：将当前查看的图片设置为桌面背景
     * 接口场景：鼠标右键菜单中的设置为桌面背景选项时
     * 接口类型：函数，直接调用
     * 备注：在生成鼠标右键菜单时要进行校验，只有Variable::BACKGROUND_SUPPORT_FORMATS中存在的格式才显示这个选项
     */
    virtual void setAsBackground() = 0; //设置为背景图
    /*
     * 接口功能：查看当前图片所在文件夹并选中
     * 接口场景：鼠标右键菜单中的在文件夹中显示选项
     * 接口类型：函数，直接调用
     */
    virtual void openImageInfile() = 0; //在文件夹中打开
    /*
     * 接口功能：主题切换。相册打开按钮图标更换
     * 接口场景：主题切换
     * 接口类型：函数，直接调用
     * 参数类型：QString
     */
    virtual void changeOpenIcon(QString theme) = 0; //更改主题，切换相册打开按钮图标
    /*
     * 接口功能：结束当前进程
     * 接口场景：点击退出按钮时
     * 接口类型：函数，直接调用
     */
    virtual void close() = 0; //关闭

    /*
     * 接口功能：获取相册model指针
     * 接口场景：MainWindow构造函数里，在第一次打开图片之前
     * 接口类型：函数，直接调用
     * 备注：对应QlistView
     */
    virtual QStandardItemModel *getAlbumModel() = 0; //获取相册model指针
    /*
     * 接口功能：重命名
     * 接口场景：右键菜单、双击标题栏重命名
     * 接口类型：函数，直接调用
     * 参数类型：旧名字，新名字
     */
    virtual void reName(QString oldPath, QString newPath) = 0; //更改主题，切换相册打开按钮图标
    /*
     * 接口功能：打印
     * 接口场景：右键菜单打印
     * 接口类型：函数，直接调用
     * 参数类型：图片
     */
    virtual void printImage(QPrinter *printer) = 0; //打印
    /*
     * 接口功能：裁剪
     * 接口场景：点击工具栏裁剪按钮进行裁剪
     * 接口类型：函数，直接调用
     * 参数类型：无
     */
    virtual void cutImage() = 0; //更改主题，切换相册打开按钮图标
    /*
     * 接口功能：裁剪区域
     * 接口场景：裁剪框范围
     * 接口类型：函数，直接调用
     * 参数类型：QPoint
     * 参数描述：鼠标在导航栏的坐标
     */
    virtual void getText() = 0; //获取图片中文字
    /*
     * 接口功能：ocr
     * 接口场景：点击工具栏ocr按钮
     * 接口类型：函数，异步调用
     * 接口参数：cv::Mat
     * 参数描述：当前打开图片
     */
    virtual void cutRegion(const QPoint &point) = 0;
    /*
     * 接口功能：鼠标变化
     * 接口场景：在裁剪页面上点击/拖动鼠标时
     * 接口类型：函数，直接调用
     * 参数类型：QPoint
     * 参数描述：鼠标在导航栏的坐标
     */
    virtual void startChange(const QPoint &point) = 0;
    /*
     * 接口功能：裁剪鼠标释放
     * 接口场景：在裁剪页面上释放鼠标时
     * 接口类型：函数，直接调用
     * 参数类型：QPoint
     * 参数描述：鼠标在导航栏的坐标
     */
    virtual void releasePos(const QPoint &point) = 0;
    /*
     * 接口功能：裁剪鼠标释放后翻转-裁剪框尺寸改变
     * 接口场景：在裁剪页面上释放鼠标后，再次调整尺寸
     * 接口类型：函数，直接调用
     * 参数类型：int，QPoint
     * 参数描述：类型，鼠标在导航栏的坐标
     */
    virtual void releaseCutChange(int type, const QPoint &point) = 0;

    virtual void exitCut() = 0;
    virtual void needSaveCutImage(int proportion) = 0;
    virtual void saveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint) = 0;
    virtual void needSaveAs(QString path) = 0;
    /*
     * 接口功能：发送当前视图的中间item项，给后端进行加载
     * 接口场景：滑动侧栏滑条触发
     * 接口类型：函数，直接调用
     * 参数类型：QModelIndex
     * 参数描述：item的index
     */
    virtual void reloadImage(QModelIndex modelIndex) = 0;

    virtual void startSignImageApp() = 0; //调起画图


    // qml-----------------------------------
    virtual void qmlStartCutDisplay(QPointF startPos, QSizeF displaySize,
                                    double imageScale) = 0; //点击裁剪按钮，需要后端处理图片并发送给前端显示
    virtual void qmlStartNaviDisplay(QPointF leftUpPosAccordingShowImage, QSizeF displaySize,
                                     double imageScale) = 0; //点击裁剪按钮，需要后端处理图片并发送给前端显示
    virtual void qmlClickNaviDisplay(QPointF curMousePos,
                                     bool needChangeDisplayImage) = 0; //点击裁剪按钮，需要后端处理图片并发送给前端显示
    //添加裁剪点击-裁剪显示区域内全图功能
    /*
     * 接口功能：触发-裁剪点击-裁剪显示区域内全图
     * 接口场景：点击裁剪区域
     * 接口类型：函数，直接调用
     * 参数类型：无
     * 参数描述：无
     */
    virtual void clickCutFullViableImage() = 0;

    virtual void qmlTiffPageNavi(int pageNum, QPointF leftUpPosAccordingShowImage, QSizeF displaySize,
                                 double imageScale) = 0;
    virtual void qmlTiffImagePage(int pageNum) = 0;
    //标注
    virtual void startMark() = 0;
    virtual void toMarkImage(QVariant var) = 0;
    virtual void markUndo() = 0;
    virtual void markSave(QString savePath) = 0;
    virtual void markClear() = 0;

    //背景替换
    virtual void picMatting(QImage pic) = 0;
    virtual void replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter) = 0;
    virtual void needSaveReplaceImage(QString path) = 0;
    virtual void mattingRotate(const bool &clockwise = true) = 0;
    virtual void mattingFlipH() = 0;
    virtual void mattingFlipV() = 0;

    //扫描黑白件
    virtual void scanImage() = 0;
    virtual void needSaveScanImage(QString savePath) = 0;
    virtual void printScanImage(QPrinter *printer) = 0;
    virtual void scannerSavePDF(QPrinter *printer) = 0;
    virtual void scannerAddWatermark(bool isWatermark, QString vmContent) = 0;
    virtual void scannerRotate(const bool &clockwise = true) = 0;
    virtual void scannerFlipH() = 0;
    virtual void scannerFlipV() = 0;
    virtual void scannerSaveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint) = 0;


private:
    static Interaction *m_interaction; //单例指针
};


#endif // INTERACTION_H
